// Copyright 2022 BugSplat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UBugSplatEditorSettings;

class BUGSPLATRUNTIME_API FBugSplatRuntimeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FBugSplatRuntimeModule& Get();

	UBugSplatEditorSettings* GetSettings() const;

private:
	void SetupCrashReportingIos();
	void SetupCrashReportingAndroid();

	UBugSplatEditorSettings* BugSplatEditorSettings;
};
