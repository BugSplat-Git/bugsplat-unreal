// Copyright 2023 BugSplat. All Rights Reserved.

#include "BugSplatFeedbackDialog.h"
#include "BugSplatRuntime.h"
#include "BugSplatEditorSettings.h"

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
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Guid.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/App.h"
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
		UE_LOG(LogTemp, Warning, TEXT("BugSplat: Feedback subject is empty, not submitting"));
		return FReply::Handled();
	}

	SubmitFeedback(Subject, Description);
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

void SBugSplatFeedbackDialog::SubmitFeedback(const FString& Subject, const FString& Description)
{
	UBugSplatEditorSettings* Settings = FBugSplatRuntimeModule::Get().GetSettings();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("BugSplat: Cannot submit feedback - settings not available"));
		return;
	}

	const FString& Database = Settings->BugSplatDatabase;
	const FString& AppName = Settings->BugSplatApp;
	const FString& AppVersion = Settings->BugSplatVersion;

	if (Database.IsEmpty() || AppName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("BugSplat: Cannot submit feedback - Database or Application not configured in plugin settings"));
		return;
	}

	FString Url = FString::Printf(TEXT("https://%s.bugsplat.com/post/feedback/"), *Database);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));

	FString Boundary = FGuid::NewGuid().ToString();
	Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

	TArray<uint8> Payload;

	auto AppendString = [](TArray<uint8>& Data, const FString& Str)
	{
		FTCHARToUTF8 Utf8(*Str);
		Data.Append((const uint8*)Utf8.Get(), Utf8.Length());
	};

	auto AddField = [&Boundary, &Payload, &AppendString](const FString& Name, const FString& Value)
	{
		AppendString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
		AppendString(Payload, FString::Printf(TEXT("Content-Disposition: form-data; name=\"%s\"\r\n\r\n"), *Name));
		AppendString(Payload, Value + TEXT("\r\n"));
	};

	auto AddFileField = [&Boundary, &Payload, &AppendString](const FString& FieldName, const FString& FileName, const TArray<uint8>& FileData)
	{
		AppendString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
		AppendString(Payload, FString::Printf(TEXT("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n"), *FieldName, *FileName));
		AppendString(Payload, TEXT("Content-Type: application/octet-stream\r\n\r\n"));
		Payload.Append(FileData);
		AppendString(Payload, TEXT("\r\n"));
	};

	AddField(TEXT("database"), Database);
	AddField(TEXT("appName"), AppName);
	AddField(TEXT("appVersion"), AppVersion);
	AddField(TEXT("title"), Subject);
	if (!Description.IsEmpty())
	{
		AddField(TEXT("description"), Description);
	}

	// Include crash context attributes
	TMap<FString, FString> Attributes = FBugSplatRuntimeModule::Get().GetCrashAttributes();
	if (Attributes.Num() > 0)
	{
		TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
		for (const TPair<FString, FString>& Pair : Attributes)
		{
			JsonObject->SetStringField(Pair.Key, Pair.Value);
		}

		FString AttributesJson;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&AttributesJson);
		FJsonSerializer::Serialize(JsonObject, Writer);

		AddField(TEXT("attributes"), AttributesJson);
	}

	// Attach Unreal log file if the user opted in
	bool bIncludeLogs = IncludeLogsCheckbox.IsValid() && IncludeLogsCheckbox->IsChecked();
	if (bIncludeLogs)
	{
		FString LogFilePath = FPaths::ProjectLogDir() / FApp::GetProjectName() + TEXT(".log");
		LogFilePath = FPaths::ConvertRelativePathToFull(LogFilePath);

		TArray<uint8> LogData;
		if (FFileHelper::LoadFileToArray(LogData, *LogFilePath))
		{
			AddFileField(TEXT("logFile"), FPaths::GetCleanFilename(LogFilePath), LogData);
			UE_LOG(LogTemp, Log, TEXT("BugSplat: Attached log file %s (%d bytes)"), *LogFilePath, LogData.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BugSplat: Could not read log file at %s"), *LogFilePath);
		}
	}

	AppendString(Payload, FString::Printf(TEXT("--%s--\r\n"), *Boundary));

	Request->SetContent(Payload);

	Request->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			if (bSuccess && Resp.IsValid())
			{
				int32 Code = Resp->GetResponseCode();
				FString Body = Resp->GetContentAsString();
				UE_LOG(LogTemp, Log, TEXT("BugSplat: Feedback response (HTTP %d): %s"), Code, *Body);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BugSplat: Feedback request failed (no response)"));
			}
		}
	);

	Request->ProcessRequest();
	UE_LOG(LogTemp, Log, TEXT("BugSplat: Submitting feedback to %s (database=%s, appName=%s, appVersion=%s, title=%s)"),
		*Url, *Database, *AppName, *AppVersion, *Subject);
}
