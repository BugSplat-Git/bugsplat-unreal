// Copyright 2023 BugSplat. All Rights Reserved.

#pragma once	

#include "CoreMinimal.h"
#include <EngineSharedPCH.h>
#include "Interfaces/IPluginManager.h"

static const FString PLUGIN_BASE_DIR = FPaths::ConvertRelativePathToFull(IPluginManager::Get().FindPlugin(TEXT("BugSplat"))->GetBaseDir());

#if PLATFORM_MAC
static const FString BUGSPLAT_SYMBOL_UPLOADER_PATH = *FPaths::Combine(PLUGIN_BASE_DIR, FString("/Source/ThirdParty/SymUploader/symbol-upload-macos"));
static const FString BUGSPLAT_SYMBOL_UPLOAD_SCRIPT_PATH = *FPaths::Combine(FPaths::ProjectDir(), FString("Plugins/BugSplat/Source/Scripts/upload-symbols-mac.sh"));
#elif PLATFORM_LINUX
static const FString BUGSPLAT_SYMBOL_UPLOADER_PATH = *FPaths::Combine(PLUGIN_BASE_DIR, FString("/Source/ThirdParty/SymUploader/symbol-upload-linux"));
static const FString BUGSPLAT_SYMBOL_UPLOAD_SCRIPT_PATH = *FPaths::Combine(FPaths::ProjectDir(), FString("Plugins/BugSplat/Source/Scripts/upload-symbols-linux.sh"));
#else
static const FString BUGSPLAT_SYMBOL_UPLOADER_PATH = *FPaths::Combine(PLUGIN_BASE_DIR, FString("/Source/ThirdParty/SymUploader/symbol-upload-windows.exe"));
static const FString BUGSPLAT_SYMBOL_UPLOAD_SCRIPT_PATH = *FPaths::Combine(FPaths::ProjectDir(), FString("Plugins/BugSplat/Source/Scripts/upload-symbols-win64.ps1"));
#endif

class FBugSplatSymbols
{
public:
	FBugSplatSymbols();

	void UpdateSymbolUploadsSettings();
	FString CreateSymbolUploadScript(FString Database, FString App, FString Version, FString ClientId, FString ClientSecret);

private:
	void WriteSymbolUploadScript(FString contents);
};
