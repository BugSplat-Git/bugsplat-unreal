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

	// Add engine information
	Attributes.Add(TEXT("EngineVersion"), FEngineVersion::Current().ToString());
	Attributes.Add(TEXT("EngineBranch"), FEngineVersion::Current().GetBranch());

	// Add platform information
	Attributes.Add(TEXT("PlatformName"), FPlatformProperties::PlatformName());

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

	// Add executable name and path
	Attributes.Add(TEXT("ExecutableName"), FPlatformProcess::ExecutableName());
	#if PLATFORM_IOS
	Attributes.Add(TEXT("ExecutablePath"), FPlatformProcess::ExecutablePath());
	#endif
	// Add CPU info
	Attributes.Add(TEXT("CPUBrand"), FPlatformMisc::GetCPUBrand());
	Attributes.Add(TEXT("CPUChipset"), FPlatformMisc::GetCPUChipset());
	Attributes.Add(TEXT("CPUVendor"), FPlatformMisc::GetCPUVendor());
	Attributes.Add(TEXT("CPUCores"), FString::FromInt(FPlatformMisc::NumberOfCores()));
	Attributes.Add(TEXT("CPUCoresWithHyperthread"), FString::FromInt(FPlatformMisc::NumberOfCoresIncludingHyperthreads()));

	// Add OS info
	Attributes.Add(TEXT("OSVersion"), FPlatformMisc::GetOSVersion());
	Attributes.Add(TEXT("ComputerName"), FPlatformProcess::ComputerName());

	// Add memory info
	Attributes.Add(TEXT("TotalPhysicalGB"), FString::Printf(TEXT("%.2f"), FPlatformMemory::GetPhysicalGBRam()));
	Attributes.Add(TEXT("AvailablePhysicalMB"), FString::FromInt(FPlatformMemory::GetStats().AvailablePhysical / (1024 * 1024)));
	Attributes.Add(TEXT("TotalVirtualGB"), FString::Printf(TEXT("%.2f"), FPlatformMemory::GetStats().TotalVirtual / (1024.0f * 1024.0f * 1024.0f)));

	// Add GPU info
	Attributes.Add(TEXT("GPUBrand"), FPlatformMisc::GetPrimaryGPUBrand());

	// Add runtime info
	Attributes.Add(TEXT("IsUnattended"), FApp::IsUnattended() ? TEXT("True") : TEXT("False"));
	Attributes.Add(TEXT("IsInstalled"), FApp::IsInstalled() ? TEXT("True") : TEXT("False"));

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