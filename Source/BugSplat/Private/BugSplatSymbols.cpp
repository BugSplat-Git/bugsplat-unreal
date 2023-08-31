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

void FBugSplatSymbols::WriteSymbolUploadScript()
{
	FString SetCurrentPlatfrom = FString("@echo off\nset targetPlatform=%1");
	FString TargetPlatformNullGuard = FString("if \"%targetPlatform%\"==\"\" (\n\techo \"BugSplat [ERROR]: symbol upload invocation missing target platform...\"\n\texit /b\n)");
	FString EditorPlatformGuard = FString("if \"%targetPlatform%\"==\"Editor\" (\n\techo \"BugSplat [INFO]: Editor build detected, skipping symbol uploads...\"\n\texit /b\n)");
	UBugSplatEditorSettings* RuntimeSettings = FBugSplatRuntimeModule::Get().GetSettings();

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
