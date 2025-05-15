// Copyright BugSplat. All Rights Reserved.

#include "BugSplatUtils.h"

void UBugSplatUtils::GenerateCrash()
{
	UE_LOG(LogTemp, Log, TEXT("BugSplat: GenerateCrash"));
	volatile char *ptr = (char*)0xC0FFEEC0;
	*ptr += 1;
}

void UBugSplatUtils::GenerateAssert()
{
	UE_LOG(LogTemp, Log, TEXT("BugSplat: GenerateAssert"));
	char *ptr = nullptr;
	check(ptr != nullptr);
}

void UBugSplatUtils::GenerateEnsure()
{
	UE_LOG(LogTemp, Log, TEXT("BugSplat: GenerateEnsure"));
	char* ptr = nullptr;
	ensure(ptr != nullptr);
}