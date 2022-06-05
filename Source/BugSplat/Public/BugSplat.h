#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <Runtime/SlateCore/Public/Widgets/SUserWidget.h>
#include "BugSplatSettings.h"

class FToolBarBuilder;
class FMenuBuilder;

class FBugSplatModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

	FReply OnSettingsSaved();
	FReply OnPackageWithBugSplat();
	
private:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	FBugSplatSettings* BugSplatSettings;
};
