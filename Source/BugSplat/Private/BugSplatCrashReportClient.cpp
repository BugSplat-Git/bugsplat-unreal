// Copyright 2023 BugSplat. All Rights Reserved.

#include "BugSplatCrashReportClient.h"
#include <Developer/DesktopPlatform/Public/DesktopPlatformModule.h>
#include <Modules/BuildVersion.h>
#include <Runtime/Core/Public/Misc/EngineVersionComparison.h>

#include "BugSplatEditorSettings.h"
#include "BugSplatRuntime.h"

FBugSplatCrashReportClient::FBugSplatCrashReportClient()
{

}

FString FBugSplatCrashReportClient::CreateBugSplatEndpointUrl(FString Database, FString App, FString Version)
{
	FStringFormatOrderedArguments args;

	FString Space = " ";
	FString EncodedSpace = "%20";
	args.Add(Database);
	args.Add(App.Replace(*Space, *EncodedSpace));
	args.Add(Version.Replace(*Space, *EncodedSpace));

	return *FString::Format(*BUGSPLAT_ENDPOINT_URL_FORMAT, args);
}

void FBugSplatCrashReportClient::UpdateCrashReportClientIni(FString Database, FString App, FString Version, FString DefaultEngineIniFilePath)
{
	if (!FPaths::FileExists(DefaultEngineIniFilePath))
	{
		FMessageDialog::Debugf(FText::FromString("Could not find DefaultEngine.ini!"));
		return;
	}

	FConfigCacheIni ini(EConfigCacheType::DiskBacked);

	ini.LoadFile(DefaultEngineIniFilePath);

	FString SectionTag = FString(TEXT("CrashReportClient"));

	FString DataRouterUrlConfigTag = FString(TEXT("DataRouterUrl"));
	FString DataRouterUrlValue = CreateBugSplatEndpointUrl(Database, App, Version);

	FString CrashReportClientVersionConfigTag = FString(TEXT("CrashReportClientVersion"));
	FString CrashReportClientVersionValue = FString(TEXT("1.0"));

	ini.SetString(*SectionTag, *DataRouterUrlConfigTag, *DataRouterUrlValue, DefaultEngineIniFilePath);
	ini.SetString(*SectionTag, *CrashReportClientVersionConfigTag, *CrashReportClientVersionValue, DefaultEngineIniFilePath);
}

void FBugSplatCrashReportClient::UpdateEngineSettings()
{
	UBugSplatEditorSettings* RuntimeSettings = FBugSplatRuntimeModule::Get().GetSettings();
	
	if (!RuntimeSettings->HasValidCrashReporterSettings())
	{
		return;
	}

	if (!RuntimeSettings->bUpdateEngineDataRouterUrl)
	{
		return;
	}

	UpdateCrashReportClientIni(
		RuntimeSettings->BugSplatDatabase,
		RuntimeSettings->BugSplatApp,
		RuntimeSettings->BugSplatVersion,
		*GLOBAL_CRASH_REPORT_CLIENT_CONFIG_PATH
	);
}

void FBugSplatCrashReportClient::UpdatePackagedBuildSettings()
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

	UBugSplatEditorSettings* RuntimeSettings = FBugSplatRuntimeModule::Get().GetSettings();

	if (!RuntimeSettings->HasValidCrashReporterSettings())
	{
		FMessageDialog::Debugf(FText::FromString("Invalid settings, update plugin fields and try again."));
		return;
	}

	FString Platforms[] = {
		TEXT("Windows"),
		TEXT("Linux"),
		TEXT("Mac")
	};
	bool FoundAnyValidFolders = false;

	for (auto &Platform : Platforms)
	{
		FString PlatformTarget = GetPackagedBuildPlatformTarget(Platform);
		FString PlatformTargetPath = *FPaths::Combine(PackagedBuildFolderPath, PlatformTarget);

		if (!FPaths::DirectoryExists(PlatformTargetPath))
		{
			continue;
		}

		FoundAnyValidFolders = true;
		FString DefaultEngineIniRelativePath = GetPackagedBuildDefaultEngineIniRelativePath();
		FString DefaultEngineIniFolderPath = *FPaths::Combine(PackagedBuildFolderPath, *PlatformTarget, *DefaultEngineIniRelativePath);
		FString DefaultEngineIniFilePath = *FPaths::Combine(*DefaultEngineIniFolderPath, *INI_FILE_NAME);

		if (!FPaths::DirectoryExists(DefaultEngineIniFolderPath))
		{
			IPlatformFile &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
			PlatformFile.CreateDirectoryTree(*DefaultEngineIniFolderPath);

			if (!FPaths::DirectoryExists(DefaultEngineIniFolderPath))
			{
				FMessageDialog::Debugf(FText::FromString("Failed to create " + DefaultEngineIniFolderPath));
				return;
			}

			CreateEmptyTextFile(DefaultEngineIniFilePath);

			if (!FPaths::FileExists(DefaultEngineIniFilePath))
			{
				FMessageDialog::Debugf(FText::FromString("Failed to create " + DefaultEngineIniFilePath));
				return;
			}
		}

		UpdateCrashReportClientIni(
			RuntimeSettings->BugSplatDatabase,
			RuntimeSettings->BugSplatApp,
			RuntimeSettings->BugSplatVersion,
			DefaultEngineIniFilePath
		);
	}

	if (!FoundAnyValidFolders)
	{
		FMessageDialog::Debugf(FText::FromString("Could not find any packaged build directories to update. Please specify the root directory of a valid packaged build."));
		return;
	}
	
	FMessageDialog::Debugf(FText::FromString("Packaged build settings updated successfully!"));
}

void FBugSplatCrashReportClient::CreateEmptyTextFile(FString FullPath)
{
	FString Empty = FString("");
	FFileHelper::SaveStringToFile(Empty, *FullPath);
}

FString FBugSplatCrashReportClient::GetPackagedBuildPlatformTarget(FString Platform)
{
	return ENGINE_MAJOR_VERSION >= 5 ? Platform : Platform + TEXT("NoEditor");
}

FString FBugSplatCrashReportClient::GetPackagedBuildDefaultEngineIniRelativePath()
{
	if (ENGINE_MAJOR_VERSION == 5)
	{
		return PACKAGED_BUILD_CONFIG_PATH_5;
	}
	else if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 26)
	{
		return PACKAGED_BUILD_CONFIG_PATH_4_26_TO_5;
	}

	return PACKAGED_BUILD_CONFIG_PATH_4_25_AND_OLDER;
}
