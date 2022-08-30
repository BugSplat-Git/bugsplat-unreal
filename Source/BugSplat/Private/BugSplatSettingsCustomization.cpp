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
				.Text(FText::FromString("Integrate your game with BugSplat crash reporting by filling in the fields below. This plugin can configure crash reporting for a packaged game (required for shipping builds) or the global engine DefaultEngine.ini (optional but useful for development). Additionally, this plugin can add a PostBuild step for symbol uploads which is required to calculate function names and line numbers in crash reports."))
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


