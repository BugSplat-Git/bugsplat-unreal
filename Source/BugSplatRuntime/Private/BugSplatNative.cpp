// Copyright BugSplat. All Rights Reserved.

#include "BugSplatNative.h"
#include "BugSplatEditorSettings.h"

#include "Async/Async.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "Misc/EngineVersion.h"
#include "HAL/PlatformMisc.h"

// Only the types and enums: the functions are resolved by name at runtime (see FApi), so the
// dllimport declarations in the header are never referenced and no import library is needed.
#include "bugsplat/bugsplat.h"

DEFINE_LOG_CATEGORY_STATIC(LogBugSplatNative, Log, All);

#define BUGSPLAT_NATIVE_DESKTOP (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)

// ---- the resolved entry points -------------------------------------------------------------

struct FBugSplatNative::FApi
{
	bugsplat_options* (BUGSPLAT_CALL* options_new)(const char*, const char*, const char*) = nullptr;
	void (BUGSPLAT_CALL* options_set_upload_policy)(bugsplat_options*, bugsplat_upload_policy) = nullptr;
	void (BUGSPLAT_CALL* options_set_dump_type)(bugsplat_options*, bugsplat_dump_type) = nullptr;
	void (BUGSPLAT_CALL* options_set_hang_detection)(bugsplat_options*, int32_t, bugsplat_hang_policy) = nullptr;
	void (BUGSPLAT_CALL* options_set_open_support_url)(bugsplat_options*, int) = nullptr;
	void (BUGSPLAT_CALL* options_set_crash_completion)(bugsplat_options*, bugsplat_crash_completion) = nullptr;
	void (BUGSPLAT_CALL* options_set_protect_exception_filter)(bugsplat_options*, int) = nullptr;
	void (BUGSPLAT_CALL* options_set_chain_previous_signal_handlers)(bugsplat_options*, int) = nullptr;
	void (BUGSPLAT_CALL* options_set_monitor_path)(bugsplat_options*, const char*) = nullptr;
	void (BUGSPLAT_CALL* options_set_reporter_path)(bugsplat_options*, const char*) = nullptr;
	void (BUGSPLAT_CALL* options_set_theme_dir)(bugsplat_options*, const char*) = nullptr;
	void (BUGSPLAT_CALL* options_set_attribute)(bugsplat_options*, const char*, const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* init)(bugsplat_options*) = nullptr;
	int (BUGSPLAT_CALL* is_initialized)(void) = nullptr;
	void (BUGSPLAT_CALL* shutdown)(void) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* set_key)(const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* set_user)(const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* set_email)(const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* set_user_description)(const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* set_notes)(const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* set_environment)(const char*) = nullptr;
	const char* (BUGSPLAT_CALL* get_environment)(void) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* set_attribute)(const char*, const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* add_attachment)(const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* remove_attachment)(const char*) = nullptr;
	void (BUGSPLAT_CALL* set_quiet_mode)(int) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* capture_report)(void) = nullptr;
	bugsplat_report* (BUGSPLAT_CALL* report_new)(bugsplat_report_format) = nullptr;
	void (BUGSPLAT_CALL* report_free)(bugsplat_report*) = nullptr;
	void (BUGSPLAT_CALL* report_set_platform)(bugsplat_report*, const char*, const char*) = nullptr;
	void (BUGSPLAT_CALL* report_set_exception)(bugsplat_report*, const char*, const char*) = nullptr;
	int32_t (BUGSPLAT_CALL* report_add_thread)(bugsplat_report*, const char*, int) = nullptr;
	void (BUGSPLAT_CALL* report_add_frame)(bugsplat_report*, int32_t, const char*, const char*, int32_t, const char*, uint64_t) = nullptr;
	void (BUGSPLAT_CALL* report_add_attachment)(bugsplat_report*, const char*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* report_post)(bugsplat_report*, bugsplat_upload_result*) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* post_feedback)(const char*, const char*, const char* const*, size_t, bugsplat_upload_result*) = nullptr;
	void (BUGSPLAT_CALL* heartbeat)(void) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* watch_thread)(const char*) = nullptr;
	void (BUGSPLAT_CALL* unwatch_thread)(void) = nullptr;
	bugsplat_result (BUGSPLAT_CALL* post_pending_reports_async)(void) = nullptr;
	const char* (BUGSPLAT_CALL* version_string)(void) = nullptr;
	uint32_t (BUGSPLAT_CALL* abi_version)(void) = nullptr;
	const char* (BUGSPLAT_CALL* log_file_path)(void) = nullptr;
	void (BUGSPLAT_CALL* upload_result_free)(bugsplat_upload_result*) = nullptr;
};

namespace
{
	FString Utf8ToFString(const char* Value)
	{
		return Value ? FString(UTF8_TO_TCHAR(Value)) : FString();
	}

	FBugSplatUploadResult TakeResult(bugsplat_result Rc, bugsplat_upload_result& Raw, void (BUGSPLAT_CALL* Free)(bugsplat_upload_result*))
	{
		FBugSplatUploadResult Out;
		Out.bSuccess = Rc == BUGSPLAT_OK;
		Out.HttpStatus = Raw.http_status;
		Out.CrashId = Raw.crash_id;
		Out.StackKeyId = Raw.stack_key_id;
		Out.InfoUrl = Utf8ToFString(Raw.info_url);
		if (Free) Free(&Raw);
		return Out;
	}

	const TCHAR* DescribeResult(bugsplat_result Rc)
	{
		switch (Rc)
		{
		case BUGSPLAT_ERR_MONITOR_NOT_FOUND: return TEXT("BugSplatMonitor was not found next to the library (Source/ThirdParty/BugSplatNative/<Platform>)");
		case BUGSPLAT_ERR_REPORTER_NOT_FOUND: return TEXT("BugSplatReporter was not found next to the library (Source/ThirdParty/BugSplatNative/<Platform>)");
		case BUGSPLAT_ERR_MONITOR_START_FAILED: return TEXT("BugSplatMonitor could not be started");
		case BUGSPLAT_ERR_ALREADY_INITIALIZED: return TEXT("bugsplat-native is already initialized in this process");
		default: return TEXT("bugsplat_init failed");
		}
	}
}

// ---- FBugSplatNative -----------------------------------------------------------------------

FBugSplatNative::FBugSplatNative() = default;
FBugSplatNative::~FBugSplatNative() = default;

FBugSplatNative& FBugSplatNative::Get()
{
	static FBugSplatNative Instance;
	return Instance;
}

FString FBugSplatNative::GetRuntimeDirectory() const
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("BugSplat"));
	const FString Base = Plugin.IsValid() ? Plugin->GetBaseDir() : FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("BugSplat"));
#if PLATFORM_WINDOWS
	const TCHAR* PlatformDir = TEXT("Win64");
#elif PLATFORM_MAC
	const TCHAR* PlatformDir = TEXT("Mac");
#else
	const TCHAR* PlatformDir = TEXT("Linux");
#endif
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(Base, TEXT("Source"), TEXT("ThirdParty"), TEXT("BugSplatNative"), PlatformDir));
}

template <typename T>
bool FBugSplatNative::Bind(T& Fn, const TCHAR* Name)
{
	Fn = reinterpret_cast<T>(FPlatformProcess::GetDllExport(DllHandle, Name));
	if (!Fn)
	{
		LastError = FString::Printf(TEXT("%s is missing from the bugsplat library; it is not ABI version %u"), Name, BUGSPLAT_ABI_VERSION);
	}
	return Fn != nullptr;
}

bool FBugSplatNative::LoadLibrary()
{
#if !BUGSPLAT_NATIVE_DESKTOP
	LastError = TEXT("bugsplat-native desktop support is Windows, macOS and Linux only");
	return false;
#else
	if (bLoaded) return true;

	const FString Dir = GetRuntimeDirectory();
#if PLATFORM_WINDOWS
	const FString LibraryPath = FPaths::Combine(Dir, TEXT("BugSplat.dll"));
#elif PLATFORM_MAC
	const FString LibraryPath = FPaths::Combine(Dir, TEXT("libbugsplat.dylib"));
#else
	const FString LibraryPath = FPaths::Combine(Dir, TEXT("libbugsplat.so"));
#endif

	if (!FPaths::FileExists(LibraryPath))
	{
		LastError = FString::Printf(TEXT("%s is not present. Run Source/Scripts/fetch-bugsplat-native to populate Source/ThirdParty/BugSplatNative for this platform."), *LibraryPath);
		return false;
	}

	FPlatformProcess::PushDllDirectory(*Dir);
	DllHandle = FPlatformProcess::GetDllHandle(*LibraryPath);
	FPlatformProcess::PopDllDirectory(*Dir);
	if (!DllHandle)
	{
		LastError = FString::Printf(TEXT("%s could not be loaded"), *LibraryPath);
		return false;
	}

	Api = MakeUnique<FApi>();
	const bool bOk =
		Bind(Api->options_new, TEXT("bugsplat_options_new")) &&
		Bind(Api->options_set_upload_policy, TEXT("bugsplat_options_set_upload_policy")) &&
		Bind(Api->options_set_dump_type, TEXT("bugsplat_options_set_dump_type")) &&
		Bind(Api->options_set_hang_detection, TEXT("bugsplat_options_set_hang_detection")) &&
		Bind(Api->options_set_open_support_url, TEXT("bugsplat_options_set_open_support_url")) &&
		Bind(Api->options_set_crash_completion, TEXT("bugsplat_options_set_crash_completion")) &&
		Bind(Api->options_set_protect_exception_filter, TEXT("bugsplat_options_set_protect_exception_filter")) &&
		Bind(Api->options_set_chain_previous_signal_handlers, TEXT("bugsplat_options_set_chain_previous_signal_handlers")) &&
		Bind(Api->options_set_monitor_path, TEXT("bugsplat_options_set_monitor_path")) &&
		Bind(Api->options_set_reporter_path, TEXT("bugsplat_options_set_reporter_path")) &&
		Bind(Api->options_set_theme_dir, TEXT("bugsplat_options_set_theme_dir")) &&
		Bind(Api->options_set_attribute, TEXT("bugsplat_options_set_attribute")) &&
		Bind(Api->init, TEXT("bugsplat_init")) &&
		Bind(Api->is_initialized, TEXT("bugsplat_is_initialized")) &&
		Bind(Api->shutdown, TEXT("bugsplat_shutdown")) &&
		Bind(Api->set_key, TEXT("bugsplat_set_key")) &&
		Bind(Api->set_user, TEXT("bugsplat_set_user")) &&
		Bind(Api->set_email, TEXT("bugsplat_set_email")) &&
		Bind(Api->set_user_description, TEXT("bugsplat_set_user_description")) &&
		Bind(Api->set_notes, TEXT("bugsplat_set_notes")) &&
		Bind(Api->set_environment, TEXT("bugsplat_set_environment")) &&
		Bind(Api->get_environment, TEXT("bugsplat_get_environment")) &&
		Bind(Api->set_attribute, TEXT("bugsplat_set_attribute")) &&
		Bind(Api->add_attachment, TEXT("bugsplat_add_attachment")) &&
		Bind(Api->remove_attachment, TEXT("bugsplat_remove_attachment")) &&
		Bind(Api->set_quiet_mode, TEXT("bugsplat_set_quiet_mode")) &&
		Bind(Api->capture_report, TEXT("bugsplat_capture_report")) &&
		Bind(Api->report_new, TEXT("bugsplat_report_new")) &&
		Bind(Api->report_free, TEXT("bugsplat_report_free")) &&
		Bind(Api->report_set_platform, TEXT("bugsplat_report_set_platform")) &&
		Bind(Api->report_set_exception, TEXT("bugsplat_report_set_exception")) &&
		Bind(Api->report_add_thread, TEXT("bugsplat_report_add_thread")) &&
		Bind(Api->report_add_frame, TEXT("bugsplat_report_add_frame")) &&
		Bind(Api->report_add_attachment, TEXT("bugsplat_report_add_attachment")) &&
		Bind(Api->report_post, TEXT("bugsplat_report_post")) &&
		Bind(Api->post_feedback, TEXT("bugsplat_post_feedback")) &&
		Bind(Api->heartbeat, TEXT("bugsplat_heartbeat")) &&
		Bind(Api->watch_thread, TEXT("bugsplat_watch_thread")) &&
		Bind(Api->unwatch_thread, TEXT("bugsplat_unwatch_thread")) &&
		Bind(Api->post_pending_reports_async, TEXT("bugsplat_post_pending_reports_async")) &&
		Bind(Api->version_string, TEXT("bugsplat_version_string")) &&
		Bind(Api->abi_version, TEXT("bugsplat_abi_version")) &&
		Bind(Api->log_file_path, TEXT("bugsplat_log_file_path")) &&
		Bind(Api->upload_result_free, TEXT("bugsplat_upload_result_free"));

	if (!bOk)
	{
		FPlatformProcess::FreeDllHandle(DllHandle);
		DllHandle = nullptr;
		Api.Reset();
		return false;
	}

	if (Api->abi_version() != BUGSPLAT_ABI_VERSION)
	{
		LastError = FString::Printf(TEXT("bugsplat library ABI %u does not match the %u this plugin was built for"), Api->abi_version(), BUGSPLAT_ABI_VERSION);
		FPlatformProcess::FreeDllHandle(DllHandle);
		DllHandle = nullptr;
		Api.Reset();
		return false;
	}

	bLoaded = true;
	return true;
#endif
}

bool FBugSplatNative::Start(const UBugSplatEditorSettings& Settings)
{
	if (bRunning) return true;
	if (!LoadLibrary()) return false;

	if (Settings.BugSplatDatabase.IsEmpty() || Settings.BugSplatApp.IsEmpty() || Settings.BugSplatVersion.IsEmpty())
	{
		LastError = TEXT("Database, Application and Version must be set in Project Settings > Plugins > BugSplat");
		return false;
	}

	bugsplat_options* Options = Api->options_new(
		TCHAR_TO_UTF8(*Settings.BugSplatDatabase),
		TCHAR_TO_UTF8(*Settings.BugSplatApp),
		TCHAR_TO_UTF8(*Settings.BugSplatVersion));
	if (!Options)
	{
		LastError = TEXT("bugsplat_options_new returned null");
		return false;
	}

	Api->options_set_upload_policy(Options, static_cast<bugsplat_upload_policy>(Settings.DesktopUploadPolicy));
	Api->options_set_dump_type(Options, static_cast<bugsplat_dump_type>(Settings.DesktopDumpType));
	Api->options_set_hang_detection(Options, Settings.DesktopHangDetectionTimeoutMs, static_cast<bugsplat_hang_policy>(Settings.DesktopHangPolicy));
	Api->options_set_open_support_url(Options, Settings.bDesktopOpenSupportUrl ? 1 : 0);
	// The engine installs its own exception filter / signal handlers and wraps the game thread in a
	// guard; those must keep running first so engine crashes still reach CrashReportClient. Chaining
	// and leaving the filter unprotected is what makes the two coexist.
	Api->options_set_protect_exception_filter(Options, 0);
	Api->options_set_chain_previous_signal_handlers(Options, 1);
	Api->options_set_crash_completion(Options, BUGSPLAT_COMPLETION_CONTINUE_SEARCH);

	// The helpers live next to the library, which is where the SDK looks anyway; named explicitly
	// so a project that stages the plugin somewhere unusual still finds them.
	const FString Dir = GetRuntimeDirectory();
#if PLATFORM_WINDOWS
	Api->options_set_monitor_path(Options, TCHAR_TO_UTF8(*FPaths::Combine(Dir, TEXT("BugSplatMonitor.exe"))));
	Api->options_set_reporter_path(Options, TCHAR_TO_UTF8(*FPaths::Combine(Dir, TEXT("BugSplatReporter.exe"))));
#elif PLATFORM_MAC
	Api->options_set_monitor_path(Options, TCHAR_TO_UTF8(*FPaths::Combine(Dir, TEXT("BugSplatMonitor"))));
	Api->options_set_reporter_path(Options, TCHAR_TO_UTF8(*FPaths::Combine(Dir, TEXT("BugSplatReporter.app"))));
#else
	Api->options_set_monitor_path(Options, TCHAR_TO_UTF8(*FPaths::Combine(Dir, TEXT("BugSplatMonitor"))));
	Api->options_set_reporter_path(Options, TCHAR_TO_UTF8(*FPaths::Combine(Dir, TEXT("BugSplatReporter"))));
#endif
	Api->options_set_theme_dir(Options, TCHAR_TO_UTF8(*FPaths::Combine(Dir, TEXT("theme"))));

	// A curated slice of the engine's crash context, small enough to stay well under the SDK's
	// attribute limit; the OS itself is the SDK's Environment property.
	Api->options_set_attribute(Options, "EngineVersion", TCHAR_TO_UTF8(*FEngineVersion::Current().ToString()));
	Api->options_set_attribute(Options, "ProjectName", TCHAR_TO_UTF8(FApp::GetProjectName()));
	Api->options_set_attribute(Options, "BuildVersion", TCHAR_TO_UTF8(FApp::GetBuildVersion()));
	Api->options_set_attribute(Options, "CPUBrand", TCHAR_TO_UTF8(*FPlatformMisc::GetCPUBrand()));
	Api->options_set_attribute(Options, "GPUBrand", TCHAR_TO_UTF8(*FPlatformMisc::GetPrimaryGPUBrand()));
#if UE_BUILD_SHIPPING
	Api->options_set_attribute(Options, "BuildConfiguration", "Shipping");
#elif UE_BUILD_TEST
	Api->options_set_attribute(Options, "BuildConfiguration", "Test");
#elif UE_BUILD_DEBUG
	Api->options_set_attribute(Options, "BuildConfiguration", "Debug");
#else
	Api->options_set_attribute(Options, "BuildConfiguration", "Development");
#endif

	const bugsplat_result Rc = Api->init(Options);  // takes ownership, success or not
	if (Rc != BUGSPLAT_OK)
	{
		LastError = DescribeResult(Rc);
		return false;
	}

	bRunning = true;
	LastError.Empty();
	UE_LOG(LogBugSplatNative, Log, TEXT("bugsplat-native %s started (%s); log at %s"), *GetVersion(), *GetEnvironment(), *GetLogFilePath());
	Api->post_pending_reports_async();
	return true;
}

void FBugSplatNative::Shutdown()
{
	if (bRunning && Api && Api->shutdown)
	{
		Api->shutdown();
	}
	bRunning = false;
}

FString FBugSplatNative::GetVersion() const { return bLoaded ? Utf8ToFString(Api->version_string()) : FString(); }
FString FBugSplatNative::GetEnvironment() const { return bRunning ? Utf8ToFString(Api->get_environment()) : FString(); }
FString FBugSplatNative::GetLogFilePath() const { return bRunning ? Utf8ToFString(Api->log_file_path()) : FString(); }

void FBugSplatNative::SetUser(const FString& Value) { if (bRunning) Api->set_user(TCHAR_TO_UTF8(*Value)); }
void FBugSplatNative::SetEmail(const FString& Value) { if (bRunning) Api->set_email(TCHAR_TO_UTF8(*Value)); }
void FBugSplatNative::SetKey(const FString& Value) { if (bRunning) Api->set_key(TCHAR_TO_UTF8(*Value)); }
void FBugSplatNative::SetDescription(const FString& Value) { if (bRunning) Api->set_user_description(TCHAR_TO_UTF8(*Value)); }
void FBugSplatNative::SetNotes(const FString& Value) { if (bRunning) Api->set_notes(TCHAR_TO_UTF8(*Value)); }
void FBugSplatNative::SetEnvironment(const FString& Value) { if (bRunning) Api->set_environment(Value.IsEmpty() ? nullptr : TCHAR_TO_UTF8(*Value)); }
void FBugSplatNative::SetAttribute(const FString& Key, const FString& Value) { if (bRunning && !Key.IsEmpty()) Api->set_attribute(TCHAR_TO_UTF8(*Key), TCHAR_TO_UTF8(*Value)); }
void FBugSplatNative::RemoveAttribute(const FString& Key) { if (bRunning && !Key.IsEmpty()) Api->set_attribute(TCHAR_TO_UTF8(*Key), nullptr); }
bool FBugSplatNative::AddAttachment(const FString& FilePath) { return bRunning && Api->add_attachment(TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(FilePath))) == BUGSPLAT_OK; }
bool FBugSplatNative::RemoveAttachment(const FString& FilePath) { return bRunning && Api->remove_attachment(TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(FilePath))) == BUGSPLAT_OK; }
void FBugSplatNative::SetQuietMode(bool bQuiet) { if (bRunning) Api->set_quiet_mode(bQuiet ? 1 : 0); }
bool FBugSplatNative::CaptureReport() { return bRunning && Api->capture_report() == BUGSPLAT_OK; }
void FBugSplatNative::Heartbeat() { if (bRunning) Api->heartbeat(); }
bool FBugSplatNative::WatchThread(const FString& Name) { return bRunning && Api->watch_thread(TCHAR_TO_UTF8(*Name)) == BUGSPLAT_OK; }
void FBugSplatNative::UnwatchThread() { if (bRunning) Api->unwatch_thread(); }
void FBugSplatNative::PostPendingReportsAsync() { if (bRunning) Api->post_pending_reports_async(); }

bool FBugSplatNative::PostFeedback(const FString& Title, const FString& Description, const TArray<FString>& Attachments, FBugSplatUploadResult& OutResult)
{
	if (!bRunning) return false;

	TArray<FTCHARToUTF8> Converted;
	TArray<const char*> Pointers;
	for (const FString& Path : Attachments)
	{
		Converted.Emplace(*FPaths::ConvertRelativePathToFull(Path));
	}
	for (const FTCHARToUTF8& C : Converted)
	{
		Pointers.Add(C.Get());
	}

	bugsplat_upload_result Raw = BUGSPLAT_UPLOAD_RESULT_INIT;
	const bugsplat_result Rc = Api->post_feedback(TCHAR_TO_UTF8(*Title), TCHAR_TO_UTF8(*Description),
		Pointers.Num() ? Pointers.GetData() : nullptr, static_cast<size_t>(Pointers.Num()), &Raw);
	OutResult = TakeResult(Rc, Raw, Api->upload_result_free);
	return OutResult.bSuccess;
}

bool FBugSplatNative::PostStructuredReport(const FString& Code, const FString& Explanation, const TArray<FString>& Frames, const TArray<FString>& Attachments, FBugSplatUploadResult& OutResult)
{
	if (!bRunning) return false;

	bugsplat_report* Report = Api->report_new(BUGSPLAT_REPORT_XML);
	if (!Report) return false;

	Api->report_set_platform(Report, "Unreal", TCHAR_TO_UTF8(*FPlatformMisc::GetOSVersion()));
	Api->report_set_exception(Report, TCHAR_TO_UTF8(*Code), TCHAR_TO_UTF8(*Explanation));
	const int32 Thread = Api->report_add_thread(Report, "GameThread", 1);
	if (Frames.Num() == 0)
	{
		Api->report_add_frame(Report, Thread, TCHAR_TO_UTF8(*Explanation), "", 0, "", 0);
	}
	for (const FString& Frame : Frames)
	{
		// "Function|File|Line" with the tail optional; anything else is the function text.
		TArray<FString> Parts;
		Frame.ParseIntoArray(Parts, TEXT("|"), false);
		const FString Function = Parts.Num() > 0 ? Parts[0] : Frame;
		const FString File = Parts.Num() > 1 ? Parts[1] : FString();
		const int32 Line = Parts.Num() > 2 ? FCString::Atoi(*Parts[2]) : 0;
		Api->report_add_frame(Report, Thread, TCHAR_TO_UTF8(*Function), TCHAR_TO_UTF8(*File), Line, "", 0);
	}
	for (const FString& Path : Attachments)
	{
		Api->report_add_attachment(Report, TCHAR_TO_UTF8(*FPaths::ConvertRelativePathToFull(Path)));
	}

	bugsplat_upload_result Raw = BUGSPLAT_UPLOAD_RESULT_INIT;
	const bugsplat_result Rc = Api->report_post(Report, &Raw);
	Api->report_free(Report);
	OutResult = TakeResult(Rc, Raw, Api->upload_result_free);
	return OutResult.bSuccess;
}

// ---- UBugSplatNative (Blueprint) ------------------------------------------------------------

bool UBugSplatNative::IsNativeCrashReportingRunning() { return FBugSplatNative::Get().IsRunning(); }
FString UBugSplatNative::GetNativeVersion() { return FBugSplatNative::Get().GetVersion(); }
FString UBugSplatNative::GetEnvironment() { return FBugSplatNative::Get().GetEnvironment(); }
bool UBugSplatNative::CaptureReport() { return FBugSplatNative::Get().CaptureReport(); }
bool UBugSplatNative::AddAttachment(const FString& FilePath) { return FBugSplatNative::Get().AddAttachment(FilePath); }
bool UBugSplatNative::RemoveAttachment(const FString& FilePath) { return FBugSplatNative::Get().RemoveAttachment(FilePath); }
void UBugSplatNative::SetUser(const FString& User) { FBugSplatNative::Get().SetUser(User); }
void UBugSplatNative::SetEmail(const FString& Email) { FBugSplatNative::Get().SetEmail(Email); }
void UBugSplatNative::SetKey(const FString& Key) { FBugSplatNative::Get().SetKey(Key); }
void UBugSplatNative::SetDescription(const FString& Description) { FBugSplatNative::Get().SetDescription(Description); }
void UBugSplatNative::SetNotes(const FString& Notes) { FBugSplatNative::Get().SetNotes(Notes); }
void UBugSplatNative::SetCrashDialogEnabled(bool bEnabled) { FBugSplatNative::Get().SetQuietMode(!bEnabled); }
void UBugSplatNative::PostPendingReports() { FBugSplatNative::Get().PostPendingReportsAsync(); }

void UBugSplatNative::PostStructuredReport(const FString& Code, const FString& Explanation, const TArray<FString>& Frames, const TArray<FString>& Attachments, const FBugSplatUploadComplete& OnComplete)
{
	if (!FBugSplatNative::Get().IsRunning())
	{
		UE_LOG(LogBugSplatNative, Warning, TEXT("PostStructuredReport: bugsplat-native is not running"));
		FBugSplatUploadResult Result;
		OnComplete.ExecuteIfBound(Result);
		return;
	}

	// The upload blocks until the server answers; keep it off the game thread and hand the
	// result back there, where Blueprint callbacks expect to run.
	Async(EAsyncExecution::Thread, [Code, Explanation, Frames, Attachments, OnComplete]()
	{
		FBugSplatUploadResult Result;
		FBugSplatNative::Get().PostStructuredReport(Code, Explanation, Frames, Attachments, Result);
		AsyncTask(ENamedThreads::GameThread, [Result, OnComplete]()
		{
			UE_LOG(LogBugSplatNative, Log, TEXT("Structured report %s (crashId=%lld http=%d) %s"), Result.bSuccess ? TEXT("uploaded") : TEXT("failed"), Result.CrashId, Result.HttpStatus, *Result.InfoUrl);
			OnComplete.ExecuteIfBound(Result);
		});
	});
}
