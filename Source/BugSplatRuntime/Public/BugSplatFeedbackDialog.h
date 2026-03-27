// Copyright BugSplat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;
class SMultiLineEditableTextBox;
class STextBlock;
class SCheckBox;

class BUGSPLATRUNTIME_API SBugSplatFeedbackDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBugSplatFeedbackDialog) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static void Show();
	static void Dismiss();
	static bool IsShowing();
	static void ShowConfirmationAndDismiss();
	static void ShowError(const FString& Message);

	virtual bool SupportsKeyboardFocus() const override { return true; }

private:
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	TSharedPtr<SEditableTextBox> SubjectField;
	TSharedPtr<SMultiLineEditableTextBox> DescriptionField;
	TSharedPtr<SCheckBox> IncludeLogsCheckbox;
	TSharedPtr<STextBlock> StatusText;

	FReply OnSubmitClicked();
	FReply OnCancelClicked();

	static TSharedPtr<SWidget> ViewportWidget;
};
