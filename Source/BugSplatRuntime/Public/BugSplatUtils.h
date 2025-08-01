// Copyright 2023 BugSplat. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BugSplatUtils.generated.h"

UCLASS()
class BUGSPLATRUNTIME_API UBugSplatUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void GenerateCrash();

	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void GenerateAssert();

	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void GenerateEnsure();

	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void GenerateFatalLog();
};