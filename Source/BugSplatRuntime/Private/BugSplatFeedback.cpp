#include "BugSplatFeedback.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/Archive.h"

// Minimal ZIP implementation for a single uncompressed file
namespace
{
    uint32 Crc32(const uint8* Data, int32 Length)
    {
        uint32 Crc = 0xFFFFFFFF;
        for (int32 i = 0; i < Length; i++)
        {
            Crc ^= Data[i];
            for (int32 j = 0; j < 8; j++)
            {
                Crc = (Crc >> 1) ^ (Crc & 1 ? 0xEDB88320 : 0);
            }
        }
        return Crc ^ 0xFFFFFFFF;
    }

    void WriteUint16(TArray<uint8>& Out, uint16 Value)
    {
        Out.Add(Value & 0xFF);
        Out.Add((Value >> 8) & 0xFF);
    }

    void WriteUint32(TArray<uint8>& Out, uint32 Value)
    {
        Out.Add(Value & 0xFF);
        Out.Add((Value >> 8) & 0xFF);
        Out.Add((Value >> 16) & 0xFF);
        Out.Add((Value >> 24) & 0xFF);
    }

    TArray<uint8> CreateSingleFileZip(const FString& FileName, const TArray<uint8>& FileData)
    {
        TArray<uint8> Result;
        FTCHARToUTF8 NameUtf8(*FileName);
        const uint8* NameBytes = (const uint8*)NameUtf8.Get();
        int32 NameLen = NameUtf8.Length();
        uint32 Crc = Crc32(FileData.GetData(), FileData.Num());
        uint32 FileSize = FileData.Num();

        // DOS time/date
        FDateTime Now = FDateTime::Now();
        uint16 DosTime = ((Now.GetHour() << 11) | (Now.GetMinute() << 5) | (Now.GetSecond() >> 1)) & 0xFFFF;
        uint16 DosDate = (((Now.GetYear() - 1980) << 9) | (Now.GetMonth() << 5) | Now.GetDay()) & 0xFFFF;

        // Local file header
        int32 LocalHeaderOffset = Result.Num();
        WriteUint32(Result, 0x04034B50);  // Signature
        WriteUint16(Result, 20);           // Version needed
        WriteUint16(Result, 0);            // Flags
        WriteUint16(Result, 0);            // Compression: stored
        WriteUint16(Result, DosTime);
        WriteUint16(Result, DosDate);
        WriteUint32(Result, Crc);
        WriteUint32(Result, FileSize);     // Compressed size
        WriteUint32(Result, FileSize);     // Uncompressed size
        WriteUint16(Result, NameLen);
        WriteUint16(Result, 0);            // Extra field length
        Result.Append(NameBytes, NameLen);
        Result.Append(FileData.GetData(), FileData.Num());

        // Central directory
        int32 CentralDirOffset = Result.Num();
        WriteUint32(Result, 0x02014B50);  // Signature
        WriteUint16(Result, 20);           // Version made by
        WriteUint16(Result, 20);           // Version needed
        WriteUint16(Result, 0);            // Flags
        WriteUint16(Result, 0);            // Compression: stored
        WriteUint16(Result, DosTime);
        WriteUint16(Result, DosDate);
        WriteUint32(Result, Crc);
        WriteUint32(Result, FileSize);
        WriteUint32(Result, FileSize);
        WriteUint16(Result, NameLen);
        WriteUint16(Result, 0);            // Extra field length
        WriteUint16(Result, 0);            // Comment length
        WriteUint16(Result, 0);            // Disk number
        WriteUint16(Result, 0);            // Internal attributes
        WriteUint32(Result, 0);            // External attributes
        WriteUint32(Result, LocalHeaderOffset);
        Result.Append(NameBytes, NameLen);

        int32 CentralDirSize = Result.Num() - CentralDirOffset;

        // End of central directory
        WriteUint32(Result, 0x06054B50);
        WriteUint16(Result, 0);            // Disk number
        WriteUint16(Result, 0);            // Central dir disk
        WriteUint16(Result, 1);            // Entries on disk
        WriteUint16(Result, 1);            // Total entries
        WriteUint32(Result, CentralDirSize);
        WriteUint32(Result, CentralDirOffset);
        WriteUint16(Result, 0);            // Comment length

        return Result;
    }
}

void UBugSplatFeedback::PostFeedback(
    const FString& Database,
    const FString& Application,
    const FString& Version,
    const FString& Title,
    const FString& Description,
    const FString& User,
    const FString& Email,
    const FOnFeedbackComplete& OnComplete)
{
    if (Title.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("BugSplat: Feedback title cannot be empty"));
        OnComplete.ExecuteIfBound(false, TEXT("Title cannot be empty"));
        return;
    }

    TArray<uint8> ZipData = CreateFeedbackZip(Title, Description);
    GetPresignedUrl(Database, Application, Version, ZipData, Title, Description, User, Email, OnComplete);
}

TArray<uint8> UBugSplatFeedback::CreateFeedbackZip(const FString& Title, const FString& Description)
{
    TSharedPtr<FJsonObject> FeedbackObj = MakeShareable(new FJsonObject());
    FeedbackObj->SetStringField(TEXT("title"), Title);
    FeedbackObj->SetStringField(TEXT("description"), Description);

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(FeedbackObj.ToSharedRef(), Writer);

    FTCHARToUTF8 JsonUtf8(*JsonString);
    TArray<uint8> JsonBytes;
    JsonBytes.Append((const uint8*)JsonUtf8.Get(), JsonUtf8.Length());

    return CreateSingleFileZip(TEXT("feedback.json"), JsonBytes);
}

void UBugSplatFeedback::GetPresignedUrl(
    const FString& Database,
    const FString& Application,
    const FString& Version,
    const TArray<uint8>& ZipData,
    const FString& Title,
    const FString& Description,
    const FString& User,
    const FString& Email,
    FOnFeedbackComplete OnComplete)
{
    FString Url = FString::Printf(
        TEXT("https://%s.bugsplat.com/api/getCrashUploadUrl?database=%s&appName=%s&appVersion=%s&crashPostSize=%d"),
        *Database, *Database, *Application, *Version, ZipData.Num()
    );

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb(TEXT("GET"));

    // Capture data for the callback
    Request->OnProcessRequestComplete().BindLambda(
        [Database, Application, Version, ZipData, Description, User, Email, OnComplete]
        (FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (!bSuccess || !Resp.IsValid() || Resp->GetResponseCode() != 200)
            {
                UE_LOG(LogTemp, Error, TEXT("BugSplat: Failed to get presigned URL"));
                OnComplete.ExecuteIfBound(false, TEXT("Failed to get presigned URL"));
                return;
            }

            TSharedPtr<FJsonObject> JsonObj;
            TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Resp->GetContentAsString());
            if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
            {
                OnComplete.ExecuteIfBound(false, TEXT("Failed to parse presigned URL response"));
                return;
            }

            FString PresignedUrl = JsonObj->GetStringField(TEXT("url"));
            UploadToS3(PresignedUrl, ZipData, Database, Application, Version, Description, User, Email, OnComplete);
        }
    );

    Request->ProcessRequest();
}

void UBugSplatFeedback::UploadToS3(
    const FString& PresignedUrl,
    const TArray<uint8>& ZipData,
    const FString& Database,
    const FString& Application,
    const FString& Version,
    const FString& Description,
    const FString& User,
    const FString& Email,
    FOnFeedbackComplete OnComplete)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(PresignedUrl);
    Request->SetVerb(TEXT("PUT"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/zip"));
    Request->SetContent(ZipData);

    Request->OnProcessRequestComplete().BindLambda(
        [PresignedUrl, Database, Application, Version, Description, User, Email, OnComplete]
        (FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (!bSuccess || !Resp.IsValid() || Resp->GetResponseCode() != 200)
            {
                UE_LOG(LogTemp, Error, TEXT("BugSplat: Failed to upload feedback to S3"));
                OnComplete.ExecuteIfBound(false, TEXT("Failed to upload to S3"));
                return;
            }

            FString Etag = Resp->GetHeader(TEXT("ETag")).Replace(TEXT("\""), TEXT(""));
            CommitUpload(Database, Application, Version, PresignedUrl, Etag, Description, User, Email, OnComplete);
        }
    );

    Request->ProcessRequest();
}

void UBugSplatFeedback::CommitUpload(
    const FString& Database,
    const FString& Application,
    const FString& Version,
    const FString& S3Key,
    const FString& Md5,
    const FString& Description,
    const FString& User,
    const FString& Email,
    FOnFeedbackComplete OnComplete)
{
    FString Url = FString::Printf(TEXT("https://%s.bugsplat.com/api/commitS3CrashUpload"), *Database);
    FString Boundary = FGuid::NewGuid().ToString();

    FString Body;
    auto AddField = [&Body, &Boundary](const FString& Name, const FString& Value)
    {
        Body += FString::Printf(TEXT("--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n"), *Boundary, *Name, *Value);
    };

    AddField(TEXT("database"), Database);
    AddField(TEXT("appName"), Application);
    AddField(TEXT("appVersion"), Version);
    AddField(TEXT("crashTypeId"), TEXT("36"));
    AddField(TEXT("s3Key"), S3Key);
    AddField(TEXT("md5"), Md5);
    AddField(TEXT("description"), Description);
    AddField(TEXT("user"), User);
    AddField(TEXT("email"), Email);
    Body += FString::Printf(TEXT("--%s--\r\n"), *Boundary);

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Url);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

    FTCHARToUTF8 BodyUtf8(*Body);
    TArray<uint8> BodyBytes;
    BodyBytes.Append((const uint8*)BodyUtf8.Get(), BodyUtf8.Length());
    Request->SetContent(BodyBytes);

    Request->OnProcessRequestComplete().BindLambda(
        [OnComplete](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            if (!bSuccess || !Resp.IsValid() || Resp->GetResponseCode() != 200)
            {
                UE_LOG(LogTemp, Error, TEXT("BugSplat: Failed to commit feedback upload"));
                OnComplete.ExecuteIfBound(false, TEXT("Failed to commit feedback"));
                return;
            }

            UE_LOG(LogTemp, Log, TEXT("BugSplat: Feedback posted successfully"));
            OnComplete.ExecuteIfBound(true, Resp->GetContentAsString());
        }
    );

    Request->ProcessRequest();
}
