// Copyright Epic Games, Inc. All Rights Reserved.

#include "BugSplatRuntime.h"
#include "BugSplatEditorSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "CoreMinimal.h"

#if PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <Bugsplat/Bugsplat.h>
#endif

#if PLATFORM_ANDROID
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

#if PLATFORM_ANDROID
	const FString Section = "/Script/AndroidRuntimeSettings.AndroidRuntimeSettings";
	const FString ConfigFileName = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*(FPaths::ProjectConfigDir() + "DefaultEngine.ini"));
	
	FString androidPackageName;
	FString androidPackageVersion;
	GConfig->GetString(*Section, TEXT("PackageName"), androidPackageName, ConfigFileName);
	GConfig->GetString(*Section, TEXT("VersionDisplayName"), androidPackageVersion, ConfigFileName);

	UE_LOG(LogBugsplat, Log, TEXT("BUGSPLAT PackageName: %s"), *androidPackageName);
	UE_LOG(LogBugsplat, Log, TEXT("BUGSPLAT VersionDisplayName: %s"), *androidPackageVersion);

	string dataDir = "/data/data/com.bugsplat.demo";
		//string(TCHAR_TO_UTF8(*FString::Printf(TEXT("/data/data/%s"), *androidPackageName)));

	// Crashpad file paths
	FilePath handler(dataDir + "/lib/libcrashpad_handler.so");
	FilePath reportsDir(dataDir + "/crashpad");
	FilePath metricsDir(dataDir + "/crashpad");

	// Crashpad upload URL for BugSplat database	
	string url = "http://ue_plugin_test.bugsplat.com/post/bp/crash/crashpad.php";
		//string(TCHAR_TO_UTF8(*FString::Printf(TEXT("http://%s.bugsplat.com/post/bp/crash/crashpad.php"), *BugsplatEditoSettings->BugSplatDatabase)));

	// Crashpad annotations
	map<string, string> annotations;
	annotations["format"] = "minidump";
	annotations["database"] = "ue_plugin_test";			//string(TCHAR_TO_UTF8(*BugsplatEditoSettings->BugSplatDatabase));
	annotations["product"] = "PlaygroundBugsplat";		//string(TCHAR_TO_UTF8(*BugsplatEditoSettings->BugSplatApp));
	annotations["version"] = "1.0";						//string(TCHAR_TO_UTF8(*androidPackageVersion));

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
	bool status = client->StartHandlerAtCrash(handler, reportsDir, metricsDir, url, annotations, arguments, attachments);

	if(status)
	{
		UE_LOG(LogBugsplat, Log, TEXT("BUGSPLAT status: true"));
	}
	else
	{
		UE_LOG(LogBugsplat, Log, TEXT("BUGSPLAT status: false"));
	}
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