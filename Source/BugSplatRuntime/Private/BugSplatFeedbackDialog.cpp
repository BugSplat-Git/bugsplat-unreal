// Copyright BugSplat. All Rights Reserved.

#include "BugSplatFeedbackDialog.h"
#include "BugSplatFeedback.h"

#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SBoxPanel.h"
#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"
#include "Containers/Ticker.h"

namespace
{
	static const FSlateColorBrush OverlayBrush(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f));
	static const FSlateColorBrush DialogBrush(FLinearColor(0.15f, 0.15f, 0.15f, 1.0f));
}

TSharedPtr<SWidget> SBugSplatFeedbackDialog::ViewportWidget;

void SBugSplatFeedbackDialog::Construct(const FArguments& InArgs)
{
	SetVisibility(EVisibility::Visible);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(&OverlayBrush)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(&DialogBrush)
			.Padding(24.0f)
			[
				SNew(SBox)
				.WidthOverride(420.0f)
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 16)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Send Feedback")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(FSlateColor(FLinearColor::White))
					]

					// Subject label
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 4)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Subject")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
					]

					// Subject input
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 12)
					[
						SAssignNew(SubjectField, SEditableTextBox)
						.HintText(FText::FromString(TEXT("Brief summary of your feedback")))
					]

					// Description label
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 4)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("Description (optional)")))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
					]

					// Description text area
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 20)
					[
						SNew(SBox)
						.HeightOverride(150.0f)
						[
							SAssignNew(DescriptionField, SMultiLineEditableTextBox)
							.HintText(FText::FromString(TEXT("Add more details...")))
						]
					]

					// Include logs checkbox
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 20)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SAssignNew(IncludeLogsCheckbox, SCheckBox)
							.IsChecked(ECheckBoxState::Checked)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(8, 0, 0, 0)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Include application logs")))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
						]
					]

					// Buttons
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SSpacer)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0, 0, 8, 0)
						[
							SNew(SButton)
							.OnClicked(this, &SBugSplatFeedbackDialog::OnCancelClicked)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Cancel")))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.OnClicked(this, &SBugSplatFeedbackDialog::OnSubmitClicked)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Submit")))
							]
						]
					]

					// Status message (hidden until submit)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 12, 0, 0)
					[
						SAssignNew(StatusText, STextBlock)
						.Visibility(EVisibility::Collapsed)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.9f, 0.4f)))
					]
				]
			]
		]
	];
}

void SBugSplatFeedbackDialog::Show()
{
	if (ViewportWidget.IsValid())
	{
		return;
	}

	if (!GEngine || !GEngine->GameViewport)
	{
		UE_LOG(LogTemp, Warning, TEXT("BugSplat: Cannot show feedback dialog - no game viewport"));
		return;
	}

	TSharedRef<SBugSplatFeedbackDialog> Dialog = SNew(SBugSplatFeedbackDialog);
	ViewportWidget = Dialog;

	GEngine->GameViewport->AddViewportWidgetContent(Dialog, 100);

	FSlateApplication::Get().SetKeyboardFocus(Dialog, EFocusCause::SetDirectly);
}

void SBugSplatFeedbackDialog::Dismiss()
{
	if (!ViewportWidget.IsValid())
	{
		return;
	}

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(ViewportWidget.ToSharedRef());
	}

	ViewportWidget.Reset();
}

bool SBugSplatFeedbackDialog::IsShowing()
{
	return ViewportWidget.IsValid();
}

FReply SBugSplatFeedbackDialog::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

FReply SBugSplatFeedbackDialog::OnSubmitClicked()
{
	FString Subject = SubjectField.IsValid() ? SubjectField->GetText().ToString() : TEXT("");
	FString Description = DescriptionField.IsValid() ? DescriptionField->GetText().ToString() : TEXT("");

	if (Subject.IsEmpty())
	{
		if (StatusText.IsValid())
		{
			StatusText->SetText(FText::FromString(TEXT("Subject is required")));
			StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.3f, 0.3f)));
			StatusText->SetVisibility(EVisibility::Visible);
		}
		return FReply::Handled();
	}

	TArray<FString> Attachments;
	bool bIncludeLogs = IncludeLogsCheckbox.IsValid() && IncludeLogsCheckbox->IsChecked();
	if (bIncludeLogs)
	{
		Attachments.Add(UBugSplatFeedback::GetLogFilePath());
	}

	UBugSplatFeedback::PostFeedback(Subject, Description, Attachments, TEXT(""), TEXT(""), TEXT(""), TMap<FString, FString>());
	ShowConfirmationAndDismiss();
	return FReply::Handled();
}

FReply SBugSplatFeedbackDialog::OnCancelClicked()
{
	Dismiss();
	return FReply::Handled();
}

void SBugSplatFeedbackDialog::ShowConfirmationAndDismiss()
{
	if (StatusText.IsValid())
	{
		StatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.9f, 0.4f)));
		StatusText->SetText(FText::FromString(TEXT("Feedback sent — thank you!")));
		StatusText->SetVisibility(EVisibility::Visible);
	}

	FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([](float DeltaTime) -> bool
		{
			SBugSplatFeedbackDialog::Dismiss();
			return false;
		}),
		1.5f
	);
}
