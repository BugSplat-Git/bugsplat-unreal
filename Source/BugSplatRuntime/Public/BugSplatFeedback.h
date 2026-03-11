#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BugSplatFeedback.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnFeedbackComplete, bool, bSuccess, const FString&, Response);

UCLASS()
class BUGSPLATRUNTIME_API UBugSplatFeedback : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Post user feedback to BugSplat.
     * Title becomes the stack key for grouping feedback in the dashboard.
     *
     * @param Database The BugSplat database name
     * @param Application The application name
     * @param Version The application version
     * @param Title The feedback title (becomes stack key)
     * @param Description Additional feedback context
     * @param User The user's name or id
     * @param Email The user's email
     * @param AttachmentPaths File paths to attach to the feedback report
     * @param AppKey Optional application key for restricted databases
     * @param OnComplete Callback invoked when the operation completes
     */
    UFUNCTION(BlueprintCallable, Category = "BugSplat")
    static void PostFeedback(
        const FString& Database,
        const FString& Application,
        const FString& Version,
        const FString& Title,
        const FString& Description = TEXT(""),
        const FString& User = TEXT(""),
        const FString& Email = TEXT(""),
        const TArray<FString>& AttachmentPaths = TArray<FString>(),
        const FString& AppKey = TEXT(""),
        const FOnFeedbackComplete& OnComplete = FOnFeedbackComplete()
    );

private:
    static void GetPresignedUrl(
        const FString& Database,
        const FString& Application,
        const FString& Version,
        const TArray<uint8>& ZipData,
        const FString& Title,
        const FString& Description,
        const FString& User,
        const FString& Email,
        const FString& AppKey,
        FOnFeedbackComplete OnComplete
    );

    static void UploadToS3(
        const FString& PresignedUrl,
        const TArray<uint8>& ZipData,
        const FString& Database,
        const FString& Application,
        const FString& Version,
        const FString& Description,
        const FString& User,
        const FString& Email,
        const FString& AppKey,
        FOnFeedbackComplete OnComplete
    );

    static void CommitUpload(
        const FString& Database,
        const FString& Application,
        const FString& Version,
        const FString& S3Key,
        const FString& Md5,
        const FString& Description,
        const FString& User,
        const FString& Email,
        const FString& AppKey,
        FOnFeedbackComplete OnComplete
    );

    static TArray<uint8> CreateFeedbackZip(const FString& Title, const FString& Description, const TArray<FString>& AttachmentPaths);
};
