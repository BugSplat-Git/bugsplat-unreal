#!/bin/bash
# Copyright (C) BugSplat. All Rights Reserved.

export targetName=$1
export pluginPath=$2

export uploaderFolderPath=$pluginPath/Source/ThirdParty/SymUploader
export uploaderPath=$uploaderFolderPath/symbol-upload-linux

echo "BugSplat [INFO]: Input target name: $targetName" 
echo "BugSplat [INFO]: Symbol uploader path: $uploaderPath"

if echo "$targetName" | grep -q "Editor"; then
    echo "BugSplat [INFO]: Editor build detected, skipping pre-build step..."
    exit
fi

if ! command -v curl > /dev/null 2>&1; then
    echo "BugSplat [ERROR]: 'curl' is not installed. This script requires curl to download the symbol uploader. Please install curl and try again."
    exit 1
fi

if [ ! -f "$uploaderPath" ]; then
    echo "BugSplat [INFO]: File $uploaderPath does not exist - downloading..."
    mkdir -p $uploaderFolderPath
    curl -sL "https://app.bugsplat.com/download/symbol-upload-linux" -o $uploaderPath
    chmod +x $uploaderPath
fi
