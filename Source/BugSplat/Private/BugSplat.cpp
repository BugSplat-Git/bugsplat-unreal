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
#include <Editor/MainFrame/Public/Interfaces/IMainFrameModule.h>

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


TSharedRef<SBox> FBugSplatModule::CreateBugSplatWindowContent()
{	
	TSharedPtr<SVerticalBox> DatabaseField = CreateInputFieldWidget(FText::FromString("Database"), BugSplatInputField::Database);

	TSharedPtr<SVerticalBox> ApplicationField = CreateInputFieldWidget(FText::FromString("Application"), BugSplatInputField::ApplicationName);
	
	TSharedPtr<SVerticalBox> VersionField = CreateInputFieldWidget(FText::FromString("Version"), BugSplatInputField::Version);

	TSharedPtr<SVerticalBox> UsernameField = CreateInputFieldWidget(FText::FromString("Client ID"), BugSplatInputField::ClientID);

	TSharedPtr<SVerticalBox> PasswordField = CreateInputFieldWidget(FText::FromString("Client Secret"), BugSplatInputField::ClientSecret);

	float BrandImageHeight = .4f;
	float DescriptionTextHeight = .2f;
	float FillableBoxHeight = .3f;
	float ButtomButtonsHeight = .1f;

	FMargin InputFieldMargin = FMargin(0, 0, 20, 0);
	FMargin BlockMargin = FMargin(20, 10, 20, 10);

	return SNew(SBox)
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
			.Padding(FMargin(20, 10, 20, 0))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(.15)
				[
					SNew(STextBlock)

					.TextStyle(FBugSplatStyle::Get(), "HeaderText")
					.Text(FText::FromString("BugSplat Unreal Settings"))
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.FillHeight(.8)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.TextStyle(FBugSplatStyle::Get(), "SubtitleText")
					.Text(FText::FromString("Integrate your game with BugSplat crash reporting by filling in the fields below. This plugin can configure crash reporting for a packaged game (required for shipping builds) or the global engine DefaultEngine.ini (optional but useful for development). Additionally, this plugin can add a PostBuild step for symbol uploads which is required to calculate function names and line numbers in crash reports."))
				]
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Top)
				.FillHeight(.1)
				[
					SNew(SRichTextBlock)
					.Text(FText::FromString(TEXT("<a id=\"browser\" href=\"https://docs.bugsplat.com/introduction/getting-started/integrations/game-development/unreal-engine#unreal-engine-plugin\">View BugSplat's Unreal Engine Docs-></>")))
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
						ApplicationField->AsShared()
					]
					
				]
				+ SVerticalBox::Slot()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(InputFieldMargin)
					[
						VersionField->AsShared()
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
				.FillWidth(.33)
				.Padding(FMargin(0, 10, 10, 10))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Raw(this, &FBugSplatModule::OnUpdateGlobalIni)
					.TextStyle(FBugSplatStyle::Get(), "ButtonText")
					.Text(FText::FromString("Update Global INI"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(.33)
				.Padding(FMargin(10, 10, 10, 10))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Raw(this, &FBugSplatModule::OnUpdateLocalIni)
					.TextStyle(FBugSplatStyle::Get(), "ButtonText")
					.Text(FText::FromString("Update Game INI"))
				]
				+ SHorizontalBox::Slot()
					.FillWidth(.33)
					.Padding(FMargin(10, 10, 0, 10))
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FMargin(8, 2))
						.OnClicked_Raw(this, &FBugSplatModule::OnUpdateSendPdbsScript)
						.TextStyle(FBugSplatStyle::Get(), "ButtonText")
						.Text(FText::FromString("Add Symbol Uploads"))
					]
			]
		];
}

FReply FBugSplatModule::OnUpdateGlobalIni()
{
	BugSplatSettings->UpdateGlobalIni();
	return FReply::Handled();
}

FReply FBugSplatModule::OnUpdateLocalIni()
{
	BugSplatSettings->UpdateLocalIni();
	return FReply::Handled();
}

FReply FBugSplatModule::OnUpdateSendPdbsScript()
{
	BugSplatSettings->WriteSendPdbsToScript();
	return FReply::Handled();
}


void FBugSplatModule::PluginButtonClicked()
{
	BugSplatSettings = new FBugSplatSettings(BUGSPLAT_UPROJECT_PATH);

	TSharedRef<SWindow> BugSplatWindow =
		SNew(SWindow)
		.Title(LOCTEXT("FBugSplatTabTitle", "BugSplat"))
		.ClientSize(FVector2D(640, 800))
		.MinHeight(800)
		.MinWidth(640);

	BugSplatWindow->SetContent(CreateBugSplatWindowContent());
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	auto RootWindow = MainFrameModule.GetParentWindow();
	FSlateApplication::Get().AddModalWindow(BugSplatWindow, RootWindow);
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
		case(BugSplatInputField::ClientSecret):
			TextBox = SAssignNew(TextBox, SEditableTextBox)
				.Style(FBugSplatStyle::Get(), "InputField")
				.HintText(InputFieldName)
				.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetClientSecret)
				.Text(BugSplatSettings->GetClientSecret());
			break;
		case(BugSplatInputField::Version):
			TextBox = SAssignNew(TextBox, SEditableTextBox)
				.Style(FBugSplatStyle::Get(), "InputField")
				.HintText(InputFieldName)
				.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetVersion)
				.Text(BugSplatSettings->GetVersion());
			break;
		case(BugSplatInputField::ClientID):
			TextBox = SAssignNew(TextBox, SEditableTextBox)
				.Style(FBugSplatStyle::Get(), "InputField")
				.HintText(InputFieldName)
				.OnTextChanged_Raw(BugSplatSettings, &FBugSplatSettings::SetClientID)
				.Text(BugSplatSettings->GetClientID());
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
