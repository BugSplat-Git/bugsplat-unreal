// Copyright 2022 BugSplat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BugSplatEditorSettings.generated.h"

UCLASS(Config = Engine, defaultconfig)
class BUGSPLATRUNTIME_API UBugSplatEditorSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Common", Meta = (DisplayName = "Database", ToolTip = "Database name"))
	FString BugSplatDatabase;

	UPROPERTY(Config, EditAnywhere, Category = "Common", Meta = (DisplayName = "Application", ToolTip = "Application name"))
	FString BugSplatApp;

	UPROPERTY(Config, EditAnywhere, Category = "Common", Meta = (DisplayName = "Version", ToolTip = "Application version number"))
	FString BugSplatVersion;

	UPROPERTY(Config, EditAnywhere, Category = "Common", Meta = (DisplayName = "User", ToolTip = "Username (required for debug symbol uploads)"))
	FString BugSplatUser;

	UPROPERTY(Config, EditAnywhere, Category = "Common", Meta = (DisplayName = "Password", ToolTip = "Password (required for debug symbol uploads)"))
	FString BugSplatPassword;	

	UPROPERTY(Config, EditAnywhere, Category = "IOS",
		Meta = (DisplayName = "Enable iOS crash reporting", ToolTip = "Flag indicating whether to capture crashes on iOS"))
	bool bEnableCrashReportingIos;

	UPROPERTY(Config, EditAnywhere, Category = "IOS",
		Meta = (DisplayName = "Enable automatic symbols uploads", ToolTip = "Flag indicating whether to upload iOS debug symbols automatically", EditCondition = "bEnableCrashReportingIos"))
	bool bUploadDebugSymbolsIos;

	UPROPERTY(Config, EditAnywhere, Category = "Android",
		Meta = (DisplayName = "Enable Android crash reporting", ToolTip = "Flag indicating whether to capture crashes on Android (Android Support Coming Soon)"))
	bool bEnableCrashReportingAndroid;

	UPROPERTY(Config, VisibleAnywhere, Category = "Android",
		Meta = (DisplayName = "Enable automatic symbols uploads", ToolTip = "Flag indicating whether to upload Android debug symbols automatically (Android Support Coming Soon)", EditCondition = "bEnableCrashReportingAndroid"))
	bool bUploadDebugSymbolsAndroid;
};