// Copyright BugSplat. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BugSplatNative.generated.h"

class UBugSplatEditorSettings;

/** What happens to a bugsplat-native report once it exists on disk. */
UENUM(BlueprintType)
enum class EBugSplatUploadPolicy : uint8
{
	/** Show the BugSplat dialog (BugSplatReporter), then upload. */
	Dialog = 0,
	/** Upload without any UI. */
	Quiet = 1,
	/** Leave reports on disk; PostPendingReports sends them. */
	Manual = 2,
};

/** How much process memory a crash dump carries. Size is enforced by the database's limit at upload. */
UENUM(BlueprintType)
enum class EBugSplatDumpType : uint8
{
	Normal = 0,
	Heap = 1,
	Full = 2,
};

/** What bugsplat-native does when the game thread stops sending heartbeats for longer than the hang timeout. */
UENUM(BlueprintType)
enum class EBugSplatHangPolicy : uint8
{
	/** Dump the live process, upload a hang report, let the game continue. */
	Report = 0,
	/** Report, offer the user Wait / Close, terminate the game on Close. */
	ReportAndTerminate = 1,
};

/** Outcome of a report bugsplat-native uploaded on the caller's behalf. */
USTRUCT(BlueprintType)
struct FBugSplatUploadResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "BugSplat")
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly, Category = "BugSplat")
	int32 HttpStatus = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BugSplat")
	int64 CrashId = 0;

	UPROPERTY(BlueprintReadOnly, Category = "BugSplat")
	int64 StackKeyId = 0;

	/** Support-response URL, empty if none. */
	UPROPERTY(BlueprintReadOnly, Category = "BugSplat")
	FString InfoUrl;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FBugSplatUploadComplete, const FBugSplatUploadResult&, Result);

/**
 * The desktop bugsplat-native SDK (Windows, macOS, Linux), loaded at runtime from
 * Source/ThirdParty/BugSplatNative/<Platform>/ so the plugin builds and packages whether or not the
 * binaries are present. Everything here is a thin call over bugsplat.h; see that header for the
 * contract (UTF-8 strings, thread-safe after Start, nothing frozen at init).
 *
 * Engine crashes keep going through Unreal's own handler and CrashReportClient. What this adds on
 * desktop is what CrashReportClient cannot do: hang detection driven by a game-thread heartbeat,
 * live captures (a dump of the running process), user feedback and structured reports through
 * BugSplat's presigned uploader, and attributes / attachments that can change at any time.
 */
class BUGSPLATRUNTIME_API FBugSplatNative
{
public:
	static FBugSplatNative& Get();

	/** Loads the library and calls bugsplat_init. Returns false with LastError set when the runtime is missing or refuses to start. */
	bool Start(const UBugSplatEditorSettings& Settings);
	void Shutdown();

	bool IsRunning() const { return bRunning; }
	FString GetVersion() const;
	FString GetEnvironment() const;
	FString GetLogFilePath() const;
	const FString& GetLastError() const { return LastError; }

	void SetUser(const FString& Value);
	void SetEmail(const FString& Value);
	void SetKey(const FString& Value);
	void SetDescription(const FString& Value);
	void SetNotes(const FString& Value);
	/** Empty restores the SDK's own OS/hardware detection. */
	void SetEnvironment(const FString& Value);
	void SetAttribute(const FString& Key, const FString& Value);
	void RemoveAttribute(const FString& Key);
	bool AddAttachment(const FString& FilePath);
	bool RemoveAttachment(const FString& FilePath);
	void SetQuietMode(bool bQuiet);

	/** Dump the live process out of process; the report flows like a crash. The game keeps running. */
	bool CaptureReport();
	/** Told to the hang detector every frame by the runtime module's ticker. */
	void Heartbeat();
	bool WatchThread(const FString& Name);
	void UnwatchThread();
	void PostPendingReportsAsync();

	/** Blocking; call from a worker thread (UBugSplatNative does). */
	bool PostFeedback(const FString& Title, const FString& Description, const TArray<FString>& Attachments, FBugSplatUploadResult& OutResult);
	/** Blocking. Frames are "Function|File|Line" or plain text; see UBugSplatNative::PostStructuredReport. */
	bool PostStructuredReport(const FString& Code, const FString& Explanation, const TArray<FString>& Frames, const TArray<FString>& Attachments, FBugSplatUploadResult& OutResult);

private:
	FBugSplatNative();
	~FBugSplatNative();

	bool LoadLibrary();
	FString GetRuntimeDirectory() const;
	template <typename T> bool Bind(T& Fn, const TCHAR* Name);

	void* DllHandle = nullptr;
	bool bLoaded = false;
	bool bRunning = false;
	FString LastError;

	struct FApi;
	TUniquePtr<FApi> Api;
};

/** Blueprint access to the desktop bugsplat-native SDK. Every call is a no-op when it is not running. */
UCLASS()
class BUGSPLATRUNTIME_API UBugSplatNative : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** True when bugsplat-native started for this process (desktop players with the runtime shipped and the setting on). */
	UFUNCTION(BlueprintPure, Category = "BugSplat|Native")
	static bool IsNativeCrashReportingRunning();

	/** The bugsplat-native version, or empty. */
	UFUNCTION(BlueprintPure, Category = "BugSplat|Native")
	static FString GetNativeVersion();

	/** The OS/hardware string sent as a first-class report property. */
	UFUNCTION(BlueprintPure, Category = "BugSplat|Native")
	static FString GetEnvironment();

	/** Dump the live process and report it like a crash, without crashing. */
	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static bool CaptureReport();

	/** Attach a file to every native report from now on; the monitor copies it right after the dump. */
	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static bool AddAttachment(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static bool RemoveAttachment(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static void SetUser(const FString& User);

	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static void SetEmail(const FString& Email);

	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static void SetKey(const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static void SetDescription(const FString& Description);

	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static void SetNotes(const FString& Notes);

	/** Show or suppress the BugSplat crash dialog from now on (desktop). */
	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static void SetCrashDialogEnabled(bool bEnabled);

	/** Upload reports left on disk (offline, or the Manual upload policy) on a background thread. */
	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native")
	static void PostPendingReports();

	/**
	 * Post a structured (client-generated) report, crash type 21, through bugsplat-native. Each frame
	 * is "Function|File|Line" (File and Line optional) or free text. Uploads on a worker thread and
	 * calls OnComplete on the game thread.
	 */
	UFUNCTION(BlueprintCallable, Category = "BugSplat|Native", meta = (AutoCreateRefTerm = "Attachments,OnComplete"))
	static void PostStructuredReport(const FString& Code, const FString& Explanation, const TArray<FString>& Frames, const TArray<FString>& Attachments, const FBugSplatUploadComplete& OnComplete);
};
