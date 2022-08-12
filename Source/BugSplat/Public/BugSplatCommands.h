// Copyright 2022 BugSplat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BugSplatStyle.h"

class FBugSplatCommands : public TCommands<FBugSplatCommands>
{
public:

	FBugSplatCommands()
		: TCommands<FBugSplatCommands>(TEXT("BugSplat"), NSLOCTEXT("Contexts", "BugSplat", "BugSplat Plugin"), NAME_None, FBugSplatStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};