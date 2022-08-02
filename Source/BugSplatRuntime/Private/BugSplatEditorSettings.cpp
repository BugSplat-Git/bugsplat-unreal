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