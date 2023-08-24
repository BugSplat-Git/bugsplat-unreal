// Copyright 2023 BugSplat. All Rights Reserved.

#include "BugSplatUtils.h"

void UBugSplatUtils::GenerateCrash()
{
	char *ptr = 0;
	*ptr += 1;
}

void UBugSplatUtils::GenerateAssert()
{
	char *ptr = nullptr;
	check(ptr != nullptr);
}
