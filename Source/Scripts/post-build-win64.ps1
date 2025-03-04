# Copyright (C) BugSplat. All Rights Reserved.

$targetPlatform = $args[0]
$targetName = $args[1]
$projectDir = $args[2]
$pluginPath = $args[3]

$uploadScriptPath = Join-Path $pluginPath "Source\Scripts\upload-symbols-win64.ps1"
$symbolUploaderFolderPath = Join-Path $pluginPath "Source\ThirdParty\SymUploader"
$symbolUploaderPath = Join-Path $symbolUploaderFolderPath "symbol-upload-windows.exe"

Write-Host "BugSplat [INFO]: Post-build step invoked with parameters: $targetPlatform, $targetName"

if ($targetName -like '*Editor*') {
    Write-Host 'BugSplat [INFO]: Skipping symbol upload for Editor build...'
    exit 0
}

if (-not (Test-Path $uploadScriptPath)) {
    Write-Host "BugSplat [WARN]: Symbol uploads not configured via plugin - skipping..."
    exit
}

Write-Host "BugSplat [INFO]: Invoking symbol upload script at $uploadScriptPath"

& "$uploadScriptPath" $targetPlatform $targetName
