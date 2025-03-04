$targetPlatform = $args[0]
$projectDir = $args[1]
$targetName = $args[2]
$uploadScriptPath = Join-Path $projectDir "Plugins\BugSplat\Source\Scripts\upload-symbols-win64.ps1"
$symbolUploaderFolderPath = Join-Path $projectDir "Plugins\BugSplat\Source\ThirdParty\SymUploader"
$symbolUploaderPath = Join-Path $symbolUploaderFolderPath "symbol-upload-windows.exe"

Write-Host "BugSplat [INFO]: Post-build step invoked with parameters: $targetPlatform, $targetName"

if (-not (Test-Path $uploadScriptPath)) {
    Write-Host "BugSplat [WARN]: Symbol uploads not configured via plugin - skipping..."
    exit
}

if (-not (Test-Path $symbolUploaderPath)) {
    Write-Host "BugSplat [INFO]: File $symbolUploaderPath not found - downloading..."
    New-Item -ItemType Directory -Force -Path $symbolUploaderFolderPath | Out-Null
    Invoke-WebRequest -Uri "https://app.bugsplat.com/download/symbol-upload-windows.exe" -OutFile $symbolUploaderPath
}

if ($targetName -like '*Editor*') {
    Write-Host 'BugSplat [WARN]: Skipping symbol upload for Editor build...'
    exit 0
}

Write-Host "BugSplat [INFO]: Invoking upload script at $uploadScriptPath"

& "$uploadScriptPath" $targetPlatform $targetName
