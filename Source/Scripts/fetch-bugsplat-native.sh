#!/bin/sh
# Populates Source/ThirdParty/BugSplatNative/<Platform> with a bugsplat-native release so the
# plugin's desktop path (Project Settings > Plugins > BugSplat > Desktop) has a runtime to load.
#   sh Source/Scripts/fetch-bugsplat-native.sh 9.0.0 [Win64|Mac|Linux|all] [local.zip]
set -eu
VERSION="${1:-9.0.0}"
PLATFORM="${2:-all}"
ARCHIVE="${3:-}"
BASE="${BUGSPLAT_NATIVE_RELEASES:-https://github.com/BugSplat-Git/bugsplat-native/releases/download}"
PLUGIN="$(cd "$(dirname "$0")/../.." && pwd)"
ROOT="$PLUGIN/Source/ThirdParty/BugSplatNative"

layout() {
  case "$1" in
    Win64) echo "windows-x64|BugSplat.dll BugSplatMonitor.exe BugSplatReporter.exe BugSplatWer.dll theme" ;;
    Mac)   echo "macos|libbugsplat.dylib BugSplatMonitor BugSplatReporter.app theme" ;;
    Linux) echo "linux-x64|libbugsplat.so BugSplatMonitor BugSplatReporter theme" ;;
    *) echo "unknown platform $1" >&2; exit 2 ;;
  esac
}

install() {
  name="$1"
  spec="$(layout "$name")"
  archive="$(echo "$spec" | cut -d'|' -f1)"; files="$(echo "$spec" | cut -d'|' -f2)"
  zip="$ARCHIVE"
  if [ -z "$zip" ]; then
    zip="${TMPDIR:-/tmp}/bugsplat-native-$VERSION-$archive.zip"
    echo "Downloading $BASE/v$VERSION/bugsplat-native-$VERSION-$archive.zip"
    curl -fsSL -o "$zip" "$BASE/v$VERSION/bugsplat-native-$VERSION-$archive.zip"
  fi
  stage="$(mktemp -d)"
  unzip -q -o "$zip" -d "$stage"
  mkdir -p "$ROOT/$name" "$ROOT/include/bugsplat"
  for f in $files; do
    src="$(find "$stage" -name "$f" | head -n 1)"
    [ -n "$src" ] || { echo "$f not found in $zip" >&2; exit 1; }
    rm -rf "$ROOT/$name/$f"; cp -R "$src" "$ROOT/$name/$f"; echo "  $name/$f"
  done
  header="$(find "$stage" -name bugsplat.h | head -n 1)"
  [ -z "$header" ] || cp -f "$header" "$ROOT/include/bugsplat/bugsplat.h"
  chmod -R +x "$ROOT/$name" 2>/dev/null || true
  rm -rf "$stage"
}

if [ "$PLATFORM" = "all" ]; then
  for p in Win64 Mac Linux; do echo "== $p"; install "$p"; done
else
  install "$PLATFORM"
fi
echo "Done. The files are staged with the plugin by BugSplatRuntime.Build.cs; sign the macOS helpers with the app."
