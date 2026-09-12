// Copyright BugSplat. All Rights Reserved.
#include "BugSplatUtils.h"
#include "BugSplatFeedbackDialog.h"
#include "BugSplatNative.h"

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"
#endif

#if PLATFORM_IOS
#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>
#endif

void UBugSplatUtils::GenerateCrash()
{
	UE_LOG(LogTemp, Log, TEXT("BugSplat: GenerateCrash"));
	volatile char *ptr = (char*)0xC0FFEEC0;
	*ptr += 1;
}

void UBugSplatUtils::GenerateAssert()
{
	UE_LOG(LogTemp, Log, TEXT("BugSplat: GenerateAssert"));
	char *ptr = nullptr;
	check(ptr != nullptr);
}

void UBugSplatUtils::GenerateEnsure()
{
	UE_LOG(LogTemp, Log, TEXT("BugSplat: GenerateEnsure"));
	char* ptr = nullptr;
	ensure(ptr != nullptr);
}

void UBugSplatUtils::GenerateFatalLog()
{
	UE_LOG(LogTemp, Fatal, TEXT("BugSplat: GenerateFatalLog"));
}

void UBugSplatUtils::GenerateHang()
{
	UE_LOG(LogTemp, Log, TEXT("BugSplat: GenerateHang"));
#if PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass BugSplatClass = FAndroidApplication::FindJavaClass("com/bugsplat/android/BugSplat");
	jmethodID HangMethod = FJavaWrapper::FindStaticMethod(Env, BugSplatClass, "hang", "()V", false);
	Env->CallStaticVoidMethod(BugSplatClass, HangMethod);
	Env->DeleteLocalRef(BugSplatClass);
#elif PLATFORM_IOS
	// Block the iOS main thread until the OS watchdog terminates the app.
	// bugsplat-apple's hang tracker observes the unresponsive runloop and persists
	// a fatal-hang report that uploads on the next launch. Sleeping until
	// distantFuture (vs spinning) keeps the CPU quiet while frozen; matches the
	// bugsplat-apple sample's Hang button.
	dispatch_async(dispatch_get_main_queue(), ^{
		[NSThread sleepUntilDate:[NSDate distantFuture]];
	});
#else
	if (FBugSplatNative::Get().IsRunning())
	{
		// Blocking the game thread stops the per-tick heartbeat; once the configured timeout
		// passes, bugsplat-native dumps the live process and uploads a hang report.
		UE_LOG(LogTemp, Warning, TEXT("BugSplat: blocking the game thread for 15 s so bugsplat-native's hang detector fires."));
		FPlatformProcess::Sleep(15.0f);
		UE_LOG(LogTemp, Log, TEXT("BugSplat: the game thread is responsive again."));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("BugSplat: GenerateHang needs bugsplat-native on desktop (Project Settings > Plugins > BugSplat > Desktop); ignoring."));
#endif
}

void UBugSplatUtils::ShowFeedbackDialog()
{
	UE_LOG(LogTemp, Log, TEXT("BugSplat: ShowFeedbackDialog"));
	SBugSplatFeedbackDialog::Show();
}

bool UBugSplatUtils::IsAndroid()
{
#if PLATFORM_ANDROID
	return true;
#else
	return false;
#endif
}

bool UBugSplatUtils::IsIOS()
{
#if PLATFORM_IOS
	return true;
#else
	return false;
#endif
}

bool UBugSplatUtils::IsMobile()
{
	return IsAndroid() || IsIOS();
}