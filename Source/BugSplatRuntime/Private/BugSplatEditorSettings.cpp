#include "BugSplatEditorSettings.h"

UBugSplatEditorSettings::UBugSplatEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BugSplatDatabase = TEXT("");
	BugSplatUser = TEXT("");
	BugSplatPassword = TEXT("");
}