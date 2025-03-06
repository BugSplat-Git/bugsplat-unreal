#!/bin/bash
# Copyright (C) BugSplat. All Rights Reserved.

export targetPlatform=$1
export targetName=$2
export projectPath=$3
export pluginPath=$4

export binariesPath=$projectPath/Binaries/IOS
export configPath=$projectPath/Config
export scriptsPath=$pluginPath/Source/Scripts

export uploaderFolderPath=$pluginPath/Source/ThirdParty/SymUploader
export uploaderPath=$uploaderFolderPath/symbol-upload-macos

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

if [ "$targetPlatform" != "IOS" ] && [ "$targetPlatform" != "Mac" ]; then
    echo "BugSplat [WARN]: Unexpected platform ${targetPlatform}, skipping symbol upload..." 
    exit
fi

if [ "$targetPlatform" == "Mac" ] && [ -f "$scriptsPath/upload-symbols-mac.sh" ]; then
    echo "BugSplat [INFO]: Running upload-symbols-mac.sh"
    sh "$scriptsPath/upload-symbols-mac.sh" "$targetPlatform" "$targetName"
    exit
elif [ "$targetPlatform" == "Mac" ]; then
    echo "BugSplat [WARN]: Symbol uploads not configured via plugin - skipping..."
    exit
fi

echo "BugSplat [INFO]: Start debug symbols upload for iOS"

export reportCrashes=$(awk -F "=" '/bEnableCrashReportingIos/ {print $2}' ${configPath}/DefaultEngine.ini)

# By default crash reporting for iOS is enabled and hence not included in DefaultEngine.ini
if [ ! -z "$reportCrashes" ]; then
    if [ $reportCrashes != "True" ]; then
        echo "BugSplat [INFO]: Crash reporting is disabled in plugin settings. Terminating..." 
        exit
    fi
fi

export uploadSymbols=$(awk -F "=" '/bUploadDebugSymbols/ {print $2}' ${configPath}/DefaultEngine.ini)

if [ ! -z "$uploadSymbols" ]; then
    if [ $uploadSymbols != "True" ]; then
        echo "BugSplat [INFO]: Automatic symbols upload is disabled in plugin settings. Terminating..." 
        exit
    fi
fi

# Get app info directly from the app bundle
export appPath="$binariesPath/$targetName.app"
export dsymFileSpec="$targetName.app.dSYM"

echo "BugSplat [INFO]: Checking app path: $appPath"

# Check if Info.plist exists
if [ ! -f "${appPath}/Info.plist" ]; then
    echo "BugSplat [WARN]: Info.plist not found at ${appPath}/Info.plist"
    
    # Use fallback values
    export appMarketingVersion="1.0"
    export appBundleVersion=""
    export appExecutable="$targetName"
    
    echo "BugSplat [INFO]: Using fallback values - Version: $appMarketingVersion, Executable: $appExecutable"
else
    # Read version information directly from Info.plist
    export appMarketingVersion=$(/usr/libexec/PlistBuddy -c "Print CFBundleShortVersionString" "${appPath}/Info.plist" 2>/dev/null)
    export appBundleVersion=$(/usr/libexec/PlistBuddy -c "Print CFBundleVersion" "${appPath}/Info.plist" 2>/dev/null)
    export appExecutable=$(/usr/libexec/PlistBuddy -c "Print CFBundleExecutable" "${appPath}/Info.plist" 2>/dev/null)
    
    # Check if values were successfully retrieved
    if [ -z "$appMarketingVersion" ]; then
        echo "BugSplat [WARN]: CFBundleShortVersionString not found, using default"
        appMarketingVersion="1.0"
    fi
    
    if [ -z "$appExecutable" ]; then
        echo "BugSplat [WARN]: CFBundleExecutable not found, using target name"
        appExecutable="$targetName"
    fi
fi

# Format version string
export appVersion="${appMarketingVersion}"
if [ -n "${appBundleVersion}" ]; then
    appVersion="${appVersion} (${appBundleVersion})"
fi

# Get BugSplat credentials from config
export bugSplatDatabase=$(awk -F "=" '/BugSplatDatabase/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatClientId=$(awk -F "=" '/BugSplatClientId/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatClientSecret=$(awk -F "=" '/BugSplatClientSecret/ {print $2}' ${configPath}/DefaultEngine.ini)

# Validate credentials
if [ -z "$bugSplatDatabase" ] || [ -z "$bugSplatClientId" ] || [ -z "$bugSplatClientSecret" ]; then
    echo "BugSplat [ERROR]: Missing BugSplat credentials in DefaultEngine.ini"
    exit 1
fi

echo "BugSplat [INFO]: Uploading symbols directly..."

# Directly invoke the uploader with the required parameters
$uploaderPath -b $bugSplatDatabase -a $appExecutable -v "$appVersion" -d "$binariesPath" -f "$dsymFileSpec" -i $bugSplatClientId -s "$bugSplatClientSecret"

echo "BugSplat [INFO]: Symbol upload completed"
