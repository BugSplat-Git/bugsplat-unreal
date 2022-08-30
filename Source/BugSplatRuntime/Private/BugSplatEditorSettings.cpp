// Copyright 2022 BugSplat. All Rights Reserved.

#include "BugSplatEditorSettings.h"

UBugSplatEditorSettings::UBugSplatEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BugSplatDatabase = TEXT("");
	BugSplatUser = TEXT("");
	BugSplatPassword = TEXT("");
	BugSplatVersion = TEXT("");
	bEnableCrashReportingIos = true;
	bUploadDebugSymbolsIos = true;
	bEnableCrashReportingAndroid = false;
	bUploadDebugSymbolsAndroid = false;
}