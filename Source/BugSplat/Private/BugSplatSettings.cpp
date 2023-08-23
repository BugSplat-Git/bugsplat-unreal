// Copyright 2022 BugSplat. All Rights Reserved.

#include "BugSplatSettings.h"
#include <Developer/DesktopPlatform/Public/DesktopPlatformModule.h>
#include <Modules/BuildVersion.h>
#include <Runtime/Core/Public/Misc/EngineVersionComparison.h>

#include "BugSplatEditorSettings.h"
#include "BugSplatRuntime.h"

FBugSplatSettings::FBugSplatSettings()
{

}

FString FBugSplatSettings::CreateBugSplatEndpointUrl()
{
	FStringFormatOrderedArguments args;

	UBugSplatEditorSettings* runtimeSettings = FBugSplatRuntimeModule::Get().GetSettings();

	FString Space = " ";
	FString EncodedSpace = "%20";
	args.Add(runtimeSettings->BugSplatDatabase);
	args.Add(runtimeSettings->BugSplatApp.Replace(*Space, *EncodedSpace));
	args.Add(runtimeSettings->BugSplatVersion.Replace(*Space, *EncodedSpace));

	return *FString::Format(*BUGSPLAT_ENDPOINT_URL_FORMAT, args);
}

void FBugSplatSettings::UpdateCrashReportClientIni(FString DefaultEngineIniFilePath)
{
	if (!FPaths::FileExists(DefaultEngineIniFilePath))
	{
		FMessageDialog::Debugf(FText::FromString("Could not find DefaultEngine.ini!"));
		return;
	}

	FConfigCacheIni ini(EConfigCacheType::DiskBacked);

	ini.LoadFile(DefaultEngineIniFilePath);

	FString sectionTag = FString(TEXT("CrashReportClient"));

	FString dataRouterUrlConfigTag = FString(TEXT("DataRouterUrl"));
	FString dataRouterUrlValue = CreateBugSplatEndpointUrl();

	FString crashReportClientVersionConfigTag = FString(TEXT("CrashReportClientVersion"));
	FString crashReportClientVersionValue = FString(TEXT("1.0"));

	ini.SetString(*sectionTag, *dataRouterUrlConfigTag, *dataRouterUrlValue, DefaultEngineIniFilePath);
	ini.SetString(*sectionTag, *crashReportClientVersionConfigTag, *crashReportClientVersionValue, DefaultEngineIniFilePath);

	FMessageDialog::Debugf(FText::FromString("Configuration File Successfully Updated!"));
}

void FBugSplatSettings::WriteSymbolUploadScript()
{
	FString SetCurrentPlatfrom = FString("@echo off\nset targetPlatform=%1");
	FString TargetPlatformNullGuard = FString("if \"%targetPlatform%\"==\"\" (\n\techo \"BugSplat [ERROR]: symbol upload invocation missing target platform...\"\n\texit /b\n)");
	FString EditorPlatformGuard = FString("if \"%targetPlatform%\"==\"Editor\" (\n\techo \"BugSplat [INFO]: Editor build detected, skipping symbol uploads...\"\n\texit /b\n)");
	UBugSplatEditorSettings* RuntimeSettings = FBugSplatRuntimeModule::Get().GetSettings();

	FString PostBuildStepsConsoleCommandFormat =
		FString(
			"{0}\n"		  // Set Platform
			"{1}\n"		  // Target Platform Null Guard
			"{2}\n"		  // Editor Platform Guard
			"\"{3}\" "	  // Uploader Path
			"-i {4} "	  // Client ID
			"-s {5} "	  // Client Secret
			"-b {6} "	  // Database
			"-a \"{7}\" " // Application
			"-v \"{8}\" " // Version
			"-d \"{9}\" " // Project Directory
			"-f \"{10}\" " // File Pattern
		);

	FStringFormatOrderedArguments args;
	args.Add(SetCurrentPlatfrom);
	args.Add(TargetPlatformNullGuard);
	args.Add(EditorPlatformGuard);
	args.Add(BUGSPLAT_SYMBOL_UPLOADER_PATH);
	args.Add(RuntimeSettings->BugSplatClientId);
	args.Add(RuntimeSettings->BugSplatClientSecret);
	args.Add(RuntimeSettings->BugSplatDatabase);
	args.Add(RuntimeSettings->BugSplatApp);
	args.Add(RuntimeSettings->BugSplatVersion);
	args.Add(FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), "Binaries"));
	args.Add("**/*.{pdb,dll,exe}");
	
	FString UploadSymbolsScript = FString::Format(*PostBuildStepsConsoleCommandFormat, args);
	FFileHelper::SaveStringToFile(UploadSymbolsScript, *BUGSPLAT_BASH_DIR);
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

		UpdateCrashReportClientIni(DefaultEngineIniFilePath);
	}

	if (!FoundAnyValidFolders)
	{
		FMessageDialog::Debugf(FText::FromString("Could not find any packaged build directories to update. Please specify the root directory of a valid packaged build."));
	}
}

void FBugSplatSettings::CreateEmptyTextFile(FString FullPath)
{
	FString Empty = FString("");
	FFileHelper::SaveStringToFile(Empty, *FullPath);
}

FString FBugSplatSettings::GetPackagedBuildPlatformTarget(FString Platform)
{
	return ENGINE_MAJOR_VERSION >= 5 ? Platform : Platform + TEXT("NoEditor");
}

FString FBugSplatSettings::GetPackagedBuildDefaultEngineIniRelativePath()
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
