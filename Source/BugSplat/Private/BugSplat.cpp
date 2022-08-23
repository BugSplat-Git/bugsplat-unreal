// Copyright 2022 BugSplat. All Rights Reserved.

#include "BugSplat.h"
#include "BugSplatSettings.h"
#include "BugSplatSettingsCustomization.h"
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

	auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		"BugSplatEditorSettings",
		FOnGetDetailCustomizationInstance::CreateStatic(&FBugSplatSettingsCustomization::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FBugSplatModule::ShutdownModule()
{
	delete BugSplatSettings;

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout("BugSplatEditorSettings");
	}
}

FBugSplatModule& FBugSplatModule::Get()
{
	return FModuleManager::LoadModuleChecked<FBugSplatModule>("BugSplat");
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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBugSplatModule, BugSplat)
