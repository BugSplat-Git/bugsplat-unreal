#pragma once

#include "CoreMinimal.h"
#include <EngineSharedPCH.h>

// TODO It feels like these should just go in the class declaration. What is c++ best practice?
static const FString BUGSPLAT_ENDPOINT_URL_FORMAT = FString("https://{0}.bugsplat.com/post/ue4/{1}/{2}");
static const FString BUGSPLAT_SENDPDBS_DIR = FString("\\Plugins\\BugSplat\\Source\\BugSplatSendPdbs\\bin\\SendPdbs.exe");
static const FString POST_BUILD_STEPS_CONSOLE_COMMAND_FORMAT = 
FString(
	"call \"$(ProjectDir){0}\" " //Send PDBS Endpoint
	"/u {1} " // Username
	"/p {2} " // Password
	"/a {3} " // AppName
	"/v {4} " // Version
	"/b {5} " // Database
	"/d \"$(ProjectDir)\\Binaries\\$(TargetPlatform)\"" // Project Directory
);

static const FString LOCAL_CONFIG_PATH = *FPaths::Combine(FPaths::ProjectDir(), FString("/Config/DefaultEngine.ini"));
static const FString GLOBAL_CRASH_REPORT_CLIENT_CONFIG_PATH = *FPaths::Combine(FPaths::EngineDir(), FString("/Programs/CrashReportClient/Config/DefaultEngine.ini"));
static const FString PACKAGED_BUILD_CONFIG_PATH = FString("WindowsNoEditor\\Engine\\Programs\\CrashReportClient\\Config\\DefaultEngine.ini");
static const FString BUGSPLAT_UPROJECT_PATH = *FPaths::Combine(FPaths::ProjectDir(), FString("/Plugins/BugSplat/BugSplat.uplugin"));

static const FString DATABASE_TAG = FString("Database");
static const FString APP_NAME_TAG = FString("AppName");
static const FString VERSION_TAG = FString("AppVersion"); // "Version" is reserved for plugin version.
static const FString USERNAME_TAG = FString("Username");
static const FString PASSWORD_TAG = FString("Password");
static const FString USE_GLOBAL_INI_TAG = FString("UseGlobalIni");

static const FString WIN_64_LABEL = FString("Win64");
static const FString POST_BUILD_STEPS_LABEL = FString("PostBuildSteps");

class FBugSplatSettings
{
public:
	FBugSplatSettings(FString uProjectFilePath);

	void SetAppName(const FText& appName);
	void SetVersion(const FText& version);
	void SetDatabase(const FText& database);
	void SetUsername(const FText& username);
	void SetPassword(const FText& password);
	void SetUseGlobalIni(const ECheckBoxState newState);

	FText GetAppName() { return FText::FromString(AppName); };
	FText GetVersion() { return FText::FromString(Version); };
	FText GetDatabase() { return FText::FromString(Database); };
	FText GetUsername() { return FText::FromString(Username); };
	FText GetPassword() { return FText::FromString(Password); };
	bool GetUseGlobalIni() { return bUseGlobalIni; };
	ECheckBoxState GetUseGlobalIniCheckboxState() const { return bUseGlobalIni ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; };

	FString BuildBugSplatEndpointUrl();
	FString BuildPostBuildStepsConsoleCommand();

	void PackageWithBugSplat();

	void Save();

private:
	void LoadSettingsFromConfigFile();

	void UpdateCrashReportClientIni(FString iniFilePath);
	void SaveSettingsToUProject();

	void AddPostBuildSteps(TSharedRef<FJsonObject> jsonObject);

	FString UProjectFilePath;

	FString Database;
	FString AppName;
	FString Version;
	FString Username;
	FString Password;
	bool bUseGlobalIni;
};
