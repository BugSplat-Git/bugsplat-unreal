# Mobile Hang / ANR Reporting

Fatal hangs on mobile platforms are uploaded automatically by the BugSplat
plugin: ANRs on Android and main-thread hangs on iOS. This document describes
how each integration works, how to add a test button to your project, and how
to verify reports end-to-end.

## How it works

### Android (ANR)

The plugin bundles `bugsplat-android` v1.3.0+, which detects ANRs via the
Android `ApplicationExitInfo` API (Android 11 / API 30+). Detection is enabled
automatically on the first call to `BugSplat.init(...)` — there is no flag or
option to configure. On devices older than API 30, ANR detection is silently a
no-op; regular crash reporting continues to work.

When the OS kills the app due to an ANR, the SDK records the exit reason. On
the next app launch, the SDK reads `ApplicationExitInfo`, packages the thread
dump (Java stacks plus native frames with BuildIDs), and uploads it. Reports
appear in the BugSplat dashboard with crash type **`Android.ANR`**.

### iOS (App Hang)

The plugin bundles `bugsplat-apple` v3.2.0+, which monitors the main runloop
while the app is active in the foreground. Detection is opt-in: the plugin's
iOS init enables it by setting `enableHangDetection = YES` and a
`hangDetectionThreshold` of 2.0 s before calling `-start`, both in
`FBugSplatRuntimeModule::SetupCrashReportingIos` (`BugSplatRuntime.cpp`).

When the main thread is unresponsive past the threshold AND the app is later
terminated without recovering (watchdog kill, user force-quit), the SDK
persists a hang report and uploads it on the next launch using the same
pipeline as crash reports. **Non-fatal hangs (main thread eventually resumes)
are discarded** in this SDK version.

Reports carry the exception name `App Hang (Fatal)` and include attributes
prefixed with `bugsplat-hang-` (duration, detection time, app state, launch
id). Detection is suppressed when a debugger is attached.

## Triggering a test hang

The plugin exposes `UBugSplatUtils::GenerateHang()` as a Blueprint-callable
helper that works on both Android and iOS:

```cpp
UFUNCTION(BlueprintCallable, Category = "BugSplat")
static void GenerateHang();
```

- **Android:** calls the Java SDK's `BugSplat.hang()` helper, which blocks the
  main thread until the OS issues an ANR.
- **iOS:** dispatches an infinite spin onto the main queue, blocking the
  runloop until the iOS watchdog terminates the app (typically within
  ~20 seconds while in the foreground).
- **Other platforms:** logs a warning and is a no-op.

Companion platform helpers exposed as `BlueprintPure`:

```cpp
UFUNCTION(BlueprintPure, Category = "BugSplat")
static bool IsAndroid();

UFUNCTION(BlueprintPure, Category = "BugSplat")
static bool IsIOS();

UFUNCTION(BlueprintPure, Category = "BugSplat")
static bool IsMobile();  // IsAndroid() || IsIOS()
```

Use `IsMobile` to gate UI that should appear on Android and iOS but not
desktop.

## Adding a Hang button to a Widget Blueprint

The steps below describe wiring the Hang button into your test widget (e.g.
the `W_BugSplatDemo` widget shipped with the plugin's sample project). You
need the plugin C++ to be compiled first so the new UFUNCTIONs are visible in
the Blueprint node picker.

### 1. Open the widget

In the Content Browser, locate your test widget (for the plugin sample:
`BugSplat Content → UI → W_BugSplatDemo`) and double-click.

### 2. Add the button

In the **Hierarchy** panel, right-click an existing crash button and select
**Duplicate**. Rename the new entry to `BTN_Hang` and set its label to
`Trigger Hang`.

If the existing buttons are instances of a reusable `W_BugSplatButton` widget,
edit the exposed `Label` property. If they are raw `UButton`s, edit the child
`TextBlock`'s Text.

### 3. Wire OnClicked to GenerateHang

With `BTN_Hang` selected:

1. In the **Details** panel, scroll to the **Events** section.
2. Click **+** next to **On Clicked**. This drops you into the Event Graph.
3. Right-click on empty graph space, search for `Generate Hang`, place the
   node. (If it does not appear, uncheck **Context Sensitive** in the search
   popup. If still missing, the C++ has not been rebuilt — see Troubleshooting.)
4. Wire the execution pin from `On Clicked (BTN_Hang)` to `Generate Hang`.
5. Compile and Save.

### 4. Bind Visibility to IsMobile

Switch back to the **Designer** tab and select `BTN_Hang`.

1. In the **Details** panel, find **Behavior → Visibility**.
2. Click the **Bind** dropdown next to the Visibility property → **Create
   Binding**. This creates a function and opens its graph.
3. In the function graph:
   - Place a call to `Is Mobile` (under `Bug Splat Utils`).
   - Place a `Select` node.
   - Wire `Is Mobile` to the Select node's index/condition pin.
   - Set the True / Option 0 pin to `Visible`.
   - Set the False / Option 1 pin to `Collapsed`.
   - Wire the Select output to the function's return value.
4. Compile and Save.

> If you previously wired this binding to `Is Android`, just swap the node
> for `Is Mobile` — the rest of the graph stays the same.

## Verification

### In PIE (cheap sanity check)

Hit **Play** in the editor. The Hang button should be **Collapsed** (invisible)
because PIE reports the desktop platform, not Android or iOS. All other crash
buttons should still be visible and functional. This confirms the visibility
binding works.

### On-device (Android)

1. Package for Android (e.g. `File → Package Project → Android`).
2. Install the APK on an Android 11+ device.
3. Launch the app. The Hang button should now be visible.
4. Tap the Hang button. After roughly 5 seconds the system ANR dialog
   appears.
5. Tap **Close app**. (Do not tap **Wait** — the SDK relies on the system
   killing the process to record the ANR via `ApplicationExitInfo`.)
6. Launch the app one more time. On startup, the SDK reads the exit reason
   and uploads the ANR report in the background.
7. In the BugSplat dashboard, filter by your database / app / version. A new
   report with crash type **`Android.ANR`** should appear.

### On-device (iOS)

1. Package for iOS (`File → Package Project → iOS`).
2. Install the IPA on a physical iOS device. **Detach the debugger before
   testing** — iOS hang detection is suppressed while a debugger is attached.
3. Launch the app. The Hang button should now be visible.
4. Tap the Hang button. The UI freezes immediately; within ~20 seconds the OS
   watchdog terminates the app.
5. Launch the app one more time. On startup, the SDK reads the persisted hang
   report and uploads it via the normal crash-report pipeline.
6. In the BugSplat dashboard, a new report with exception name **`App Hang
   (Fatal)`** should appear. Attributes prefixed with `bugsplat-hang-` carry
   the duration, app state, and launch id.

## Troubleshooting

- **`Generate Hang` / `Is Mobile` does not show in the Blueprint node
  picker:** the plugin C++ has not been rebuilt since the function was added.
  Close the editor, rebuild the project, and reopen.

- **The Hang button is still visible on desktop:** the visibility binding's
  Select node has `Visible` and `Collapsed` swapped. Confirm `Is Mobile`
  returns `false` on desktop and that the False branch maps to `Collapsed`.

- **(iOS) The app freezes but no report uploads:** the device may have been
  attached to a debugger (Xcode, lldb), which suppresses hang detection.
  Detach and retry. Also confirm the next launch happened — uploads occur on
  the launch *after* the hang.

- **(iOS) The button is visible in PIE on a Mac:** `Is Mobile` returns false
  for Mac/Win64. If the button is still visible, check the Select node wiring
  per the Visibility step above.

- **(Android) The ANR dialog appears but no report shows up:** verify the
  device is on Android 11+ (API 30+). On older Android versions the SDK
  cannot read `ApplicationExitInfo` and silently skips ANR upload. Regular
  crash reporting is unaffected.

- **(Android) The report does not upload:** the upload happens on the
  **next** app launch after the ANR, not the same session. After tapping
  **Close app**, open the app once more and wait for the BugSplat init logs.

- **(Android) `Wait` was tapped instead of `Close app`:** no report is
  generated because the process was not killed. Trigger the ANR again and
  let the system close the app.
