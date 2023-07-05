echo off

set targetPlatform=%1
set pluginDir=%2
set uploadScriptPath=%pluginDir%\BugSplat.bat

IF NOT EXIST %uploadScriptPath% (
	echo "BugSplat [WARN]: symbol uploads not configured via plugin - skipping..."
	exit /b
)

call %uploadScriptPath% %targetPlatform%