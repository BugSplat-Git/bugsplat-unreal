#include "BugSplatSettings.h"
#include <Developer/DesktopPlatform/Public/DesktopPlatformModule.h>
#include <Modules/BuildVersion.h>
#include <Runtime/Core/Public/Misc/EngineVersionComparison.h>

FBugSplatSettings::FBugSplatSettings(FString UProjectFilePath)
{
	FBugSplatSettings::UProjectFilePath = UProjectFilePath;
	LoadSettingsFromConfigFile();
}

void FBugSplatSettings::SetAppName(const FText& NewAppName)
{
	AppName = NewAppName.ToString();
	SaveSettingsToUProject();
}

void FBugSplatSettings::SetVersion(const FText& NewVersion)
{
	Version = NewVersion.ToString();
	SaveSettingsToUProject();
}

void FBugSplatSettings::SetDatabase(const FText& NewDatabase)
{
	Database = NewDatabase.ToString();
	SaveSettingsToUProject();
}

void FBugSplatSettings::SetClientID(const FText& NewClientID)
{
	ClientID = NewClientID.ToString();
	SaveSettingsToUProject();
}

void FBugSplatSettings::SetClientSecret(const FText& NewPassword)
{
	ClientSecret = NewPassword.ToString();
	SaveSettingsToUProject();
}

FString FBugSplatSettings::CreateBugSplatEndpointUrl()
{
	FStringFormatOrderedArguments args;

	args.Add(Database);
	args.Add(AppName);
	args.Add(Version);

	return *FString::Format(*BUGSPLAT_ENDPOINT_URL_FORMAT, args);
}

void FBugSplatSettings::LoadSettingsFromConfigFile()
{
	FString fileText;
	TSharedPtr<FJsonObject> fileJson = MakeShareable(new FJsonObject());
	FFileHelper::LoadFileToString(fileText, *UProjectFilePath);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(fileText);
	FJsonSerializer::Deserialize(Reader, fileJson);

	FString outString;

	if (!fileJson->TryGetStringField(DATABASE_TAG, outString))
	{
		FString empty = FString("");
		Database = empty;
		AppName = empty;
		Version = empty;
		ClientID = empty;
		ClientSecret = empty;
		return;
	}

	Database = fileJson->GetStringField(DATABASE_TAG);
	AppName = fileJson->GetStringField(APP_NAME_TAG);
	Version = fileJson->GetStringField(VERSION_TAG);
	ClientID = fileJson->GetStringField(CLIENT_ID_TAG);
	ClientSecret = fileJson->GetStringField(CLIENT_SECRET_TAG);
}

void FBugSplatSettings::UpdateCrashReportClientIni(FString IniFilePath)
{
	if (!FPaths::FileExists(IniFilePath))
	{
		FMessageDialog::Debugf(FText::FromString("Could not find DefaultEngine.ini!"));
		return;
	}

	FConfigCacheIni ini(EConfigCacheType::DiskBacked);

	ini.LoadFile(IniFilePath);

	FString sectionTag = FString(TEXT("CrashReportClient"));

	FString dataRouterUrlConfigTag = FString(TEXT("DataRouterUrl"));
	FString dataRouterUrlValue = CreateBugSplatEndpointUrl();

	FString crashReportClientVersionConfigTag = FString(TEXT("CrashReportClientVersion"));
	FString crashReportClientVersionValue = FString(TEXT("1.0"));

	ini.SetString(*sectionTag, *dataRouterUrlConfigTag, *dataRouterUrlValue, IniFilePath);
	ini.SetString(*sectionTag, *crashReportClientVersionConfigTag, *crashReportClientVersionValue, IniFilePath);

	FMessageDialog::Debugf(FText::FromString("Configuration File Successfully Updated!"));
}

void FBugSplatSettings::SaveSettingsToUProject()
{
	FString fileText;

	TSharedPtr<FJsonObject> fileJson = MakeShareable(new FJsonObject());
	FFileHelper::LoadFileToString(fileText, *UProjectFilePath);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(fileText);
	FJsonSerializer::Deserialize(Reader, fileJson);

	FString Text;
	TSharedRef<FJsonObject> PluginJsonObject = fileJson.ToSharedRef();
	TSharedRef< TJsonWriter<> > WriterRef = TJsonWriterFactory<>::Create(&Text);
	TJsonWriter<>& Writer = WriterRef.Get();

	PluginJsonObject->SetStringField(DATABASE_TAG, Database);
	PluginJsonObject->SetStringField(APP_NAME_TAG, AppName);
	PluginJsonObject->SetStringField(VERSION_TAG, Version);
	PluginJsonObject->SetStringField(CLIENT_ID_TAG, ClientID);	
	PluginJsonObject->SetStringField(CLIENT_SECRET_TAG, ClientSecret);

	FJsonSerializer::Serialize(PluginJsonObject, Writer);
	FFileHelper::SaveStringToFile(Text, *UProjectFilePath);
}

void FBugSplatSettings::WriteSendPdbsToScript()
{
	FString PostBuildStepsConsoleCommandFormat =
		FString(
			"\"{0}\" " //Send PDBS Endpoint
			"/u {1} " // Username
			"/p {2} " // Password
			"/a {3} " // AppName
			"/v {4} " // Version
			"/b {5} " // Database
			"/d \"{6}\"" // Project Directory
		);

	FStringFormatOrderedArguments args;

	args.Add(BUGSPLAT_SENDPDBS_DIR);
	args.Add(ClientID);
	args.Add(ClientSecret);
	args.Add(AppName);
	args.Add(Version);
	args.Add(Database);
	args.Add(FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), FString("/Binaries/%1")));

	FString FormattedString = *FString::Format(*PostBuildStepsConsoleCommandFormat, args);
	FFileHelper::SaveStringToFile(FormattedString, *BUGSPLAT_BASH_DIR);
	FMessageDialog::Debugf(FText::FromString("Symbol uploads added successfully!"));
}

void FBugSplatSettings::UpdateGlobalIni()
{
	UpdateCrashReportClientIni(*GLOBAL_CRASH_REPORT_CLIENT_CONFIG_PATH);
}

void FBugSplatSettings::UpdateLocalIni()
{
	FString PackagedBuildFolderPath;

	if (!FDesktopPlatformModule::Get()->OpenDirectoryDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		FString("Packaged Directory"),
		FPaths::GetProjectFilePath(),
		PackagedBuildFolderPath))
	{
		return;
	}

	FString PackagedBuildConfigPath;
	if (ENGINE_MAJOR_VERSION == 5)
	{
		PackagedBuildConfigPath = PACKAGED_BUILD_CONFIG_PATH_5;
	}
	else if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26)
	{
		PackagedBuildConfigPath = PACKAGED_BUILD_CONFIG_PATH_4_26_TO_5;
	}
	else
	{
		PackagedBuildConfigPath = PACKAGED_BUILD_CONFIG_PATH_4_25_AND_OLDER;
	}

	FString PackagedBuildRoot = ENGINE_MAJOR_VERSION >= 5 ? PACKAGED_BUILD_ROOT_5 : PACKAGED_BUILD_ROOT_OLDER_THAN_5;

	FString Path = *FPaths::Combine(PackagedBuildFolderPath, PackagedBuildRoot);
	if (!FPaths::DirectoryExists(*FPaths::Combine(PackagedBuildFolderPath, PackagedBuildRoot)))
	{
		FMessageDialog::Debugf(FText::FromString("Invalid Project Directory!"));
		return;
	}

	FString ConfigDirectory = *FPaths::Combine(PackagedBuildFolderPath, *PackagedBuildRoot, *PackagedBuildConfigPath);
	FString IniFilePath = *FPaths::Combine(*ConfigDirectory, *INI_FILE_NAME);

	if (!FPaths::DirectoryExists(ConfigDirectory))
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*ConfigDirectory);
		if (!FPaths::DirectoryExists(ConfigDirectory))
		{
			FMessageDialog::Debugf(FText::FromString("Failed to create project Ini directories!"));
			return;
		}

		// Creates an empty .ini file with nothing inside of it.
		FString Empty = FString("");
		FFileHelper::SaveStringToFile(Empty, *IniFilePath );
		
		if (!FPaths::FileExists(IniFilePath))
		{
			FMessageDialog::Debugf(FText::FromString("Failed to create ini config file!"));
			return;
		}
	}

	UpdateCrashReportClientIni(IniFilePath);
}	
