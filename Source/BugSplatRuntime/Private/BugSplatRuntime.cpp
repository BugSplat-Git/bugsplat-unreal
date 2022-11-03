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

#include <jni.h>
#include <string>
#include <unistd.h>
#include "client/crashpad_client.h"
#include "client/crash_report_database.h"
#include "client/settings.h"
#endif

#if PLATFORM_ANDROID
using namespace base;
using namespace crashpad;
using namespace std;
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
	if(BugSplatEditorSettings->bEnableCrashReportingAndroid)
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
	jclass ActivityClass = Env->GetObjectClass(Activity);

	jmethodID GetAppInfoMethod = Env->GetMethodID(ActivityClass, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");

	jobject ApplicationInfo = Env->CallObjectMethod(Activity, GetAppInfoMethod);
	jclass ApplicationInfoClass = Env->GetObjectClass(ApplicationInfo);

	jfieldID DataDirFieldId = Env->GetFieldID(ApplicationInfoClass, "dataDir", "Ljava/lang/String;");
	jfieldID LibDirFieldId = Env->GetFieldID(ApplicationInfoClass, "nativeLibraryDir", "Ljava/lang/String;");

	string dataDir = Env->GetStringUTFChars(static_cast<jstring>(Env->GetObjectField(ApplicationInfo, DataDirFieldId)), nullptr);
	string libDir = Env->GetStringUTFChars(static_cast<jstring>(Env->GetObjectField(ApplicationInfo, LibDirFieldId)), nullptr);

	// Crashpad file paths
	FilePath handler(libDir + "/libcrashpad_handler.so");
	FilePath reportsDir(dataDir + "/crashpad");
	FilePath metricsDir(dataDir + "/crashpad");

	// Crashpad upload URL for BugSplat database	
	string url = string(TCHAR_TO_UTF8(*FString::Printf(TEXT("http://%s.bugsplat.com/post/bp/crash/crashpad.php"), *BugSplatEditorSettings->BugSplatDatabase)));

	// Crashpad annotations
	map<string, string> annotations;
	annotations["format"] = "minidump";
	annotations["database"] = string(TCHAR_TO_UTF8(*BugSplatEditorSettings->BugSplatDatabase));
	annotations["product"] = string(TCHAR_TO_UTF8(*BugSplatEditorSettings->BugSplatApp));
	annotations["version"] = string(TCHAR_TO_UTF8(*FString::Printf(TEXT("%s-android"), *BugSplatEditorSettings->BugSplatVersion)));

	// Crashpad arguments
	vector<string> arguments;
	arguments.push_back("--no-rate-limit");

	// Crashpad local database
	unique_ptr<CrashReportDatabase> crashReportDatabase = CrashReportDatabase::Initialize(reportsDir);
	if (crashReportDatabase == NULL) return;

	// Enable automated crash uploads
	Settings *settings = crashReportDatabase->GetSettings();
	if (settings == NULL) return;
	settings->SetUploadsEnabled(true);

	// File paths of attachments to be uploaded with the minidump file at crash time - default bundle limit is 20MB
	vector<FilePath> attachments;
	FilePath attachment(dataDir + "/files/attachment.txt");
	attachments.push_back(attachment);

	// Start Crashpad crash handler
	static CrashpadClient *client = new CrashpadClient();
	client->StartHandlerAtCrash(handler, reportsDir, metricsDir, url, annotations, arguments, attachments);
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBugSplatRuntimeModule, BugSplatRuntime)