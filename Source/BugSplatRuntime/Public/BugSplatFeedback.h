// Copyright BugSplat. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BugSplatFeedback.generated.h"

UCLASS()
class BUGSPLATRUNTIME_API UBugSplatFeedback : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Submit user feedback to BugSplat.
	 * Reads Database, Application, and Version from plugin settings.
	 * Crash context attributes (engine, platform, CPU, GPU, memory, OS) are included automatically.
	 *
	 * @param Title        Required. Brief summary of the feedback.
	 * @param Description  Optional. Additional details.
	 * @param Attachments  Optional. Array of absolute file paths to attach to the report.
	 */
	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void PostFeedback(const FString& Title, const FString& Description, const TArray<FString>& Attachments);

	/** Returns the full path to the current Unreal Engine log file. Useful for passing to PostFeedback Attachments. */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BugSplat")
	static FString GetLogFilePath();
};
