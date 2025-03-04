// Copyright 2023 BugSplat. All Rights Reserved.

#include "BugSplatSymbols.h"
#include <Developer/DesktopPlatform/Public/DesktopPlatformModule.h>
#include <Modules/BuildVersion.h>
#include <Runtime/Core/Public/Misc/EngineVersionComparison.h>

#include "BugSplatEditorSettings.h"
#include "BugSplatRuntime.h"

FBugSplatSymbols::FBugSplatSymbols()
{

}


// TODO BG Mac/Linux uploads

void FBugSplatSymbols::UpdateSymbolUploadsSettings()
{
	FString SymbolUploadScript = TEXT("echo \"BugSplat [INFO]: Symbol uploads not configured, skipping...\"");

	UBugSplatEditorSettings* RuntimeSettings = FBugSplatRuntimeModule::Get().GetSettings();

	if (RuntimeSettings->HasValidSymbolUploadSettings() && RuntimeSettings->bUploadDebugSymbols)
	{
		SymbolUploadScript = CreateSymbolUploadScript(
			RuntimeSettings->BugSplatDatabase,
			RuntimeSettings->BugSplatApp,
			RuntimeSettings->BugSplatVersion,
			RuntimeSettings->BugSplatClientId,
			RuntimeSettings->BugSplatClientSecret
		);
	}

	WriteSymbolUploadScript(SymbolUploadScript);
}

FString FBugSplatSymbols::CreateSymbolUploadScript(FString Database, FString App, FString Version, FString ClientId, FString ClientSecret)
{
	FString SetCurrentPlatfrom = TEXT("$targetPlatform = $args[0]\n$targetName = $args[1]\n");
	FString EditorBuildCheck = TEXT("if ($targetName -like '*Editor*') {\n    Write-Host 'BugSplat [WARN]: Skipping symbol upload for Editor build...'\n    exit 0\n}\n");
	FString TargetPlatformNullGuard = TEXT("if (-not $targetPlatform) {\n    Write-Host 'BugSplat [ERROR]: Symbol upload invocation missing target platform...'\n    exit 1\n}\n");
	FString EditorPlatformGuard = TEXT("$isWindows = $false\nif ($targetPlatform -eq 'Win64' -or $targetPlatform -eq 'XSX' -or $targetPlatform -eq 'XB1') {\n    $isWindows = $true\n}\nif (-not $isWindows) {\n    Write-Host 'BugSplat [INFO]: Non-Windows build detected, skipping Windows symbol uploads...'\n    exit 0\n}\n");

	FString PostBuildStepsConsoleCommandFormat =
		FString(
			"{0}\n"		   // Set Platform
			"{1}\n"		   // Editor Build Check
			"{2}\n"		   // Target Platform Null Guard
			"{3}\n"		   // Editor Platform Guard
			"& \"{4}\" "	   // Uploader Path
			"-i {5} "	   // Client ID
			"-s {6} "	   // Client Secret
			"-b {7} "	   // Database
			"-a \"{8}\" "  // Application
			"-v \"{9}\" "  // Version
			"-d \"{10}/$targetPlatform\" " // Output Directory
			"-f \"{11}\" " // File Pattern
		);

	FStringFormatOrderedArguments args;
	args.Add(SetCurrentPlatfrom);
	args.Add(EditorBuildCheck);
	args.Add(TargetPlatformNullGuard);
	args.Add(EditorPlatformGuard);
	args.Add(BUGSPLAT_SYMBOL_UPLOADER_PATH);
	args.Add(ClientId);
	args.Add(ClientSecret);
	args.Add(Database);
	args.Add(App);
	args.Add(Version);
	args.Add(FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), FString("Binaries")));
	args.Add(FString("**/*.{pdb,dll,exe}"));
	
	return FString::Format(*PostBuildStepsConsoleCommandFormat, args);
}

void FBugSplatSymbols::WriteSymbolUploadScript(FString Contents)
{
	FFileHelper::SaveStringToFile(Contents, *BUGSPLAT_SYMBOL_UPLOAD_SCRIPT_PATH);
}