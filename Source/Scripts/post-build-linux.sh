#!/bin/bash
# Copyright (C) BugSplat. All Rights Reserved.

export targetPlatform=$1
export targetName=$2
export projectPath=$3
export pluginPath=$4

export binariesPath=$projectPath/Binaries/Linux
export configPath=$projectPath/Config
export scriptsPath=$pluginPath/Source/Scripts

export uploaderFolderPath=$pluginPath/Source/ThirdParty/SymUploader
export uploaderPath=$uploaderFolderPath/symbol-upload-linux

echo "BugSplat [INFO]: Input target platform: $targetPlatform" 
echo "BugSplat [INFO]: Input target name: $targetName" 
echo "BugSplat [INFO]: Input binaries path: $binariesPath"
echo "BugSplat [INFO]: Input config path: $configPath"
echo "BugSplat [INFO]: Input scripts path: $scriptsPath"
echo "BugSplat [INFO]: Symbol uploader path: $uploaderPath"

if [[ "$targetName" == *"Editor"* ]]; then
    echo "BugSplat [INFO]: Editor build detected, skipping symbol upload..."
    exit
fi

if [ "$targetPlatform" != "Linux" ]; then
    echo "BugSplat [WARN]: Unexpected platform ${targetPlatform}, skipping symbol upload..." 
    exit
fi

echo "BugSplat [WARN]: Symbol uploads for Linux are not supported yet"