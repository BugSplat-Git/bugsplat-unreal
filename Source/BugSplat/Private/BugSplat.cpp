// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugSplat.h"
#include "BugSplatSettings.h"
#include "BugSplatStyle.h"
#include "BugSplatCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include <Runtime/Projects/Public/PluginDescriptor.h>
#include <EngineSharedPCH.h>

static const FName BugSplatTabName("BugSplat");



#define LOCTEXT_NAMESPACE "FBugSplatModule"

static void OnBrowserLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata)
{
	const FString* UrlPtr = Metadata.Find(TEXT("href"));
	if (UrlPtr)
	{
		FPlatformProcess::LaunchURL(**UrlPtr, nullptr, nullptr);
	}
}

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
	
	TSharedPtr<SVerticalBox> DatabaseField = CreateInputFieldWidget(FText::FromString("Database"), BugSplatInputField::Database);
	
	TSharedPtr<SVerticalBox> VersionField = CreateInputFieldWidget(FText::FromString("Version"), BugSplatInputField::Version);

	TSharedPtr<SVerticalBox> UsernameField = CreateInputFieldWidget(FText::FromString("Username"), BugSplatInputField::Username);

	TSharedPtr<SVerticalBox> PasswordField = CreateInputFieldWidget(FText::FromString("Password"), BugSplatInputField::Password);

	TSharedPtr<SVerticalBox> ApplicationField = CreateInputFieldWidget(FText::FromString("Application"), BugSplatInputField::ApplicationName);

	float BrandImageHeight = .4f;
	float DescriptionTextHeight = .2f;
	float FillableBoxHeight = .3f;
	float ButtomButtonsHeight = .1f;

	FMargin InputFieldMargin = FMargin(0, 0, 20, 0);
	FMargin BlockMargin = FMargin(20, 10, 20, 10);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.FillHeight(BrandImageHeight)
			[
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SImage)
					.Image(FBugSplatStyle::Get().GetBrush("ProductImage"))
				]

			]
			+ SVerticalBox::Slot()
			.FillHeight(DescriptionTextHeight)
			.Padding(BlockMargin)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(.2)
				[
					SNew(STextBlock)
					.TextStyle(FBugSplatStyle::Get(), "HeaderText")
					.Text(FText::FromString("Title Text in Default Font / Color"))
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.FillHeight(.65)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.TextStyle(FBugSplatStyle::Get(), "SubtitleText")
					.Text(FText::FromString("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat."))
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.FillHeight(.15)
				[
					SNew(SRichTextBlock)
					.Text(FText::FromString(TEXT("<a id=\"browser\" href=\"http://www.bugsplat.com\">View documents on BugSplat for Unreal Engine -></>")))
					.AutoWrapText(true)
					.DecoratorStyleSet(&FCoreStyle::Get())
					+ SRichTextBlock::HyperlinkDecorator(TEXT("browser"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnBrowserLinkClicked))
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(FillableBoxHeight)
			.Padding(BlockMargin)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)				
					+ SHorizontalBox::Slot()
					.Padding(InputFieldMargin)
					[
						DatabaseField->AsShared()
					]
					+ SHorizontalBox::Slot()
					.Padding(InputFieldMargin)
					[
						VersionField->AsShared()
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(InputFieldMargin)
					[
						ApplicationField->AsShared()
					]
					+ SHorizontalBox::Slot()
					.Padding(InputFieldMargin)
					[
						SNew(SSpacer)
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(InputFieldMargin)
					[
						UsernameField->AsShared()
					]
					+ SHorizontalBox::Slot()
					.Padding(InputFieldMargin)
					[
						PasswordField->AsShared()
					]					
				]
			]	
			+ SVerticalBox::Slot()
			.Padding(BlockMargin)
			.FillHeight(ButtomButtonsHeight)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(.30)
				.Padding(FMargin(0, 10, 10, 10))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Raw(this, &FBugSplatModule::OnSettingsSaved)
					.TextStyle(FBugSplatStyle::Get(), "ButtonText")
					.Text(FText::FromString("Update Global INI"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(.30)
				.Padding(FMargin(10, 10, 0, 10))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Raw(this, &FBugSplatModule::OnPackageWithBugSplat)
					.TextStyle(FBugSplatStyle::Get(), "ButtonText")
					.Text(FText::FromString("Update Game INI"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(.40)
				[
					SNew(SSpacer)
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

TSharedPtr<SVerticalBox> FBugSplatModule::CreateInputFieldWidget(FText InputFieldName, BugSplatInputField InputFieldType)
{
	TSharedPtr<SEditableTextBox> TextBox;

	switch (InputFieldType)
	{
		case(BugSplatInputField::Database):
			TextBox = SNew(SEditableTextBox)
				.Style(FBugSplatStyle::Get(), "InputField")
				.HintText(InputFieldName)
				.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetDatabase)
				.Text(BugSplatSettings->GetDatabase());
			break;
		case(BugSplatInputField::ApplicationName):
			TextBox = SAssignNew(TextBox, SEditableTextBox)
				.Style(FBugSplatStyle::Get(), "InputField")
				.HintText(InputFieldName)
				.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetAppName)
				.Text(BugSplatSettings->GetAppName());
			break;
		case(BugSplatInputField::Password):
			TextBox = SAssignNew(TextBox, SEditableTextBox)
				.Style(FBugSplatStyle::Get(), "InputField")
				.HintText(InputFieldName)
				.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetPassword)
				.Text(BugSplatSettings->GetPassword());
			break;
		case(BugSplatInputField::Version):
			TextBox = SAssignNew(TextBox, SEditableTextBox)
				.Style(FBugSplatStyle::Get(), "InputField")
				.HintText(InputFieldName)
				.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetVersion)
				.Text(BugSplatSettings->GetVersion());
			break;
		case(BugSplatInputField::Username):
			TextBox = SAssignNew(TextBox, SEditableTextBox)
				.Style(FBugSplatStyle::Get(), "InputField")
				.HintText(InputFieldName)
				.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetUsername)
				.Text(BugSplatSettings->GetUsername());
			break;
	}

	TSharedPtr<SVerticalBox> InputField =
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Bottom)
		[
			SNew(STextBlock)
			.Text(InputFieldName)
		]
	+ SVerticalBox::Slot()
		[
			TextBox->AsShared()
		];

	return InputField;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBugSplatModule, BugSplat)