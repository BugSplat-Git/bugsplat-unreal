// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugSplatRuntime.h"
#include "BugSplatEditorSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <Bugsplat/Bugsplat.h>
#endif

#define LOCTEXT_NAMESPACE "FBugSplatRuntimeModule"

void FBugSplatRuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	BugsplatEditoSettings = NewObject<UBugSplatEditorSettings>(GetTransientPackage(), "BugSplatEditorSettings", RF_Standalone);
	BugsplatEditoSettings->AddToRoot();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "BugSplat",
			LOCTEXT("RuntimeSettingsName", "BugSplat"),
			LOCTEXT("RuntimeSettingsDescription", "Configure BugSplat"),
			BugsplatEditoSettings);
	}
	
#if PLATFORM_IOS
	dispatch_async(dispatch_get_main_queue(), ^
	{
		BugsplatStartupManager* bugsplatStartupManager = [[BugsplatStartupManager alloc] init];
		[bugsplatStartupManager start];
	});
#endif
}

void FBugSplatRuntimeModule::ShutdownModule()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "BugSplat");
	}

	if (!GExitPurge)
	{
		// If we're in exit purge, this object has already been destroyed
		BugsplatEditoSettings->RemoveFromRoot();
	}
	else
	{
		BugsplatEditoSettings = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBugSplatRuntimeModule, BugSplatRuntime)