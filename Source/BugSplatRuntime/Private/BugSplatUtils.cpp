// Copyright BugSplat. All Rights Reserved.
#include "BugSplatUtils.h"
#include "BugSplatFeedbackDialog.h"

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
	// a fatal-hang report that uploads on the next launch.
	dispatch_async(dispatch_get_main_queue(), ^{
		while (true) { /* spin until watchdog kill */ }
	});
#else
	UE_LOG(LogTemp, Warning, TEXT("BugSplat: GenerateHang is mobile-only; ignoring on this platform."));
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