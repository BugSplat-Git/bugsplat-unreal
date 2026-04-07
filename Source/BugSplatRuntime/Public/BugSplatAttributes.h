// Copyright BugSplat. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BugSplatAttributes.generated.h"

UCLASS()
class BUGSPLATRUNTIME_API UBugSplatAttributes : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Set a custom attribute that will be included in crash reports.
	 * Can be called at any time after BugSplat has been initialized.
	 *
	 * On Windows/macOS/Linux this sets game data on the crash context.
	 * On iOS/Android this sets the attribute on the native BugSplat SDK.
	 *
	 * @param Key   The attribute key.
	 * @param Value The attribute value.
	 */
	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void SetAttribute(const FString& Key, const FString& Value);

	/**
	 * Remove a custom attribute so it is no longer included in crash reports.
	 *
	 * @param Key The attribute key to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void RemoveAttribute(const FString& Key);
};
