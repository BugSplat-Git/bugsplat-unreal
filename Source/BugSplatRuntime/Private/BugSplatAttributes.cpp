// Copyright BugSplat. All Rights Reserved.

#include "BugSplatAttributes.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"

#if PLATFORM_IOS
#import <BugSplat/BugSplat.h>
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include "Android/AndroidJava.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogBugSplatAttributes, Log, All);

void UBugSplatAttributes::SetAttribute(const FString& Key, const FString& Value)
{
	if (Key.IsEmpty())
	{
		UE_LOG(LogBugSplatAttributes, Warning, TEXT("SetAttribute called with empty key"));
		return;
	}

#if PLATFORM_IOS
	NSString* NsKey = [NSString stringWithUTF8String:TCHAR_TO_UTF8(*Key)];
	NSString* NsValue = [NSString stringWithUTF8String:TCHAR_TO_UTF8(*Value)];
	[[BugSplat shared] setValue:NsValue forAttribute:NsKey];
#elif PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass BugSplatClass = FAndroidApplication::FindJavaClass("com/bugsplat/android/BugSplat");
	jmethodID SetAttributeMethod = FJavaWrapper::FindStaticMethod(Env, BugSplatClass, "setAttribute",
		"(Ljava/lang/String;Ljava/lang/String;)V", false);
	Env->CallStaticVoidMethod(BugSplatClass, SetAttributeMethod,
		*FJavaClassObject::GetJString(Key),
		*FJavaClassObject::GetJString(Value));
	Env->DeleteLocalRef(BugSplatClass);
#else
	FGenericCrashContext::SetGameData(Key, Value);
#endif

	UE_LOG(LogBugSplatAttributes, Verbose, TEXT("SetAttribute: %s = %s"), *Key, *Value);
}

void UBugSplatAttributes::RemoveAttribute(const FString& Key)
{
	if (Key.IsEmpty())
	{
		UE_LOG(LogBugSplatAttributes, Warning, TEXT("RemoveAttribute called with empty key"));
		return;
	}

#if PLATFORM_IOS
	NSString* NsKey = [NSString stringWithUTF8String:TCHAR_TO_UTF8(*Key)];
	[[BugSplat shared] setValue:nil forAttribute:NsKey];
#elif PLATFORM_ANDROID
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jclass BugSplatClass = FAndroidApplication::FindJavaClass("com/bugsplat/android/BugSplat");
	jmethodID RemoveAttributeMethod = FJavaWrapper::FindStaticMethod(Env, BugSplatClass, "removeAttribute",
		"(Ljava/lang/String;)V", false);
	Env->CallStaticVoidMethod(BugSplatClass, RemoveAttributeMethod,
		*FJavaClassObject::GetJString(Key));
	Env->DeleteLocalRef(BugSplatClass);
#else
	FGenericCrashContext::SetGameData(Key, TEXT(""));
#endif

	UE_LOG(LogBugSplatAttributes, Verbose, TEXT("RemoveAttribute: %s"), *Key);
}
