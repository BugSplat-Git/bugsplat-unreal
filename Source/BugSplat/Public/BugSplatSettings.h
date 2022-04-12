#pragma once

#include "CoreMinimal.h"
#include <EngineSharedPCH.h>

// TODO It feels like these should just go in the class declaration. What is c++ best practice?
static const FString BUGSPLAT_ENDPOINT_URL_FORMAT = FString("https://{0}.bugsplat.com/post/ue4/{1}/{2}");
static const FString POST_BUILD_STEPS_CONSOLE_COMMAND_FORMAT =
FString(
	"call \"$(ProjectDir)\\Plugins\\BugSplat\\Source\\BugSplatSendPdbs\\bin\\SendPdbs.exe\" "
	"/ u {0} " // Username
	"/ p {1} " // Password
	"/ a {2} " // AppName
	"/ v {3} " // Version
	"/ b {4} " // Database
	"/ d \"$(ProjectDir)\\Binaries\\$(TargetPlatform)\"" // Project Directory
);

static const FString LOCAL_CONFIG_PATH = *FPaths::Combine(FPaths::ProjectDir(), FString("/Config/DefaultEngine.ini"));
static const FString BUGSPLAT_UPROJECT_PATH = *FPaths::Combine(FPaths::ProjectDir(), FString("/Plugins/BugSplat/BugSplat.uplugin"));

static const FString DATABASE_TAG = FString("Database");
static const FString APP_NAME_TAG = FString("AppName");
static const FString VERSION_TAG = FString("AppVersion"); // "Version" is reserved for plugin version.
static const FString USERNAME_TAG = FString("Username");
static const FString PASSWORD_TAG = FString("Password");

class BugSplatSettings
{
public:
	BugSplatSettings(FString configFilePath, FString uProjectFilePath);

	void setAppName(const FText& appName);
	void setVersion(const FText& version);
	void setDatabase(const FText& database);
	void setUsername(const FText& username);
	void setPassword(const FText& password);

	// We should probably just make all these values public...
	FText getAppName() { return FText::FromString(_appName); };
	FText getVersion() { return FText::FromString(_version); };
	FText getDatabase() { return FText::FromString(_database); };
	FText getUsername() { return FText::FromString(_username); };
	FText getPassword() { return FText::FromString(_password); };

	FString buildEndpointUrl();
	FString buildPostBuildStepsConsoleCommand();

	void save();

private:
	void loadSettingsFromConfigFile();

	void saveSettingsToConfigFile();
	void saveSettingsToUProject();

	void addPostBuildSteps(TSharedRef<FJsonObject> jsonObject);

	FString _configFilePath;
	FString _uProjectFilePath;

	FString _database;
	FString _appName;
	FString _version;
	FString _username;
	FString _password;
};
