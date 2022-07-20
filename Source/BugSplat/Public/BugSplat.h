#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <Runtime/SlateCore/Public/Widgets/SUserWidget.h>
#include "BugSplatSettings.h"

class FToolBarBuilder;
class FMenuBuilder;
class UBugSplatEditorSettings;

enum BugSplatInputField
{
	ClientID,
	ClientSecret,
	Database,
	ApplicationName,
	Version
};

class FBugSplatModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

	FReply OnUpdateGlobalIni();
	FReply OnUpdateLocalIni();
	FReply OnUpdateSendPdbsShell();
	
private:

	void RegisterMenus();

	void RegisterEditorSettingsPanel();
	void UnregisterEditorSettingsPanel();

	TSharedRef<class SBox> CreateBugSplatWindowContent();
	TSharedPtr<SVerticalBox> CreateInputFieldWidget(FText InputFieldName, BugSplatInputField InputFieldType);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
	FBugSplatSettings* BugSplatSettings;

	UBugSplatEditorSettings* BugsplatEditoSettings;
};
