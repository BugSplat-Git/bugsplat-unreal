// Copyright 2022 BugSplat. All Rights Reserved.

#include "BugSplatSettingsCustomization.h"
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

TSharedRef<IDetailCustomization> FBugSplatSettingsCustomization::MakeInstance()
{
	return MakeShareable(new FBugSplatSettingsCustomization);
}

void FBugSplatSettingsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CommonCategory = DetailBuilder.EditCategory(TEXT("Crash Reporter Configuration"));
	IDetailCategoryBuilder& DesktopCategory = DetailBuilder.EditCategory(TEXT("Desktop"));

	CommonCategory.AddCustomRow(FText::FromString(TEXT("BugSplatCRC")), false)
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
				.Text(FText::FromString("This plugin enables uploading of crash reports and debug symbols to BugSplat on both Desktop and Mobile platforms. To configure crash reporting and symbol uploads, start by filling in the Database, Application, Version, User, and Password fields below.\n\nFor Desktop, the BugSplat plugin can configure crash reporting in packaged games (required for shipping builds) or globally for the current engine's DefaultEngine.ini file (optional but useful for development). To configure crash reporting for Desktop, click either the \"Update Game INI\" or \"Update Global INI\" buttons below. When using the \"Update Game INI\" option, navigate to the root directory of your packaged game when prompted. Windows symbol uploads can be configured by adding a script to the PostBuildSteps section of the .uproject file via the \"Add Symbol Uploads\" button.\n\nTo add crash reporting and symbol uploads for Mobile games, simply click the appropriate checkboxes below."))
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

	DesktopCategory.AddCustomRow(FText::FromString(TEXT("BugSplatDesktop")), false)
		.WholeRowWidget
		[
			SNew(SVerticalBox)
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
					.Text(FText::FromString("Update Game INI"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(5, 0, 5, 0))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Raw(&FBugSplatModule::Get(), &FBugSplatModule::OnUpdateGlobalIni)
					.Text(FText::FromString("Update Global INI"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(5, 0, 5, 0))
				[
					SNew(SButton)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.ContentPadding(FMargin(8, 2))
					.OnClicked_Raw(&FBugSplatModule::Get(), &FBugSplatModule::OnUpdateSendPdbsScript)
					.Text(FText::FromString("Add Symbol Uploads"))
				]
			]
		];
}


