// Copyright 2023 BugSplat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <Runtime/SlateCore/Public/Widgets/SUserWidget.h>
#include "BugSplatCrashReportClient.h"
#include "BugSplatSymbols.h"

class FBugSplatModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FBugSplatModule& Get();

	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

	FReply OnUpdateGlobalIni();
	FReply OnUpdateLocalIni();
	FReply OnUpdateWindowsSymbolUploadScript();

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	FBugSplatCrashReportClient* BugSplatSettings;
	FBugSplatSymbols* BugSplatSymbols;
};
