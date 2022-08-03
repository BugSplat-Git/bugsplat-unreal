// Copyright 2022 BugSplat. All Rights Reserved.

#include "BugSplatStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FBugSplatStyle::StyleInstance = NULL;

void FBugSplatStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FBugSplatStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FBugSplatStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("BugSplatStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);


TSharedRef< FSlateStyleSet > FBugSplatStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("BugSplatStyle"));

	const FTextBlockStyle DefaultTextBlockStyle =
		FTextBlockStyle()
		.SetColorAndOpacity(FLinearColor::Gray)
		.SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 10));

	Style->SetContentRoot(IPluginManager::Get().FindPlugin("BugSplat")->GetBaseDir() / TEXT("Resources"));
	Style->Set("BugSplat.OpenPluginWindow", new IMAGE_BRUSH(TEXT("logo"), Icon40x40));
	Style->Set("ProductImage", new IMAGE_BRUSH(TEXT("bs-unreal-plugin-header"), FVector2D(2000, 814)));
	Style->Set("InputField",
		FEditableTextBoxStyle()
		.SetForegroundColor(FLinearColor::Gray)
		.SetBackgroundColor(FLinearColor::Black));

	Style->Set("HeaderText",
		FTextBlockStyle(DefaultTextBlockStyle)
		.SetFontSize(12)
	);

	Style->Set("SubtitleText",
		FTextBlockStyle(DefaultTextBlockStyle)
		.SetFontSize(10)
	);

	Style->Set("ButtonText",
		FTextBlockStyle(DefaultTextBlockStyle)
		.SetFontSize(8)
		.SetColorAndOpacity(FLinearColor::Black)
	);

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FBugSplatStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FBugSplatStyle::Get()
{
	return *StyleInstance;
}
