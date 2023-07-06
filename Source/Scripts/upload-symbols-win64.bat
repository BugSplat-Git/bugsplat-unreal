@echo off

set targetPlatform=%1
set projectDir=%2
set uploadScriptPath=%projectDir%\Plugins\BugSplat\Source\Scripts\BugSplat.bat

echo "BugSplat [INFO]: invoking upload script at %uploadScriptPath%"

if not exist "%uploadScriptPath%" (
	echo "BugSplat [WARN]: symbol uploads not configured via plugin - skipping..."
	exit /b
)

call %uploadScriptPath% %targetPlatform%
