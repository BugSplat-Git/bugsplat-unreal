<#
.SYNOPSIS
  Populates Source/ThirdParty/BugSplatNative/<Platform> with a bugsplat-native release so the
  plugin's desktop path (Project Settings > Plugins > BugSplat > Desktop) has a runtime to load.
.EXAMPLE
  pwsh Source/Scripts/fetch-bugsplat-native.ps1 -Version 9.0.0 -Platform Win64
  pwsh Source/Scripts/fetch-bugsplat-native.ps1 -Platform Mac -Archive ~/Downloads/bugsplat-native-9.0.0-macos.zip
#>
param(
  [string] $Version = "9.0.0",
  [ValidateSet("Win64", "Mac", "Linux", "all")] [string] $Platform = "all",
  [string] $Archive = "",
  [string] $ReleaseBase = "https://github.com/BugSplat-Git/bugsplat-native/releases/download"
)
$ErrorActionPreference = "Stop"
$plugin = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$root = Join-Path $plugin "Source/ThirdParty/BugSplatNative"

$layout = @{
  "Win64" = @{ archive = "windows-x64"; files = @("BugSplat.dll", "BugSplatMonitor.exe", "BugSplatReporter.exe", "BugSplatWer.dll", "theme") }
  "Mac"   = @{ archive = "macos";       files = @("libbugsplat.dylib", "BugSplatMonitor", "BugSplatReporter.app", "theme") }
  "Linux" = @{ archive = "linux-x64";   files = @("libbugsplat.so", "BugSplatMonitor", "BugSplatReporter", "theme") }
}

function Install-Platform([string] $name) {
  $entry = $layout[$name]
  $zip = $Archive
  if (-not $zip) {
    $zip = Join-Path ([System.IO.Path]::GetTempPath()) "bugsplat-native-$Version-$($entry.archive).zip"
    $url = "$ReleaseBase/v$Version/bugsplat-native-$Version-$($entry.archive).zip"
    Write-Host "Downloading $url"
    Invoke-WebRequest -Uri $url -OutFile $zip
  }
  $stage = Join-Path ([System.IO.Path]::GetTempPath()) ("bugsplat-native-" + $name + "-" + [guid]::NewGuid().ToString("N"))
  Expand-Archive -Path $zip -DestinationPath $stage -Force
  $dest = Join-Path $root $name
  New-Item -ItemType Directory -Force $dest | Out-Null
  foreach ($file in $entry.files) {
    $src = Get-ChildItem -Path $stage -Recurse -Filter $file | Select-Object -First 1
    if (-not $src) { throw "$file not found in $zip" }
    Copy-Item -Recurse -Force $src.FullName (Join-Path $dest $file)
    Write-Host "  $name/$file"
  }
  $header = Get-ChildItem -Path $stage -Recurse -Filter "bugsplat.h" | Select-Object -First 1
  if ($header) {
    New-Item -ItemType Directory -Force (Join-Path $root "include/bugsplat") | Out-Null
    Copy-Item -Force $header.FullName (Join-Path $root "include/bugsplat/bugsplat.h")
  }
  Remove-Item -Recurse -Force $stage
}

$targets = if ($Platform -eq "all") { @("Win64", "Mac", "Linux") } else { @($Platform) }
foreach ($t in $targets) { Write-Host "== $t"; Install-Platform $t }
Write-Host "Done. The files are staged with the plugin by BugSplatRuntime.Build.cs; on macOS and Linux keep them executable and sign the macOS helpers with the app."
