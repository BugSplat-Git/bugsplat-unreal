$targetPlatform = $args[0]
$projectDir = $args[1]
$targetName = $args[2]
$uploadScriptPath = Join-Path $projectDir "Plugins\BugSplat\Source\Scripts\BugSplat.bat"
$symbolUploaderFolderPath = Join-Path $projectDir "Plugins\BugSplat\Source\ThirdParty\SymUploader"
$symbolUploaderPath = Join-Path $symbolUploaderFolderPath "symbol-upload-windows.exe"

Write-Host "BugSplat [INFO]: invoking upload script at $uploadScriptPath"

if (-not (Test-Path $uploadScriptPath)) {
    Write-Host "BugSplat [WARN]: symbol uploads not configured via plugin - skipping..."
    exit
}

if (-not (Test-Path $symbolUploaderPath)) {
    Write-Host "BugSplat [INFO]: $symbolUploaderPath not found - downloading..."
    New-Item -ItemType Directory -Force -Path $symbolUploaderFolderPath | Out-Null
    Invoke-WebRequest -Uri "https://app.bugsplat.com/download/symbol-upload-windows.exe" -OutFile $symbolUploaderPath
}

& $uploadScriptPath $targetPlatform $targetName
