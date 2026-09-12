/*
 * bugsplat.h - BugSplat native crash reporting, public C ABI (version 1).
 *
 * Design contract (see docs/ARCHITECTURE.md):
 *   - UTF-8 everywhere; all strings are copied on call and never retained.
 *   - Opaque handles; out-structs carry struct_size so the ABI can grow.
 *   - No exceptions cross this boundary; every fallible call returns bugsplat_result.
 *   - Everything after bugsplat_init() is thread-safe unless noted.
 *   - Capture, dump writing, the dialog and the upload all happen out of process
 *     (BugSplatMonitor + BugSplatReporter). A missing monitor or reporter is a
 *     packaging error reported by bugsplat_init(), never a silent degradation.
 *   - Attachments and attributes may be added, changed or removed at any time
 *     during the process lifetime; nothing is frozen at init.
 *
 * Copyright (c) 2026 BugSplat, LLC. MIT license.
 */
#ifndef BUGSPLAT_H
#define BUGSPLAT_H

#include <stddef.h>
#include <stdint.h>

#define BUGSPLAT_ABI_VERSION 1u
#define BUGSPLAT_SDK_VERSION "9.0.0-dev"

/* ---- export / calling convention ----------------------------------------- */
#if defined(_WIN32)
#  define BUGSPLAT_CALL __cdecl
#  if defined(BUGSPLAT_STATIC)
#    define BUGSPLAT_API
#  elif defined(BUGSPLAT_EXPORTS)
#    define BUGSPLAT_API __declspec(dllexport)
#  else
#    define BUGSPLAT_API __declspec(dllimport)
#  endif
#else
#  define BUGSPLAT_CALL
#  if defined(BUGSPLAT_STATIC)
#    define BUGSPLAT_API
#  else
#    define BUGSPLAT_API __attribute__((visibility("default")))
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---- enums ---------------------------------------------------------------- */

/** Result of every fallible call. Values are stable across ABI version 1. */
typedef enum bugsplat_result {
  BUGSPLAT_OK = 0,
  BUGSPLAT_ERR_INVALID_ARGUMENT = 1,
  BUGSPLAT_ERR_ALREADY_INITIALIZED = 2,
  BUGSPLAT_ERR_NOT_INITIALIZED = 3,
  BUGSPLAT_ERR_MONITOR_NOT_FOUND = 4,   /**< BugSplatMonitor missing next to the app / in the framework. */
  BUGSPLAT_ERR_MONITOR_START_FAILED = 5,
  BUGSPLAT_ERR_REPORTER_NOT_FOUND = 6,  /**< BugSplatReporter missing (desktop platforms). */
  BUGSPLAT_ERR_IO = 7,
  BUGSPLAT_ERR_LIMIT_EXCEEDED = 8,      /**< 24 attachments, 64 attributes, 20 KB per value. */
  BUGSPLAT_ERR_UNSUPPORTED = 9,         /**< Not available on this platform / backend. */
  BUGSPLAT_ERR_NETWORK = 10,
  BUGSPLAT_ERR_HTTP = 11,               /**< See bugsplat_upload_result.http_status. */
  BUGSPLAT_ERR_REJECTED = 12,           /**< Server refused the report (size limit, throttling). */
  BUGSPLAT_ERR_CANCELLED = 13,          /**< User chose not to send. */
  BUGSPLAT_ERR_INTERNAL = 14
} bugsplat_result;

/** What happens to a report once it exists in the store. */
typedef enum bugsplat_upload_policy {
  BUGSPLAT_UPLOAD_DIALOG = 0,  /**< Show the BugSplat dialog, then upload (desktop) / prompt next launch (mobile). */
  BUGSPLAT_UPLOAD_QUIET = 1,   /**< Upload without UI. */
  BUGSPLAT_UPLOAD_MANUAL = 2   /**< Leave reports pending; the app drains them with bugsplat_pending_reports(). */
} bugsplat_upload_policy;

/** How much of the process memory the dump carries. Size is enforced server-side at upload. */
typedef enum bugsplat_dump_type {
  BUGSPLAT_DUMP_NORMAL = 0,  /**< Threads, stacks, modules, exception (Crashpad default). */
  BUGSPLAT_DUMP_HEAP = 1,    /**< Plus private writable memory (CLR/GC/native heaps). */
  BUGSPLAT_DUMP_FULL = 2     /**< Every readable region. */
} bugsplat_dump_type;

/** Hang policy once the watchdog fires. */
typedef enum bugsplat_hang_policy {
  BUGSPLAT_HANG_REPORT = 0,                /**< Non-fatal: dump the live process, report, continue. */
  BUGSPLAT_HANG_REPORT_AND_TERMINATE = 1   /**< Fatal: report, offer Wait / Close, terminate on Close. */
} bugsplat_hang_policy;

/** What the in-process filter does after the monitor has written the dump. */
typedef enum bugsplat_crash_completion {
  BUGSPLAT_COMPLETION_EXIT = 0,
  BUGSPLAT_COMPLETION_TERMINATE = 1,
  BUGSPLAT_COMPLETION_CONTINUE_SEARCH = 2  /**< Let the next handler run (Windows). */
} bugsplat_crash_completion;

/** Structured (client-generated) report serialization. */
typedef enum bugsplat_report_format {
  BUGSPLAT_REPORT_XML = 0,   /**< BugSplat's existing bsCrashReport.xml schema. */
  BUGSPLAT_REPORT_JSON = 1   /**< 1:1 JSON mirror of the XML schema. */
} bugsplat_report_format;

/** Capability queries; answers depend on platform, backend and configuration. */
typedef enum bugsplat_capability {
  BUGSPLAT_CAP_OUT_OF_PROCESS = 1,
  BUGSPLAT_CAP_WER = 2,                 /**< Windows: BugSplatWer.dll registered and allow-listed. */
  BUGSPLAT_CAP_HANG_DETECTION = 3,
  BUGSPLAT_CAP_CRASH_DIALOG = 4,
  BUGSPLAT_CAP_ON_CRASH_CALLBACK = 5,   /**< Never on Apple platforms. */
  BUGSPLAT_CAP_FULL_MEMORY_DUMP = 6,    /**< Desktop only. */
  BUGSPLAT_CAP_DYNAMIC_ATTACHMENTS = 7,
  BUGSPLAT_CAP_CRASH_SIGNATURE = 8
} bugsplat_capability;

typedef enum bugsplat_log_level {
  BUGSPLAT_LOG_DEBUG = 0,
  BUGSPLAT_LOG_INFO = 1,
  BUGSPLAT_LOG_WARN = 2,
  BUGSPLAT_LOG_ERROR = 3
} bugsplat_log_level;

/* ---- opaque handles and out-structs -------------------------------------- */

typedef struct bugsplat_options bugsplat_options;
typedef struct bugsplat_report bugsplat_report;            /**< Structured report builder. */
typedef struct bugsplat_report_list bugsplat_report_list;  /**< Snapshot of pending reports. */

/** Filled by every upload. Free info_url with bugsplat_upload_result_free(). */
typedef struct bugsplat_upload_result {
  uint32_t struct_size;      /**< Set to sizeof(bugsplat_upload_result) before the call. */
  bugsplat_result result;
  int32_t http_status;       /**< Last HTTP status, 0 if none. */
  int64_t crash_id;          /**< 0 until the server accepted the report. */
  int64_t stack_key_id;      /**< 0 while the server is still processing. */
  char* info_url;            /**< Support-response URL, NULL if none; owned by the caller. */
} bugsplat_upload_result;

#define BUGSPLAT_UPLOAD_RESULT_INIT { (uint32_t)sizeof(bugsplat_upload_result), BUGSPLAT_OK, 0, 0, 0, NULL }

/**
 * Best-effort in-process crash callback. Runs on the faulting thread in the
 * crashing process where a hook exists (Windows, Linux, Android); never on
 * Apple platforms. Must be async-signal-safe; must not allocate, lock or call
 * back into bugsplat_*. Put state in attributes ahead of time instead.
 */
typedef void (BUGSPLAT_CALL *bugsplat_on_crash_fn)(void* user_data);
typedef void (BUGSPLAT_CALL *bugsplat_log_fn)(bugsplat_log_level level, const char* message, void* user_data);

/* ---- options -------------------------------------------------------------- */

BUGSPLAT_API bugsplat_options* BUGSPLAT_CALL bugsplat_options_new(const char* database,
                                                                    const char* app_name,
                                                                    const char* app_version);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_free(bugsplat_options* options);

BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_upload_policy(bugsplat_options*, bugsplat_upload_policy);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_store_dir(bugsplat_options*, const char* path);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_monitor_path(bugsplat_options*, const char* path);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_reporter_path(bugsplat_options*, const char* path);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_theme_dir(bugsplat_options*, const char* path);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_crash_type_id(bugsplat_options*, int32_t crash_type_id);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_dump_type(bugsplat_options*, bugsplat_dump_type);
/** Windows only: raw MINIDUMP_TYPE flags for the DbgHelp dump; overrides set_dump_type. */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_minidump_flags(bugsplat_options*, uint32_t minidump_type);
/** timeout_ms = 0 disables hang detection. */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_hang_detection(bugsplat_options*, int32_t timeout_ms, bugsplat_hang_policy);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_crash_completion(bugsplat_options*, bugsplat_crash_completion);
/** Windows only: guard SetUnhandledExceptionFilter against later replacement. Default on. */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_protect_exception_filter(bugsplat_options*, int enabled);
/** Open the support-response URL after a successful interactive upload. Default on (desktop), off (mobile). */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_open_support_url(bugsplat_options*, int enabled);
/** Compute crashSignature/crashHash before upload. Default on. */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_crash_signature(bugsplat_options*, int enabled);
/** Linux/Android hosts with their own signal handlers (.NET, Mono, JVM): defer to the previous handler first. */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_chain_previous_signal_handlers(bugsplat_options*, int enabled);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_on_crash(bugsplat_options*, bugsplat_on_crash_fn, void* user_data);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_logger(bugsplat_options*, bugsplat_log_fn, void* user_data);

/* Initial values; every one of these can be changed later with the bugsplat_set_* calls. */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_key(bugsplat_options*, const char* key);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_user(bugsplat_options*, const char* user);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_email(bugsplat_options*, const char* email);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_user_description(bugsplat_options*, const char* description);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_notes(bugsplat_options*, const char* notes);
/**
 * Environment is a first-class report property like user and email. The SDK fills it in
 * automatically with the OS and hardware it runs on, e.g. "Windows 11 (10.0.26200) x64",
 * "macOS 14.5 (23F79) arm64", "Android 14 (API 34) arm64-v8a; Google Pixel 8". Set it here
 * (or later with bugsplat_set_environment) to override the detected value.
 */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_environment(bugsplat_options*, const char* environment);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_set_attribute(bugsplat_options*, const char* name, const char* value);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_options_add_attachment(bugsplat_options*, const char* path);

/* ---- lifecycle ------------------------------------------------------------ */

/** Takes ownership of options (even on failure). Once per process. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_init(bugsplat_options* options);
BUGSPLAT_API int BUGSPLAT_CALL bugsplat_is_initialized(void);
BUGSPLAT_API int BUGSPLAT_CALL bugsplat_has_capability(bugsplat_capability capability);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_shutdown(void);

/* ---- dynamic metadata: any time, any thread ------------------------------- */

BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_set_key(const char* key);
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_set_user(const char* user);
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_set_email(const char* email);
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_set_user_description(const char* description);
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_set_notes(const char* notes);
/** Overrides the auto-detected environment; NULL restores detection. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_set_environment(const char* environment);
/** The environment string the SDK detected or was given; valid until the next set/shutdown. NULL before init. */
BUGSPLAT_API const char* BUGSPLAT_CALL bugsplat_get_environment(void);
/** value == NULL removes the attribute. Captured at the instant of the crash. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_set_attribute(const char* name, const char* value);
/** The file is copied into the report by the monitor right after the dump. Not callable from on_crash. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_add_attachment(const char* path);
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_remove_attachment(const char* path);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_set_quiet_mode(int enabled);

/* ---- reports that do not end the process ---------------------------------- */

/** Dump the live process out of process (Crashpad DumpWithoutCrash); flows like a crash. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_capture_report(void);

/** Structured report builder: for engines, script runtimes and managed code. */
BUGSPLAT_API bugsplat_report* BUGSPLAT_CALL bugsplat_report_new(bugsplat_report_format format);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_report_free(bugsplat_report*);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_report_set_platform(bugsplat_report*, const char* platform, const char* os);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_report_set_exception(bugsplat_report*, const char* code, const char* explanation);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_report_add_module(bugsplat_report*, const char* name, const char* path,
                                                            uint64_t base_address, uint64_t size,
                                                            const char* file_version, const char* product_version);
/** Returns the thread index to pass to bugsplat_report_add_frame. */
BUGSPLAT_API int32_t BUGSPLAT_CALL bugsplat_report_add_thread(bugsplat_report*, const char* thread_id, int is_crashing_thread);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_report_add_frame(bugsplat_report*, int32_t thread_index,
                                                           const char* function, const char* file, int32_t line,
                                                           const char* module, uint64_t address);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_report_add_register(bugsplat_report*, const char* name, const char* value);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_report_add_attachment(bugsplat_report*, const char* path);
/** Serializes, stores, applies the upload policy, uploads; blocks until done unless policy is MANUAL. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_report_post(bugsplat_report*, bugsplat_upload_result* out);

/** AddressSanitizer text report, as today's CreateAsanReport. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_post_asan_report(const char* asan_text, bugsplat_upload_result* out);

/** User feedback (crash type 36). attachments may be NULL when count is 0. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_post_feedback(const char* title, const char* description,
                                                                   const char* const* attachments, size_t attachment_count,
                                                                   bugsplat_upload_result* out);

/* ---- hang detection ------------------------------------------------------- */

/** For apps without a platform main loop the SDK can ping: call at least once per timeout. */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_heartbeat(void);
/** Watch the calling thread in addition to the main thread; name appears in the hang report. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_watch_thread(const char* name);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_unwatch_thread(void);

/* ---- pending reports: MANUAL policy, mobile drains, safety net ------------ */

BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_post_pending_reports_async(void);
BUGSPLAT_API bugsplat_report_list* BUGSPLAT_CALL bugsplat_pending_reports(void);
BUGSPLAT_API size_t BUGSPLAT_CALL bugsplat_report_list_count(const bugsplat_report_list*);
/** Folder path of the report; valid until the list is freed. */
BUGSPLAT_API const char* BUGSPLAT_CALL bugsplat_report_list_folder(const bugsplat_report_list*, size_t index);
/** The report's BugSplatCrashData.json text; valid until the list is freed. */
BUGSPLAT_API const char* BUGSPLAT_CALL bugsplat_report_list_json(const bugsplat_report_list*, size_t index);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_report_list_free(bugsplat_report_list*);
/** user/email/description may be NULL to keep the stored values. */
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_send_report(const char* folder, const char* user, const char* email,
                                                                 const char* description, bugsplat_upload_result* out);
BUGSPLAT_API bugsplat_result BUGSPLAT_CALL bugsplat_discard_report(const char* folder);

/* ---- diagnostics and memory ---------------------------------------------- */

BUGSPLAT_API const char* BUGSPLAT_CALL bugsplat_version_string(void);   /**< BUGSPLAT_SDK_VERSION of the loaded library. */
BUGSPLAT_API uint32_t BUGSPLAT_CALL bugsplat_abi_version(void);
BUGSPLAT_API const char* BUGSPLAT_CALL bugsplat_log_file_path(void);    /**< NULL before init. */
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_free(void* p);
BUGSPLAT_API void BUGSPLAT_CALL bugsplat_upload_result_free(bugsplat_upload_result* result);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* BUGSPLAT_H */
