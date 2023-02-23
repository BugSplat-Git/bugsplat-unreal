#!/bin/bash
# Copyright (C) BugSplat. All Rights Reserved.

export targetPlatform=$1
export targetName=$2
export projectPath=$3
export pluginPath=$4

export binariesPath=$projectPath/Binaries/Android
export intermediatePath=$projectPath/Intermediate/Android
export configPath=$projectPath/Config
export scriptsPath=$pluginPath/Source/Scripts

echo "BugSplat postprocessing: Start debug symbols upload for Android"

if [ $targetPlatform != "Android" ]; then
    echo "BugSplat postprocessing: Unexpected platform ${targetPlatform}. Terminating..." 
    exit
fi

echo "BugSplat postprocessing: Input target name: $targetName" 
echo "BugSplat postprocessing: Input binaries path: $binariesPath"
echo "BugSplat postprocessing: Input config path: $configPath"
echo "BugSplat postprocessing: Input scripts path: $scriptsPath"

export reportCrashes=$(awk -F "=" '/bEnableCrashReportingAndroid/ {print $2}' ${configPath}/DefaultEngine.ini)

# By default crash reporting for Android is enabled and hence not included in DefaultEngine.ini
if [ ! -z "$reportCrashes" ]; then
    if [ $reportCrashes != "True" ]; then
        echo "BugSplat postprocessing: Crash reporting is disabled in plugin settings. Terminating..." 
        exit
    fi
fi

export uploadSymbols=$(awk -F "=" '/bUploadDebugSymbolsAndroid/ {print $2}' ${configPath}/DefaultEngine.ini)

if [ ! -z "$uploadSymbols" ]; then
    if [ $uploadSymbols != "True" ]; then
        echo "BugSplat postprocessing: Automatic symbols upload is disabled in plugin settings. Terminating..." 
        exit
    fi
fi

echo "BugSplat postprocessing: Obtain user credentials"

export bugSplatDatabase=$(awk -F "=" '/BugSplatDatabase/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatApp=$(awk -F "=" '/BugSplatApp/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatVersion=$(awk -F "=" '/BugSplatVersion/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatClientId=$(awk -F "=" '/BugSplatClientId/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatClientSecret=$(awk -F "=" '/BugSplatClientSecret/ {print $2}' ${configPath}/DefaultEngine.ini)

if [ -z "$bugSplatDatabase" ]; then
    echo "BugSplat postprocessing: bugSplatDatabase variable is empty"
fi

if [ -z "$bugSplatApp" ]; then
    echo "BugSplat postprocessing: bugSplatApp variable is empty"
fi

if [ -z "$bugSplatVersion" ]; then
    echo "BugSplat postprocessing: bugSplatVersion variable is empty"
fi

if [ -z "$bugSplatClientId" ]; then
    echo "BugSplat postprocessing: bugSplatClientId variable is empty"
fi

if [ -z "$bugSplatClientSecret" ]; then
    echo "BugSplat postprocessing: bugSplatClientSecret variable is empty"
fi

export symPathArmv7=${binariesPath}/${targetName}-armv7.so
export symPathArm64=${binariesPath}/${targetName}-arm64.so
export libUePathArmv7=${binariesPath}/${targetName}_Symbols_v1/${targetName}-armv7/libUE4.so
export libUePathArm64=${binariesPath}/${targetName}_Symbols_v1/${targetName}-arm64/libUE4.so

echo "BugSplat postprocessing: Run debug symbols upload script"
$scriptsPath/upload-symbols-android.sh $bugSplatDatabase $bugSplatApp $bugSplatVersion $bugSplatClientId $bugSplatClientSecret $symPathArmv7 $symPathArm64 $libUePathArmv7 $libUePathArm64

echo "BugSplat postprocessing: Completed"