// Copyright 2023 BugSplat. All Rights Reserved.

#include "BugSplatRuntime.h"
#include "BugSplatEditorSettings.h"

#include "Developer/Settings/Public/ISettingsModule.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "CoreMinimal.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformCrashContext.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformProperties.h"
#include "Misc/App.h"
#include "Misc/EngineVersion.h"
#include "Misc/Guid.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformProcess.h"
#include "Misc/CommandLine.h"

#if PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <BugSplat/BugSplat.h>
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"
#include "Android/AndroidJavaEnv.h"
#include "Android/AndroidPlatformOutputDevices.h"
#endif

#define LOCTEXT_NAMESPACE "FBugSplatRuntimeModule"

DECLARE_LOG_CATEGORY_EXTERN(LogBugsplat, Verbose, All);

DEFINE_LOG_CATEGORY(LogBugsplat);

// Common function to gather crash attributes
TMap<FString, FString> FBugSplatRuntimeModule::GetCrashAttributes() const
{
	TMap<FString, FString> Attributes;

#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 2
	// Add engine information
	Attributes.Add(TEXT("EngineVersion"), FEngineVersion::Current().ToString());
	Attributes.Add(TEXT("EngineBranch"), FEngineVersion::Current().GetBranch());
	Attributes.Add(TEXT("EngineCompatibleVersion"), FEngineVersion::CompatibleWith().ToString());
	// Add whether this is a UE release build
	Attributes.Add(TEXT("IsUERelease"), FApp::IsEngineInstalled() ? TEXT("True") : TEXT("False"));

	// Add platform information
	Attributes.Add(TEXT("PlatformName"), FPlatformProperties::PlatformName());
	Attributes.Add(TEXT("PlatformNameIni"), FPlatformProperties::IniPlatformName());

	// Add build configuration
	FString BuildConfiguration;
#if UE_BUILD_DEBUG
	BuildConfiguration = TEXT("Debug");
#elif UE_BUILD_DEVELOPMENT
	BuildConfiguration = TEXT("Development");
#elif UE_BUILD_TEST
	BuildConfiguration = TEXT("Test");
#elif UE_BUILD_SHIPPING
	BuildConfiguration = TEXT("Shipping");
#endif
	Attributes.Add(TEXT("BuildConfiguration"), BuildConfiguration);

	// Add command line
	Attributes.Add(TEXT("CommandLine"), FCommandLine::Get());

	// Add project name
	Attributes.Add(TEXT("ProjectName"), FApp::GetProjectName());
	Attributes.Add(TEXT("GameName"), FString(FApp::GetProjectName()));
	Attributes.Add(TEXT("GameSessionID"), FApp::GetSessionId().ToString());

	// Add executable name and path
	Attributes.Add(TEXT("ExecutableName"), FPlatformProcess::ExecutableName());
#if PLATFORM_IOS
	Attributes.Add(TEXT("ExecutablePath"), FPlatformProcess::ExecutablePath());
#endif

	// Add base and root directories
	Attributes.Add(TEXT("BaseDir"), FPlatformProcess::BaseDir());
	Attributes.Add(TEXT("RootDir"), FPaths::RootDir());
	Attributes.Add(TEXT("EngineDir"), FPaths::EngineDir());
	Attributes.Add(TEXT("ProjectDir"), FPaths::ProjectDir());

	// Add CPU info
	Attributes.Add(TEXT("CPUBrand"), FPlatformMisc::GetCPUBrand());
	Attributes.Add(TEXT("CPUChipset"), FPlatformMisc::GetCPUChipset());
	Attributes.Add(TEXT("CPUVendor"), FPlatformMisc::GetCPUVendor());
	Attributes.Add(TEXT("CPUCores"), FString::FromInt(FPlatformMisc::NumberOfCores()));
	Attributes.Add(TEXT("CPUCoresWithHyperthread"), FString::FromInt(FPlatformMisc::NumberOfCoresIncludingHyperthreads()));

	// Add OS info
	Attributes.Add(TEXT("OSVersion"), FPlatformMisc::GetOSVersion());
	Attributes.Add(TEXT("ComputerName"), FPlatformProcess::ComputerName());
	Attributes.Add(TEXT("UserName"), FPlatformProcess::UserName());
	Attributes.Add(TEXT("DefaultLocale"), FPlatformMisc::GetDefaultLocale());

	// Add memory info
	Attributes.Add(TEXT("TotalPhysicalGB"), FString::Printf(TEXT("%.2f"), FPlatformMemory::GetPhysicalGBRam()));
	Attributes.Add(TEXT("AvailablePhysicalMB"), FString::FromInt(FPlatformMemory::GetStats().AvailablePhysical / (1024 * 1024)));
	Attributes.Add(TEXT("TotalVirtualGB"), FString::Printf(TEXT("%.2f"), FPlatformMemory::GetStats().TotalVirtual / (1024.0f * 1024.0f * 1024.0f)));

	// Add more detailed memory stats
	const FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	Attributes.Add(TEXT("TotalPhysical"), FString::FromInt(MemoryStats.TotalPhysical));
	Attributes.Add(TEXT("TotalVirtual"), FString::FromInt(MemoryStats.TotalVirtual));
	Attributes.Add(TEXT("PageSize"), FString::FromInt(MemoryStats.PageSize));
	Attributes.Add(TEXT("AvailablePhysical"), FString::FromInt(MemoryStats.AvailablePhysical));
	Attributes.Add(TEXT("AvailableVirtual"), FString::FromInt(MemoryStats.AvailableVirtual));
	Attributes.Add(TEXT("UsedPhysical"), FString::FromInt(MemoryStats.UsedPhysical));
	Attributes.Add(TEXT("PeakUsedPhysical"), FString::FromInt(MemoryStats.PeakUsedPhysical));
	Attributes.Add(TEXT("UsedVirtual"), FString::FromInt(MemoryStats.UsedVirtual));
	Attributes.Add(TEXT("PeakUsedVirtual"), FString::FromInt(MemoryStats.PeakUsedVirtual));

	// Add GPU info
	Attributes.Add(TEXT("GPUBrand"), FPlatformMisc::GetPrimaryGPUBrand());

	// Add runtime info
	Attributes.Add(TEXT("IsUnattended"), FApp::IsUnattended() ? TEXT("True") : TEXT("False"));
	Attributes.Add(TEXT("IsInstalled"), FApp::IsInstalled() ? TEXT("True") : TEXT("False"));
	Attributes.Add(TEXT("ProcessId"), FString::FromInt(FPlatformProcess::GetCurrentProcessId()));
	Attributes.Add(TEXT("SecondsSinceStart"), FString::FromInt(FPlatformTime::Seconds() - GStartTime));

	// Add build information
	Attributes.Add(TEXT("IsWithDebugInfo"), FApp::GetIsWithDebugInfo() ? TEXT("True") : TEXT("False"));

	// Add engine mode
	FString EngineMode;
#if WITH_EDITOR
	EngineMode = TEXT("Editor");
#else
	EngineMode = TEXT("Game");
#endif
	Attributes.Add(TEXT("EngineMode"), EngineMode);

	// Add engine mode extended
	FString EngineModeEx;
#if UE_BUILD_DEVELOPMENT
	if (FApp::IsGame())
	{
		EngineModeEx = TEXT("Development Game");
	}
	else
	{
		EngineModeEx = TEXT("Development Editor");
	}
#elif UE_BUILD_SHIPPING
	EngineModeEx = TEXT("Shipping Game");
#elif UE_BUILD_TEST
	EngineModeEx = TEXT("Test Game");
#elif UE_BUILD_DEBUG
	if (FApp::IsGame())
	{
		EngineModeEx = TEXT("Debug Game");
	}
	else
	{
		EngineModeEx = TEXT("Debug Editor");
	}
#endif
	Attributes.Add(TEXT("EngineModeEx"), EngineModeEx);

	// Add crash GUID
	Attributes.Add(TEXT("CrashGUID"), FGuid::NewGuid().ToString());

	// Add execution GUID if available
	Attributes.Add(TEXT("ExecutionGUID"), FGenericCrashContext::ExecutionGuid.ToString());

	// Add any engine data from GenericPlatformCrashContext
	const TMap<FString, FString>& EngineData = FGenericCrashContext::GetEngineData();
	for (const TPair<FString, FString>& Pair : EngineData)
	{
		Attributes.Add(FString::Printf(TEXT("EngineData_%s"), *Pair.Key), Pair.Value);
	}

	// Add any game data from GenericPlatformCrashContext
	const TMap<FString, FString>& GameData = FGenericCrashContext::GetGameData();
	for (const TPair<FString, FString>& Pair : GameData)
	{
		Attributes.Add(FString::Printf(TEXT("GameData_%s"), *Pair.Key), Pair.Value);
	}

	// Add platform-specific memory constants
	const FPlatformMemoryConstants& MemConstants = FPlatformMemory::GetConstants();
	Attributes.Add(TEXT("MemoryConstants_TotalPhysical"), FString::FromInt(MemConstants.TotalPhysical));
	Attributes.Add(TEXT("MemoryConstants_TotalVirtual"), FString::FromInt(MemConstants.TotalVirtual));
	Attributes.Add(TEXT("MemoryConstants_PageSize"), FString::FromInt(MemConstants.PageSize));
	Attributes.Add(TEXT("MemoryConstants_TotalPhysicalGB"), FString::Printf(TEXT("%.2f"), MemConstants.TotalPhysicalGB));

	// Add misc platform info
	Attributes.Add(TEXT("Is64bitOperatingSystem"), FPlatformMisc::Is64bitOperatingSystem() ? TEXT("True") : TEXT("False"));

	// Add build version
	Attributes.Add(TEXT("BuildVersion"), FApp::GetBuildVersion());

#if PLATFORM_IOS
	// Add log file path - use a more direct approach instead of PlatformOutputDevices
	FString LogFilePath = FPaths::ProjectLogDir() / FApp::GetProjectName() + TEXT(".log");
	LogFilePath = FPaths::ConvertRelativePathToFull(LogFilePath);
	Attributes.Add(TEXT("LogFilePath"), LogFilePath);
#endif
#else
	UE_LOG(LogBugsplat, Warning, TEXT("Crash attributes are only supported in Unreal Engine 5.2 or greater. Current engine version: %d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);
#endif

	return Attributes;
}

#if PLATFORM_IOS
@interface BugSplatUnrealDelegate : NSObject <BugSplatDelegate>
@property(nonatomic, strong) NSString* logFilePath;
@end

@implementation BugSplatUnrealDelegate

- (void)bugSplatWillSendCrashReport:(BugSplat*)bugSplat {
	NSLog(@"BugSplat: Will send crash report");
}

- (void)bugSplatDidFinishSendingCrashReport : (BugSplat*)bugSplat {
	NSLog(@"BugSplat: Did finish sending crash report");
}

- (void)bugSplat : (BugSplat*)bugSplat didFailWithError : (NSError*)error {
	NSLog(@"BugSplat: Did fail with error: %@", [error localizedDescription]);
}

// Implement other delegate methods as needed
- (void)bugSplatWillSendCrashReportsAlways : (BugSplat*)bugSplat {
	NSLog(@"BugSplat: Will send crash reports always");
}

- (void)bugSplatWillCancelSendingCrashReport : (BugSplat*)bugSplat {
	NSLog(@"BugSplat: Will cancel sending crash report");
}

- (void)bugSplatWillShowSubmitCrashReportAlert : (BugSplat*)bugSplat {
	NSLog(@"BugSplat: Will show submit crash report alert");
}

// Add attachment method for iOS
- (BugSplatAttachment*)attachmentForBugSplat : (BugSplat*)bugSplat API_AVAILABLE(ios(13.0)) {
	if (self.logFilePath) {
		NSData* logData = [NSData dataWithContentsOfFile : self.logFilePath];
		if (logData) {
			NSLog(@"BugSplat: Attaching log file: %@", self.logFilePath);
			return [[BugSplatAttachment alloc]initWithFilename:@"UnrealEngine.log"
				attachmentData:logData
				contentType : @"text/plain"];
		}
		else {
			NSLog(@"BugSplat: Failed to read log file: %@", self.logFilePath);
		}
	}
	return nil;
}

@end
#endif

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
	if (BugSplatEditorSettings->bEnableCrashReportingIos)
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
			// Get the log file path
			FString LogFilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir() / FApp::GetProjectName() + TEXT(".log"));
			NSString * nsLogFilePath = [NSString stringWithUTF8String : TCHAR_TO_UTF8(*LogFilePath)];

			// Create and set the delegate
			BugSplatUnrealDelegate * delegate = [[BugSplatUnrealDelegate alloc]init];
			delegate.logFilePath = nsLogFilePath;
			[[BugSplat shared]setDelegate:delegate];

			// Configure BugSplat options if needed
			[[BugSplat shared]setAutoSubmitCrashReport:YES]; // Or NO if you want to show a dialog

			// Get attributes from the common function
			TMap<FString, FString> Attributes = FBugSplatRuntimeModule::Get().GetCrashAttributes();

			// Add all attributes to BugSplat
			for (const TPair<FString, FString>& Attribute : Attributes)
			{
				NSString* key = [NSString stringWithUTF8String : TCHAR_TO_UTF8(*Attribute.Key)];
				NSString* value = [NSString stringWithUTF8String : TCHAR_TO_UTF8(*Attribute.Value)];
				[[BugSplat shared]setValue:value forAttribute : key];
			}

			// Start BugSplat after configuration
			[[BugSplat shared]start];
		});
#endif
}

void FBugSplatRuntimeModule::SetupCrashReportingAndroid()
{
#if PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	auto Activity = FJavaWrapper::GameActivityThis;

	jclass BugSplatClass = FAndroidApplication::FindJavaClass("com/bugsplat/android/BugSplat");

	// Create a HashMap to store attributes
	jclass HashMapClass = FAndroidApplication::FindJavaClass("java/util/HashMap");
	jmethodID HashMapConstructor = FJavaWrapper::FindMethod(Env, HashMapClass, "<init>", "()V", false);
	jmethodID HashMapPut = FJavaWrapper::FindMethod(Env, HashMapClass, "put",
		"(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;", false);

	jobject AttributesMap = Env->NewObject(HashMapClass, HashMapConstructor);

	// Get attributes from the common function
	TMap<FString, FString> Attributes = GetCrashAttributes();

	// Add all attributes to the HashMap
	for (const TPair<FString, FString>& Attribute : Attributes)
	{
		Env->CallObjectMethod(AttributesMap, HashMapPut, *FJavaClassObject::GetJString(Attribute.Key), *FJavaClassObject::GetJString(Attribute.Value));
	}

	FString LogFile = FPaths::ConvertRelativePathToFull(FAndroidOutputDevices::GetAbsoluteLogFilename());

	// Create string array with single log file
	jobjectArray AttachmentArray = Env->NewObjectArray(1,
		Env->FindClass("java/lang/String"),
		*FJavaClassObject::GetJString(LogFile));

	// Update method signature to include Map parameter
	jmethodID InitBugSplatMethod = FJavaWrapper::FindStaticMethod(Env, BugSplatClass, "init",
		"(Landroid/app/Activity;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/util/Map;[Ljava/lang/String;)V", false);

	// Call the BugSplat init  method
	Env->CallStaticVoidMethod(BugSplatClass, InitBugSplatMethod, FJavaWrapper::GameActivityThis,
		*FJavaClassObject::GetJString(BugSplatEditorSettings->BugSplatDatabase),
		*FJavaClassObject::GetJString(BugSplatEditorSettings->BugSplatApp),
		*FJavaClassObject::GetJString(FString::Printf(TEXT("%s-android"), *BugSplatEditorSettings->BugSplatVersion)),
		AttributesMap,
		AttachmentArray);

	// Clean up references
	Env->DeleteLocalRef(AttributesMap);
	Env->DeleteLocalRef(HashMapClass);
	Env->DeleteLocalRef(AttachmentArray);
	Env->DeleteLocalRef(BugSplatClass);
#endif
}
#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBugSplatRuntimeModule, BugSplatRuntime)