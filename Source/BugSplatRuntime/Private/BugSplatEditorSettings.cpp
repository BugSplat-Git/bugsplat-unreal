// Copyright 2023 BugSplat. All Rights Reserved.

#include "BugSplatEditorSettings.h"

UBugSplatEditorSettings::UBugSplatEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BugSplatDatabase = TEXT("");
	BugSplatClientId = TEXT("");
	BugSplatClientSecret = TEXT("");
	BugSplatVersion = TEXT("");
	bEnableCrashReportingIos = true;
	bUploadDebugSymbolsIos = true;
	bEnableCrashReportingAndroid = true;
	bUploadDebugSymbolsAndroid = true;
}