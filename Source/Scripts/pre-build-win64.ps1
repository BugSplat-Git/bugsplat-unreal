# Copyright (C) BugSplat. All Rights Reserved.

$targetName = $args[0]
$pluginPath = $args[1]
$symbolUploaderFolderPath = Join-Path $pluginPath "Source\ThirdParty\SymUploader"
$symbolUploaderPath = Join-Path $symbolUploaderFolderPath "symbol-upload-windows.exe"

Write-Host "BugSplat [INFO]: Pre-build step invoked with parameters: $targetName"

if ($targetName -like '*Editor*') {
    Write-Host 'BugSplat [INFO]: Skipping pre-build step for Editor build...'
    exit 0
}

if (-not (Test-Path $symbolUploaderPath)) {
    Write-Host "BugSplat [INFO]: File $symbolUploaderPath not found - downloading..."
    New-Item -ItemType Directory -Force -Path $symbolUploaderFolderPath | Out-Null
    Invoke-WebRequest -Uri "https://app.bugsplat.com/download/symbol-upload-windows.exe" -OutFile $symbolUploaderPath
}

Write-Host "BugSplat [INFO]: Pre-build step completed successfully"
