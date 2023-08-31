// Copyright 2023 BugSplat. All Rights Reserved.

#pragma once	

#include "CoreMinimal.h"
#include <EngineSharedPCH.h>

static const FString BUGSPLAT_ENDPOINT_URL_FORMAT = FString("https://{0}.bugsplat.com/post/ue4/{1}/{2}");

static const FString GLOBAL_CRASH_REPORT_CLIENT_CONFIG_PATH = *FPaths::Combine(FPaths::EngineDir(), FString("Programs/CrashReportClient/Config/DefaultEngine.ini"));

static const FString PACKAGED_BUILD_CONFIG_PATH_5 = FString("Engine\\Restricted\\NoRedist\\Programs\\CrashReportClient\\Config");
static const FString PACKAGED_BUILD_CONFIG_PATH_4_26_TO_5 = FString("Engine\\Restricted\\NoRedist\\Programs\\CrashReportClient\\Config");
static const FString PACKAGED_BUILD_CONFIG_PATH_4_25_AND_OLDER = FString("Engine\\Programs\\CrashReportClient\\Config\\NoRedist");

static const FString INI_FILE_NAME = FString("DefaultEngine.ini");

class FBugSplatCrashReportClient
{
public:
	FBugSplatCrashReportClient();

	void UpdatePackagedBuildSettings();
	void UpdateEngineSettings();

private:
	FString CreateBugSplatEndpointUrl(FString Database, FString App, FString Version);
	void CreateEmptyTextFile(FString FullPath);

	FString GetPackagedBuildPlatformTarget(FString Platform);
	FString GetPackagedBuildDefaultEngineIniRelativePath();

	void UpdateCrashReportClientIni(FString Database, FString App, FString Version, FString DefaultEngineIniFilePath);
};
