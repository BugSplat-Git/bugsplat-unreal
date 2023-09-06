// Copyright 2023 BugSplat. All Rights Reserved.

#include "BugSplatEditorSettingsCustomization.h"
#include "BugSplat.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Text/SRichTextBlock.h"

static void OnDocumentationLinkClicked(const FSlateHyperlinkRun::FMetadata& Metadata)
{
	const FString* UrlPtr = Metadata.Find(TEXT("href"));
	if (UrlPtr)
	{
		FPlatformProcess::LaunchURL(**UrlPtr, nullptr, nullptr);
	}
}

TSharedRef<IDetailCustomization> FBugSplatEditorSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FBugSplatEditorSettingsCustomization);
}

void FBugSplatEditorSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& AboutCategory = DetailBuilder.EditCategory(TEXT("About"));
	IDetailCategoryBuilder& ToolsCategory = DetailBuilder.EditCategory(TEXT("Tools"));

	FSimpleDelegate UpdateBugSplatSettings = FSimpleDelegate::CreateRaw(&FBugSplatModule::Get(), &FBugSplatModule::OnUpdateBugSplatSettings);
	DetailBuilder.GetProperty("BugSplatDatabase").Get().SetOnPropertyValueChanged(UpdateBugSplatSettings);
	DetailBuilder.GetProperty("BugSplatApp").Get().SetOnPropertyValueChanged(UpdateBugSplatSettings);
	DetailBuilder.GetProperty("BugSplatVersion").Get().SetOnPropertyValueChanged(UpdateBugSplatSettings);
	DetailBuilder.GetProperty("BugSplatClientId").Get().SetOnPropertyValueChanged(UpdateBugSplatSettings);
	DetailBuilder.GetProperty("BugSplatSecret").Get().SetOnPropertyValueChanged(UpdateBugSplatSettings);
	DetailBuilder.GetProperty("bUploadDebugSymbols").Get().SetOnPropertyValueChanged(UpdateBugSplatSettings);

	AboutCategory.AddCustomRow(FText::FromString(TEXT("About")), false)
		.WholeRowWidget
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.Padding(FMargin(0, 10, 0, 10))
			.AutoHeight()
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(FText::FromString("This plugin enables uploading of crash reports and debug symbols to BugSplat for Desktop and Mobile platforms.\n\nTo configure crash reporting and symbol uploads, start by filling in the Database, Application, Version, Client ID, and Client Secret fields below. When \"Update Engine DataRouterUrl\" is enabled BugSplat will automatically update DataRouterUrl to point CrashReportClient uploads to BugSplat for Desktop crash reporting. Ensure \"Enable automatic symbol uploads\" is enabled or upload symbols yourself to ensure that crash reports contain function name and line number information.\n\nTo add crash reporting for Android and iOS games, simply click the appropriate checkboxes below.\n\nPlease note!\n\nEnabling \"Update Engine DataRouterUrl\" will change the engine's DefaultEngine.ini file. This is a global change that will affect all projects that are built with this instance of the engine. If you don't want to update the engine's configuration file you can use the \"Update Packaged Game INI\" to configure the crash reporter after your build has completed."))
			]
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.Padding(FMargin(0, 10, 0, 10))
			[
				SNew(SRichTextBlock)
				.Text(FText::FromString(TEXT("<a id=\"browser\" href=\"https://docs.bugsplat.com/introduction/getting-started/integrations/game-development/unreal-engine#unreal-engine-plugin\">View BugSplat's Unreal Engine Docs-></>")))
				.AutoWrapText(true)
				.DecoratorStyleSet(&FCoreStyle::Get())
				+ SRichTextBlock::HyperlinkDecorator(TEXT("browser"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnDocumentationLinkClicked))
			]
		];

	ToolsCategory.AddCustomRow(FText::FromString(TEXT("Tools")), false)
		.WholeRowWidget
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.VAlign(VAlign_Top)
			.Padding(FMargin(0, 10, 0, 10))
			.AutoHeight()
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(FText::FromString("The following buttons are provided for convenience and clicking them is not required."))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(0, 10, 5, 10))
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0, 0, 5, 0))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Raw(&FBugSplatModule::Get(), &FBugSplatModule::OnUpdateLocalIni)
					.Text(FText::FromString("Update Packaged Game INI"))
					.ToolTipText(FText::FromString("Useful if you have disabled \"Update Engine DataRouterUrl\" or would like to overwrite the configuration of a packaged game manually."))
				]
			]
		];
}


