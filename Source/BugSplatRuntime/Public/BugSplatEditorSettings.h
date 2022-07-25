#pragma once

#include "CoreMinimal.h"
#include "BugSplatEditorSettings.generated.h"

UCLASS(Config = Engine, defaultconfig)
class BUGSPLATRUNTIME_API UBugSplatEditorSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Common", Meta = (DisplayName = "Database", ToolTip = "Database name"))
	FString BugSplatDatabase;

	UPROPERTY(Config, EditAnywhere, Category = "IOS", Meta = (DisplayName = "User", ToolTip = "Username (required for debug symbols upload)"))
	FString BugSplatUser;

	UPROPERTY(Config, EditAnywhere, Category = "IOS", Meta = (DisplayName = "Password", ToolTip = "Password (required for debug symbols upload)"))
	FString BugSplatPassword;
};