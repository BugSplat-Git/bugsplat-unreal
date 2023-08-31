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

class FBugSplatSettings
{
public:
	FBugSplatSettings();

	FString CreateBugSplatEndpointUrl();

	void UpdateLocalIni();
	void UpdateGlobalIni();

private:
	FString GetPackagedBuildPlatformTarget(FString Platform);
	FString GetPackagedBuildDefaultEngineIniRelativePath();
	void CreateEmptyTextFile(FString FullPath);
	void UpdateCrashReportClientIni(FString iniFilePath);
};
