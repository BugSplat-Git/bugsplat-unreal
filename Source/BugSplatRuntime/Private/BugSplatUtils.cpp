// Copyright BugSplat. All Rights Reserved.
#include "BugSplatUtils.h"
#include "BugSplatFeedbackDialog.h"

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"
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
#else
	UE_LOG(LogTemp, Warning, TEXT("BugSplat: GenerateHang is Android-only; ignoring on this platform."));
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