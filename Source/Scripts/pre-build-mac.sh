#!/bin/bash
# Copyright (C) BugSplat. All Rights Reserved.

export targetName=$1
export pluginPath=$2

export uploaderFolderPath=$pluginPath/Source/ThirdParty/SymUploader
export uploaderPath=$uploaderFolderPath/symbol-upload-macos

echo "BugSplat [INFO]: Input target name: $targetName" 
echo "BugSplat [INFO]: Symbol uploader path: $uploaderPath"

if [[ "$targetName" == *"Editor"* ]]; then
    echo "BugSplat [INFO]: Editor build detected, skipping pre-build step..."
    exit
fi

if [ ! -f "$uploaderPath" ]; then
    echo "BugSplat [INFO]: File $uploaderPath does not exist - downloading..."
    mkdir -p $uploaderFolderPath
    curl -sL "https://app.bugsplat.com/download/symbol-upload-macos" -o $uploaderPath
    chmod +x $uploaderPath
fi
