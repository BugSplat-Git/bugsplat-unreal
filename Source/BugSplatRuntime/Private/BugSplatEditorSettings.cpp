// Copyright BugSplat. All Rights Reserved.

#include "BugSplatEditorSettings.h"

UBugSplatEditorSettings::UBugSplatEditorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BugSplatDatabase = TEXT("");
	BugSplatApp = TEXT("");
	BugSplatVersion = TEXT("");
	BugSplatClientId = TEXT("");
	BugSplatClientSecret = TEXT("");
	bUpdateEngineDataRouterUrl = true;
	bUploadDebugSymbols = false;
	bEnableCrashReportingIos = true;
	bEnableCrashReportingAndroid = true;
	bEnableNativeDesktop = false;
	DesktopUploadPolicy = EBugSplatUploadPolicy::Quiet;
	DesktopDumpType = EBugSplatDumpType::Normal;
	DesktopHangDetectionTimeoutMs = 0;
	DesktopHangPolicy = EBugSplatHangPolicy::Report;
	bDesktopOpenSupportUrl = true;
}

bool UBugSplatEditorSettings::HasValidCrashReporterSettings() const
{
	return !BugSplatDatabase.IsEmpty() && !BugSplatApp.IsEmpty() && !BugSplatVersion.IsEmpty();
}

bool UBugSplatEditorSettings::HasValidSymbolUploadSettings() const
{
	return !BugSplatDatabase.IsEmpty() && !BugSplatApp.IsEmpty() && !BugSplatVersion.IsEmpty() && !BugSplatClientId.IsEmpty() && !BugSplatClientSecret.IsEmpty();
}

#if WITH_EDITOR
bool UBugSplatEditorSettings::CanEditChange(const FProperty* InProperty) const
{
	const bool ParentVal = Super::CanEditChange(InProperty);

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UBugSplatEditorSettings, bUploadDebugSymbols))
	{
		return ParentVal && HasValidSymbolUploadSettings();
	}

	return ParentVal;
}
#endif