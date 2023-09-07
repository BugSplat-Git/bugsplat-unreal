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
	FString SetCurrentPlatfrom = TEXT("@echo off\nset targetPlatform=%1\nset targetName=%2\n");
	FString TargetPlatformNullGuard = TEXT("if \"%targetPlatform%\"==\"\" (\n\techo \"BugSplat [ERROR]: Symbol upload invocation missing target platform...\"\n\texit /b\n)");
	FString EditorPlatformGuard = TEXT("set isEditor=0\nif \"%targetPlatform%\"==\"Editor\" set isEditor=1\nif \"%targetPlatform%\"==\"WindowsEditor\" set isEditor=1\nif \"%targetPlatform%\"==\"DevelopmentEditor\" set isEditor=1\nif \"%targetName:~-6%\"==\"Editor\" set isEditor=1\nif %isEditor%==1 (\n\techo \"BugSplat [INFO]: Editor build detected, skipping symbol uploads...\"\n\texit /b\n)");

	FString PostBuildStepsConsoleCommandFormat =
		FString(
			"{0}\n"		   // Set Platform
			"{1}\n"		   // Target Platform Null Guard
			"{2}\n"		   // Editor Platform Guard
			"\"{3}\" "	   // Uploader Path
			"-i {4} "	   // Client ID
			"-s {5} "	   // Client Secret
			"-b {6} "	   // Database
			"-a \"{7}\" "  // Application
			"-v \"{8}\" "  // Version
			"-d \"{9}/%targetPlatform%\" " // Output Directory
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
	args.Add(FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), "Binaries"));
	args.Add(FString("**/*.{pdb,dll,exe}"));
	
	return FString::Format(*PostBuildStepsConsoleCommandFormat, args);
}

void FBugSplatSymbols::WriteSymbolUploadScript(FString Contents)
{
	FFileHelper::SaveStringToFile(Contents, *BUGSPLAT_BASH_DIR);
}