// Copyright BugSplat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BugSplatNative.h"
#include "BugSplatEditorSettings.generated.h"

UCLASS(Config = Engine, defaultconfig)
class BUGSPLATRUNTIME_API UBugSplatEditorSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif

	bool HasValidCrashReporterSettings(void) const;
	bool HasValidSymbolUploadSettings(void) const;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Database", ToolTip = "Database name"))
	FString BugSplatDatabase;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Application", ToolTip = "Application name"))
	FString BugSplatApp;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Version", ToolTip = "Application version number"))
	FString BugSplatVersion;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Client Id", ToolTip = "OAuth Client ID (required for debug symbol uploads)"))
	FString BugSplatClientId;

	UPROPERTY(Config, EditAnywhere, Category = "Settings", Meta = (DisplayName = "Client Secret", ToolTip = "OAuth Client Secret (required for debug symbol uploads)"))
	FString BugSplatClientSecret;

	UPROPERTY(Config, EditAnywhere, Category = "Settings",
		Meta = (DisplayName = "Update Engine DataRouterUrl", ToolTip = "Automatically update engine's DefaultEngine.ini when Database, Application, or Version chages"))
	bool bUpdateEngineDataRouterUrl;

	UPROPERTY(Config, EditAnywhere, Category = "Settings",
		Meta = (DisplayName = "Enable Automatic Symbol Uploads", ToolTip = "Flag indicating whether to upload debug symbols automatically"))
	bool bUploadDebugSymbols;

	UPROPERTY(Config, EditAnywhere, Category = "Settings",
		Meta = (DisplayName = "Enable Android Crash Reporting", ToolTip = "Flag indicating whether to capture crashes on Android"))
		bool bEnableCrashReportingAndroid;

	UPROPERTY(Config, EditAnywhere, Category = "Settings",
		Meta = (DisplayName = "Enable iOS Crash Reporting", ToolTip = "Flag indicating whether to capture crashes on iOS"))
	bool bEnableCrashReportingIos;

	// ---- Desktop: bugsplat-native (Windows, macOS, Linux) -------------------------------------
	// Engine crashes keep going through Unreal's own handler and CrashReportClient (DataRouterUrl).
	// bugsplat-native adds what CrashReportClient cannot do: hang detection driven by a game-thread
	// heartbeat, live captures, feedback and structured reports through BugSplat's presigned
	// uploader, and attributes / attachments that change at any time. Off by default because it
	// needs the runtime shipped in Source/ThirdParty/BugSplatNative/<Platform>.

	UPROPERTY(Config, EditAnywhere, Category = "Desktop (bugsplat-native)",
		Meta = (DisplayName = "Enable bugsplat-native on Desktop", ToolTip = "Start BugSplat's native SDK in Windows, macOS and Linux games: hang detection, live captures (CaptureReport), feedback and structured reports through the presigned uploader, runtime attributes and attachments. Engine crashes still go through CrashReportClient. Requires the runtime in Source/ThirdParty/BugSplatNative/<Platform>."))
	bool bEnableNativeDesktop;

	UPROPERTY(Config, EditAnywhere, Category = "Desktop (bugsplat-native)",
		Meta = (DisplayName = "Upload Policy", ToolTip = "Dialog: BugSplatReporter shows the BugSplat dialog before uploading a native report. Quiet: upload silently. Manual: leave reports on disk for Post Pending Reports.", EditCondition = "bEnableNativeDesktop"))
	EBugSplatUploadPolicy DesktopUploadPolicy;

	UPROPERTY(Config, EditAnywhere, Category = "Desktop (bugsplat-native)",
		Meta = (DisplayName = "Dump Type", ToolTip = "How much process memory a native dump carries. Heap adds private writable memory; Full is every readable region. Enforced against your database's size limit at upload.", EditCondition = "bEnableNativeDesktop"))
	EBugSplatDumpType DesktopDumpType;

	UPROPERTY(Config, EditAnywhere, Category = "Desktop (bugsplat-native)",
		Meta = (DisplayName = "Hang Detection Timeout (ms)", ToolTip = "0 disables hang detection. The plugin sends a heartbeat from the game thread every tick; when it stops for longer than this, bugsplat-native dumps the live process and uploads a hang report. Choose a value longer than your longest expected frame (loading screens, shader compilation).", EditCondition = "bEnableNativeDesktop", ClampMin = "0"))
	int32 DesktopHangDetectionTimeoutMs;

	UPROPERTY(Config, EditAnywhere, Category = "Desktop (bugsplat-native)",
		Meta = (DisplayName = "Hang Policy", ToolTip = "Report: dump, upload and let the game continue. Report And Terminate: also offer the user Wait / Close and end the game on Close.", EditCondition = "bEnableNativeDesktop"))
	EBugSplatHangPolicy DesktopHangPolicy;

	UPROPERTY(Config, EditAnywhere, Category = "Desktop (bugsplat-native)",
		Meta = (DisplayName = "Open Support URL", ToolTip = "Open the support-response URL in the browser after an interactive upload.", EditCondition = "bEnableNativeDesktop"))
	bool bDesktopOpenSupportUrl;
};