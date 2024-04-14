// Copyright 2023 BugSplat. All Rights Reserved.

#pragma once	

#include "CoreMinimal.h"
#include <EngineSharedPCH.h>
#include "Interfaces/IPluginManager.h"

static const FString PLUGIN_BASE_DIR = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("BugSplat"))->GetBaseDir());
static const FString BUGSPLAT_SYMBOL_UPLOADER_PATH = *FPaths::Combine(PLUGIN_BASE_DIR, FString("/Source/ThirdParty/SymUploader/symbol-upload-windows.exe"));
static const FString BUGSPLAT_BASH_DIR = *FPaths::Combine(FPaths::ProjectDir(), FString("Plugins/BugSplat/Source/Scripts/BugSplat.bat"));

class FBugSplatSymbols
{
public:
	FBugSplatSymbols();

	void UpdateSymbolUploadsSettings();
	FString CreateSymbolUploadScript(FString Database, FString App, FString Version, FString ClientId, FString ClientSecret);

private:
	void WriteSymbolUploadScript(FString contents);
};
