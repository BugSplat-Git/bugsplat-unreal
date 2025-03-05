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


// TODO BG Linux uploads

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
#if PLATFORM_WINDOWS
	FString SetCurrentPlatfrom = TEXT("$targetPlatform = $args[0]\n$targetName = $args[1]\n");
	FString TargetPlatformNullGuard = TEXT("if (-not $targetPlatform) {\n    Write-Host 'BugSplat [ERROR]: Symbol upload invocation missing target platform...'\n    exit 1\n}\n");
	FString EditorPlatformGuard = TEXT("$isWindowsTarget = $false\nif ($targetPlatform -eq 'Win64' -or $targetPlatform -eq 'XSX' -or $targetPlatform -eq 'XB1') {\n    $isWindowsTarget = $true\n}\nif (-not $isWindowsTarget) {\n    Write-Host 'BugSplat [INFO]: Non-Windows build detected, skipping Windows symbol uploads...'\n    exit 0\n}\n");

	FString PostBuildStepsConsoleCommandFormat =
		FString(
			"{0}\n"		   // Set Platform
			"{1}\n"		   // Target Platform Null Guard
			"{2}\n"		   // Editor Platform Guard
			"& \"{3}\" "	   // Uploader Path
			"-i {4} "	   // Client ID
			"-s {5} "	   // Client Secret
			"-b {6} "	   // Database
			"-a \"{7}\" "  // Application
			"-v \"{8}\" "  // Version
			"-d \"{9}/$targetPlatform\" " // Output Directory
			"-f \"{10}\" " // File Pattern
		);

	FStringFormatOrderedArguments args;
	args.Add(SetCurrentPlatfrom);
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
#elif PLATFORM_MAC
	FString TargetPlatformCheck = TEXT("if [ -z \"$1\" ]; then\n    echo \"BugSplat [ERROR]: Symbol upload invocation missing target platform...\"\n    exit 1\nfi\n");
	FString PlatformGuard = TEXT("if [ \"$1\" != \"Mac\" ]; then\n    echo \"BugSplat [INFO]: Non-Mac build detected, skipping Mac symbol uploads...\"\n    exit 0\nfi\n");

	FString PostBuildStepsConsoleCommandFormat =
		FString(
			"#!/bin/bash\n"
			"{0}\n"		   // Target Platform Check
			"{1}\n"		   // Platform Guard
			"\"{2}\" "	   // Uploader Path
			"-i {3} "	   // Client ID
			"-s {4} "	   // Client Secret
			"-b {5} "	   // Database
			"-a \"{6}\" "  // Application
			"-v \"{7}\" "  // Version
			"-d \"{8}/$1\" " // Output Directory
			"-f \"{9}\" " // File Pattern
		);

	FStringFormatOrderedArguments args;
	args.Add(TargetPlatformCheck);
	args.Add(PlatformGuard);
	args.Add(BUGSPLAT_SYMBOL_UPLOADER_PATH);
	args.Add(ClientId);
	args.Add(ClientSecret);
	args.Add(Database);
	args.Add(App);
	args.Add(Version);
	args.Add(FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), FString("Binaries")));
	args.Add(FString("**/*.{app,dSYM}"));

	return FString::Format(*PostBuildStepsConsoleCommandFormat, args);
#elif PLATFORM_LINUX
	FString TargetPlatformCheck = TEXT("if [ -z \"$1\" ]; then\n    echo \"BugSplat [ERROR]: Symbol upload invocation missing target platform...\"\n    exit 1\nfi\n");
	FString PlatformGuard = TEXT("if [ \"$1\" != \"Linux\" ]; then\n    echo \"BugSplat [INFO]: Non-Linux build detected, skipping Linux symbol uploads...\"\n    exit 0\nfi\n");

	FString PostBuildStepsConsoleCommandFormat =
		FString(
			"#!/bin/bash\n"
			"{0}\n"          // Target Platform Check
			"{1}\n"          // Platform Guard
			"\"{2}\" "       // Uploader Path
			"-i {3} "        // Client ID
			"-s {4} "        // Client Secret
			"-b {5} "        // Database
			"-a \"{6}\" "    // Application
			"-v \"{7}\" "    // Version
			"-d \"{8}/$1\" " // Output Directory
			"-f \"{9}\" "    // File Pattern
		);

	FStringFormatOrderedArguments args;
	args.Add(TargetPlatformCheck);
	args.Add(PlatformGuard);
	args.Add(BUGSPLAT_SYMBOL_UPLOADER_PATH);
	args.Add(ClientId);
	args.Add(ClientSecret);
	args.Add(Database);
	args.Add(App);
	args.Add(Version);
	args.Add(FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), FString("Binaries")));
	args.Add(FString("**/*.sym"));

	return FString::Format(*PostBuildStepsConsoleCommandFormat, args);
#else
	return TEXT("echo \"BugSplat [INFO]: Symbol uploads not supported on this platform\"");
#endif
}

void FBugSplatSymbols::WriteSymbolUploadScript(FString Contents)
{
	FFileHelper::SaveStringToFile(Contents, *BUGSPLAT_SYMBOL_UPLOAD_SCRIPT_PATH);
}