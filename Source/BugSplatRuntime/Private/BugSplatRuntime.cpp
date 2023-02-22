// Copyright 2022 BugSplat. All Rights Reserved.

#include "BugSplatRuntime.h"
#include "BugSplatEditorSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "CoreMinimal.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"

#if PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <Bugsplat/Bugsplat.h>
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"
#endif

#define LOCTEXT_NAMESPACE "FBugSplatRuntimeModule"

DECLARE_LOG_CATEGORY_EXTERN(LogBugsplat, Verbose, All);

DEFINE_LOG_CATEGORY(LogBugsplat);

void FBugSplatRuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	BugSplatEditorSettings = NewObject<UBugSplatEditorSettings>(GetTransientPackage(), "BugSplatEditorSettings", RF_Standalone);
	BugSplatEditorSettings->AddToRoot();

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "BugSplat",
			LOCTEXT("RuntimeSettingsName", "BugSplat"),
			LOCTEXT("RuntimeSettingsDescription", "Configure BugSplat"),
			BugSplatEditorSettings);
	}

#if PLATFORM_IOS
	if(BugSplatEditorSettings->bEnableCrashReportingIos)
		SetupCrashReportingIos();
#endif	

#if PLATFORM_ANDROID
	if (!BugSplatEditorSettings->bEnableCrashReportingAndroid)
		return;

	SetupCrashReportingAndroid();
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
		BugSplatEditorSettings->RemoveFromRoot();
	}
	else
	{
		BugSplatEditorSettings = nullptr;
	}
}

FBugSplatRuntimeModule& FBugSplatRuntimeModule::Get()
{
	return FModuleManager::LoadModuleChecked<FBugSplatRuntimeModule>("BugSplatRuntime");
}

UBugSplatEditorSettings* FBugSplatRuntimeModule::GetSettings() const
{
	return BugSplatEditorSettings;
}

void FBugSplatRuntimeModule::SetupCrashReportingIos()
{
#if PLATFORM_IOS
	dispatch_async(dispatch_get_main_queue(), ^
	{
		BugsplatStartupManager* bugsplatStartupManager = [[BugsplatStartupManager alloc] init];
		[bugsplatStartupManager start];
	});
#endif
}

void FBugSplatRuntimeModule::SetupCrashReportingAndroid()
{
#if PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	auto Activity = FJavaWrapper::GameActivityThis;

	jclass BridgeClass = FAndroidApplication::FindJavaClass("com/ninevastudios/bugsplatunitylib/BugSplatBridge");
	jmethodID InitBugSplatMethod = FJavaWrapper::FindStaticMethod(Env, BridgeClass, "initBugSplat", "(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", false);

	auto dataBaseString = Env->NewStringUTF(TCHAR_TO_UTF8(*BugSplatEditorSettings->BugSplatDatabase));
	auto appString = Env->NewStringUTF(TCHAR_TO_UTF8(*BugSplatEditorSettings->BugSplatApp));
	auto versionBaseString = Env->NewStringUTF(TCHAR_TO_UTF8(*BugSplatEditorSettings->BugSplatVersion));

	Env->CallStaticVoidMethod(BridgeClass, InitBugSplatMethod, FJavaWrapper::GameActivityThis,
		*FJavaClassObject::GetJString(BugSplatEditorSettings->BugSplatDatabase),
		*FJavaClassObject::GetJString(BugSplatEditorSettings->BugSplatApp),
		*FJavaClassObject::GetJString(BugSplatEditorSettings->BugSplatVersion));

	Env->DeleteLocalRef(BridgeClass);
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBugSplatRuntimeModule, BugSplatRuntime)