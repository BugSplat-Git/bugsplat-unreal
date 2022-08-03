// Copyright 2022 BugSplat. All Rights Reserved.

#include "BugSplatEditorSettings.h"

UBugSplatEditorSettings::UBugSplatEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BugSplatDatabase = TEXT("");
	bUploadDebugSymbols = false;
	BugSplatUser = TEXT("");
	BugSplatPassword = TEXT("");
	BugSplatVersion = TEXT("");	
}