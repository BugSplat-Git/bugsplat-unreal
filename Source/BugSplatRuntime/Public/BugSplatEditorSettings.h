// Copyright 2023 BugSplat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
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
};