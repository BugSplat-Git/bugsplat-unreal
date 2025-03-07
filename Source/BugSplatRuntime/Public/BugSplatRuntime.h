// Copyright 2023 BugSplat. All Rights Reserved.

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

	/** Gets a map of crash attributes to include in crash reports */
	TMap<FString, FString> GetCrashAttributes() const;

private:
	void SetupCrashReportingIos();
	void SetupCrashReportingAndroid();

	UBugSplatEditorSettings* BugSplatEditorSettings;
};
