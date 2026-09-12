# bugsplat-native runtime for the desktop path

`BugSplatRuntime` loads BugSplat's native SDK ([bugsplat-native](https://github.com/BugSplat-Git/bugsplat-native), C ABI in `include/bugsplat/bugsplat.h`) from this folder at runtime when **Project Settings > Plugins > BugSplat > Desktop > Enable bugsplat-native on Desktop** is on. Nothing links against it, so the plugin builds and packages with or without these files; `BugSplatRuntime.Build.cs` stages whatever is present next to the plugin in a packaged game.

| Platform | Files |
|---|---|
| `Win64/` | `BugSplat.dll`, `BugSplatMonitor.exe`, `BugSplatReporter.exe`, `BugSplatWer.dll`, `theme/` |
| `Mac/` | `libbugsplat.dylib`, `BugSplatMonitor`, `BugSplatReporter.app/`, `theme/` (sign them with the app) |
| `Linux/` | `libbugsplat.so`, `BugSplatMonitor`, `BugSplatReporter`, `theme/` (keep them executable) |

`Win64/` in this repository was built from bugsplat-native `feat/m1-core` (9.0.0-dev). Populate the others from a bugsplat-native release:

```
pwsh Source/Scripts/fetch-bugsplat-native.ps1 -Version 9.0.0 -Platform all
sh   Source/Scripts/fetch-bugsplat-native.sh 9.0.0 all
```

`theme/` is BugSplatReporter's dialog theme (`theme.json`, `strings.<locale>.json`); edit it to brand the crash dialog.
