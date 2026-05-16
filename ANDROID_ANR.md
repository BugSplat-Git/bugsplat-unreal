# Android ANR Reporting

ANR (Application Not Responding) reports are uploaded automatically by the
BugSplat plugin on Android. This document describes how the integration works,
how to add a test button to your project, and how to verify reports end-to-end.

## How it works

The plugin bundles `bugsplat-android` v1.3.0+, which detects ANRs via the
Android `ApplicationExitInfo` API (Android 11 / API 30+). Detection is enabled
automatically on the first call to `BugSplat.init(...)` — there is no flag or
option to configure. On devices older than API 30, ANR detection is silently a
no-op; regular crash reporting continues to work.

When the OS kills the app due to an ANR, the SDK records the exit reason. On
the next app launch, the SDK reads `ApplicationExitInfo`, packages the thread
dump (Java stacks plus native frames with BuildIDs), and uploads it. Reports
appear in the BugSplat dashboard with crash type **`Android.ANR`**.

The plugin's C++ initialization flow (`FBugSplatRuntimeModule::SetupCrashReportingAndroid`)
calls the Java SDK's `init` via JNI — no changes to that flow are required to
enable ANR reporting beyond using the upgraded AAR.

## Triggering a test ANR

The Java SDK exposes `BugSplat.hang()` for deliberately blocking the main
thread so the OS will fire the ANR dialog. The plugin exposes this via the
Blueprint-callable `UBugSplatUtils::GenerateHang()`:

```cpp
UFUNCTION(BlueprintCallable, Category = "BugSplat")
static void GenerateHang();
```

On non-Android platforms `GenerateHang` logs a warning and is a no-op.

A companion `UBugSplatUtils::IsAndroid()` is exposed as `BlueprintPure` so UI
elements can be conditionally shown on Android only.

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
`Trigger Hang (ANR)`.

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

### 4. Bind Visibility to IsAndroid

Switch back to the **Designer** tab and select `BTN_Hang`.

1. In the **Details** panel, find **Behavior → Visibility**.
2. Click the **Bind** dropdown next to the Visibility property → **Create
   Binding**. This creates a function and opens its graph.
3. In the function graph:
   - Place a call to `Is Android` (under `Bug Splat Utils`).
   - Place a `Select` node.
   - Wire `Is Android` to the Select node's index/condition pin.
   - Set the True / Option 0 pin to `Visible`.
   - Set the False / Option 1 pin to `Collapsed`.
   - Wire the Select output to the function's return value.
4. Compile and Save.

## Verification

### In PIE (cheap sanity check)

Hit **Play** in the editor. The Hang button should be **Collapsed** (invisible)
because PIE reports the desktop platform, not Android. All other crash
buttons should still be visible and functional. This confirms the visibility
binding works.

### On-device

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

## Troubleshooting

- **`Generate Hang` does not show in the Blueprint node picker:** the plugin
  C++ has not been rebuilt since the function was added. Close the editor,
  rebuild the project, and reopen.

- **The Hang button is still visible on desktop:** the visibility binding's
  Select node has `Visible` and `Collapsed` swapped. Confirm `Is Android`
  returns `false` on desktop and that the False branch maps to `Collapsed`.

- **The ANR dialog appears but no report shows up:** verify the device is on
  Android 11+ (API 30+). On older Android versions the SDK cannot read
  `ApplicationExitInfo` and silently skips ANR upload. Regular crash
  reporting is unaffected.

- **The report does not upload:** the upload happens on the **next** app
  launch after the ANR, not the same session. After tapping **Close app**,
  open the app once more and wait for the BugSplat init logs.

- **`Wait` was tapped instead of `Close app`:** no report is generated
  because the process was not killed. Trigger the ANR again and let the
  system close the app.
