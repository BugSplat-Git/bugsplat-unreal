// Copyright BugSplat. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "BugSplatUtils.generated.h"

// How GenerateGpuCrash makes the GPU fail. Both use Unreal's GPUDebugCrash command.
UENUM(BlueprintType)
enum class EBugSplatGpuCrashType : uint8
{
	// A shader that never finishes; Windows resets the GPU after 2 seconds (TDR).
	Hang,
	// A GPU access to unmapped memory. D3D11 and OpenGL fall back to Hang.
	PageFault
};

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

	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void GenerateHang();

	// Schedules a GPU crash on the next rendered frame. Unreal reports it as a GPUCrash, with the
	// RHI breadcrumbs, DRED data and any NVIDIA Aftermath dump in the crash report. Available in
	// Debug, Development and Test builds; Shipping needs ALLOW_GPUDEBUGCRASH_IN_SHIPPING=1.
	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void GenerateGpuCrash(EBugSplatGpuCrashType Type = EBugSplatGpuCrashType::Hang);

	UFUNCTION(BlueprintCallable, Category = "BugSplat")
	static void ShowFeedbackDialog();

	UFUNCTION(BlueprintPure, Category = "BugSplat")
	static bool IsAndroid();

	UFUNCTION(BlueprintPure, Category = "BugSplat")
	static bool IsIOS();

	UFUNCTION(BlueprintPure, Category = "BugSplat")
	static bool IsMobile();
};