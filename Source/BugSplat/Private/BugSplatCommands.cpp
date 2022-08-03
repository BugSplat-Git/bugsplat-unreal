// Copyright 2022 BugSplat. All Rights Reserved.

#include "BugSplatCommands.h"

#define LOCTEXT_NAMESPACE "FBugSplatModule"

void FBugSplatCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "BugSplat", "Bring up BugSplat window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
