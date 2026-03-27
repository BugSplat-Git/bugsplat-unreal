// Copyright BugSplat. All Rights Reserved.

#include "BugSplatFeedback.h"
#include "BugSplatRuntime.h"
#include "BugSplatEditorSettings.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Guid.h"
#include "Misc/App.h"

void UBugSplatFeedback::PostFeedback(const FString& Title, const FString& Description, const TArray<FString>& Attachments, const FString& User, const FString& Email, const FString& AppKey, const TMap<FString, FString>& CustomAttributes)
{
	PostFeedbackWithCallback(Title, Description, Attachments, User, Email, AppKey, CustomAttributes, FBugSplatFeedbackComplete());
}

void UBugSplatFeedback::PostFeedbackWithCallback(const FString& Title, const FString& Description, const TArray<FString>& Attachments, const FString& User, const FString& Email, const FString& AppKey, const TMap<FString, FString>& CustomAttributes, const FBugSplatFeedbackComplete& OnComplete)
{
	if (Title.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("BugSplat: Feedback title is empty, not submitting"));
		OnComplete.ExecuteIfBound(false, 0);
		return;
	}

	UBugSplatEditorSettings* Settings = FBugSplatRuntimeModule::Get().GetSettings();
	if (!Settings)
	{
		UE_LOG(LogTemp, Error, TEXT("BugSplat: Cannot submit feedback - settings not available"));
		OnComplete.ExecuteIfBound(false, 0);
		return;
	}

	const FString& Database = Settings->BugSplatDatabase;
	const FString& AppName = Settings->BugSplatApp;
	const FString& AppVersion = Settings->BugSplatVersion;

	if (Database.IsEmpty() || AppName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("BugSplat: Cannot submit feedback - Database or Application not configured in plugin settings"));
		OnComplete.ExecuteIfBound(false, 0);
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

	// Required fields
	AddField(TEXT("database"), Database);
	AddField(TEXT("appName"), AppName);
	AddField(TEXT("appVersion"), AppVersion);
	AddField(TEXT("title"), Title);

	if (!Description.IsEmpty())
	{
		AddField(TEXT("description"), Description);
	}
	if (!User.IsEmpty())
	{
		AddField(TEXT("user"), User);
	}
	if (!Email.IsEmpty())
	{
		AddField(TEXT("email"), Email);
	}
	if (!AppKey.IsEmpty())
	{
		AddField(TEXT("appKey"), AppKey);
	}

	// Include crash context attributes merged with caller-provided attributes
	TMap<FString, FString> Attributes = FBugSplatRuntimeModule::Get().GetCrashAttributes();
	Attributes.Append(CustomAttributes);
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

	// File attachments
	for (const FString& FilePath : Attachments)
	{
		TArray<uint8> FileData;
		if (FFileHelper::LoadFileToArray(FileData, *FilePath))
		{
			FString FileName = FPaths::GetCleanFilename(FilePath);
			AddFileField(FileName, FileName, FileData);
			UE_LOG(LogTemp, Log, TEXT("BugSplat: Attached file %s (%d bytes)"), *FilePath, FileData.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("BugSplat: Could not read attachment at %s"), *FilePath);
		}
	}

	AppendString(Payload, FString::Printf(TEXT("--%s--\r\n"), *Boundary));

	Request->SetContent(Payload);

	Request->OnProcessRequestComplete().BindLambda(
		[OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			if (bSuccess && Resp.IsValid())
			{
				int32 Code = Resp->GetResponseCode();
				bool bOk = Code >= 200 && Code < 300;
				UE_LOG(LogTemp, Log, TEXT("BugSplat: Feedback response (HTTP %d)"), Code);
				OnComplete.ExecuteIfBound(bOk, Code);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("BugSplat: Feedback request failed (no response)"));
				OnComplete.ExecuteIfBound(false, 0);
			}
		}
	);

	Request->ProcessRequest();
	UE_LOG(LogTemp, Log, TEXT("BugSplat: Submitting feedback to %s (database=%s, appName=%s, appVersion=%s)"),
		*Url, *Database, *AppName, *AppVersion);
}

FString UBugSplatFeedback::GetLogFilePath()
{
	FString LogFilePath = FPaths::ProjectLogDir() / FApp::GetProjectName() + TEXT(".log");
	return FPaths::ConvertRelativePathToFull(LogFilePath);
}
