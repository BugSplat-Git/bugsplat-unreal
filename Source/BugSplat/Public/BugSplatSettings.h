#pragma once

#include "CoreMinimal.h"
#include <EngineSharedPCH.h>

static const FString BUGSPLAT_ENDPOINT_URL_FORMAT = FString("https://{0}.bugsplat.com/post/ue4/{1}/{2}");
static const FString BUGSPLAT_SENDPDBS_DIR = *FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), FString("/Plugins/BugSplat/Source/Scripts/bin/SendPdbs.exe"));
static const FString BUGSPLAT_BASH_DIR = *FPaths::Combine(FPaths::ProjectDir(), FString("/Plugins/BugSplat/Source/Scripts/BugSplat.bat"));

static const FString GLOBAL_CRASH_REPORT_CLIENT_CONFIG_PATH = *FPaths::Combine(FPaths::EngineDir(), FString("/Programs/CrashReportClient/Config/DefaultEngine.ini"));
static const FString BUGSPLAT_UPROJECT_PATH = *FPaths::Combine(FPaths::ProjectDir(), FString("/Plugins/BugSplat/BugSplat.uplugin"));

static const FString PACKAGED_BUILD_CONFIG_PATH_5 = FString("Engine\\Restricted\\NoRedist\\Programs\\CrashReportClient\\Config");
static const FString PACKAGED_BUILD_CONFIG_PATH_4_26_TO_5 = FString("Engine\\Restricted\\NoRedist\\Programs\\CrashReportClient\\Config");
static const FString PACKAGED_BUILD_CONFIG_PATH_4_25_AND_OLDER = FString("Engine\\Programs\\CrashReportClient\\Config\\NoRedist");

static const FString INI_FILE_NAME = FString("DefaultEngine.ini");

static const FString DATABASE_TAG = FString("Database");
static const FString APP_NAME_TAG = FString("AppName");
static const FString VERSION_TAG = FString("AppVersion"); // "Version" is reserved for plugin version.
static const FString CLIENT_ID_TAG = FString("ClientID");
static const FString CLIENT_SECRET_TAG = FString("ClientSecret");

static const FString WIN_64_LABEL = FString("Win64");
static const FString POST_BUILD_STEPS_LABEL = FString("PostBuildSteps");

class FBugSplatSettings
{
public:
	FBugSplatSettings(FString uProjectFilePath);

	void SetAppName(const FText& AppName);
	void SetVersion(const FText& Version);
	void SetDatabase(const FText& Database);
	void SetClientID(const FText& ClientID);
	void SetClientSecret(const FText& ClientSecret);

	FText GetAppName() { return FText::FromString(AppName); };
	FText GetVersion() { return FText::FromString(Version); };
	FText GetDatabase() { return FText::FromString(Database); };
	FText GetClientID() { return FText::FromString(ClientID); };
	FText GetClientSecret() { return FText::FromString(ClientSecret); };

	FString CreateBugSplatEndpointUrl();

	void UpdateLocalIni();
	void UpdateGlobalIni();
	void WriteSendPdbsToScript();

private:
	FString GetPackagedBuildPlatformTarget(FString Platform);
	FString GetPackagedBuildDefaultEngineIniRelativePath();
	void CreateEmptyTextFile(FString FullPath);
	void LoadSettingsFromConfigFile();
	void UpdateCrashReportClientIni(FString iniFilePath);
	void SaveSettingsToUProject();

	FString UProjectFilePath;

	FString Database;
	FString AppName;
	FString Version;
	FString ClientID;
	FString ClientSecret;

};
