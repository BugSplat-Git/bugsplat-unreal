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
static const FString BugSplatSendPdbsDirectory = FString("Test");

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
	delete _bugSplatSettings;
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FBugSplatStyle::Shutdown();

	FBugSplatCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BugSplatTabName);
}

TSharedRef<SDockTab> FBugSplatModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	_bugSplatSettings = new BugSplatSettings(LOCAL_CONFIG_PATH, BUGSPLAT_UPROJECT_PATH);
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
					.OnTextChanged_Raw(_bugSplatSettings, &BugSplatSettings::setDatabase)
					.Text(_bugSplatSettings->getDatabase())
					.HintText(LOCTEXT("Database", "Database"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(_bugSplatSettings, &BugSplatSettings::setVersion)
					.Text(_bugSplatSettings->getVersion())
					.HintText(LOCTEXT("Version", "Version"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(_bugSplatSettings, &BugSplatSettings::setAppName)
					.Text(_bugSplatSettings->getAppName())
					.HintText(LOCTEXT("AppName", "Application Name"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(_bugSplatSettings, &BugSplatSettings::setUsername)
					.Text(_bugSplatSettings->getUsername())
					.HintText(LOCTEXT("User", "Username"))
				]
			+ SVerticalBox::Slot()
				[
					SNew(SEditableTextBox)
					.OnTextChanged_Raw(_bugSplatSettings, &BugSplatSettings::setPassword)
					.Text(_bugSplatSettings->getPassword())
					.HintText(LOCTEXT("Password", "Password"))
				]
			]
			+ SHorizontalBox::Slot()
				[
					SNew(SButton)
					.OnClicked_Raw(this, &FBugSplatModule::onSettingsSaved)
					[
						SNew(STextBlock)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
						.Text(LOCTEXT("ButtonText", "Submit"))
					]
				]	
		];

}

FReply FBugSplatModule::onSettingsSaved()
{
	_bugSplatSettings->save();
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