// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugSplat.h"
#include "BugSplatSettings.h"
#include "BugSplatStyle.h"
#include "BugSplatCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include <Runtime/Projects/Public/PluginDescriptor.h>
#include <EngineSharedPCH.h>

static const FName BugSplatTabName("BugSplat");

#define LOCTEXT_NAMESPACE "FBugSplatModule"

void FBugSplatModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FBugSplatStyle::Initialize();
	FBugSplatStyle::ReloadTextures();

	FBugSplatCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FBugSplatCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FBugSplatModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBugSplatModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(BugSplatTabName, FOnSpawnTab::CreateRaw(this, &FBugSplatModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FBugSplatTabTitle", "BugSplat"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FBugSplatModule::ShutdownModule()
{
	delete BugSplatSettings;

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FBugSplatStyle::Shutdown();

	FBugSplatCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BugSplatTabName);
}

TSharedRef<SDockTab> FBugSplatModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	BugSplatSettings = new FBugSplatSettings(BUGSPLAT_UPROJECT_PATH);
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetDatabase)
					.Text(BugSplatSettings->GetDatabase())
					.HintText(LOCTEXT("Database", "Database"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetVersion)
					.Text(BugSplatSettings->GetVersion())
					.HintText(LOCTEXT("Version", "Version"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetAppName)
					.Text(BugSplatSettings->GetAppName())
					.HintText(LOCTEXT("AppName", "Application Name"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetUsername)
					.Text(BugSplatSettings->GetUsername())
					.HintText(LOCTEXT("User", "Username"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetPassword)
					.Text(BugSplatSettings->GetPassword())
					.HintText(LOCTEXT("Password", "Password"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SCheckBox)
					.OnCheckStateChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetUseGlobalIni)
					.IsChecked_Raw(BugSplatSettings, &FBugSplatSettings::GetUseGlobalIniCheckboxState)
					[
						SNew(STextBlock).Text(FText::FromString("Use Global INI"))
					]
				]
			]
			+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.OnClicked_Raw(this, &FBugSplatModule::OnSettingsSaved)
					[
						SNew(STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
						.Text(LOCTEXT("ButtonText", "Submit"))
					]
				]	
			+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.OnClicked_Raw(this, &FBugSplatModule::OnPackageWithBugSplat)
					[
						SNew(STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
						.Text(LOCTEXT("PackageText", "Package"))
					]
				]
		];

}

FReply FBugSplatModule::OnSettingsSaved()
{
	BugSplatSettings->Save();
	return FReply::Handled();
}

FReply FBugSplatModule::OnPackageWithBugSplat()
{
	BugSplatSettings->PackageWithBugSplat();
	return FReply::Handled();
}


void FBugSplatModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(BugSplatTabName);
}

void FBugSplatModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Edit");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("Configuration");
			Section.AddMenuEntryWithCommandList(FBugSplatCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBugSplatModule, BugSplat)