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

if [ $targetPlatform != "IOS" ] && [ $targetPlatform != "Mac" ]; then
    echo "BugSplat [WARN]: Unexpected platform ${targetPlatform}, skipping symbol upload..." 
    exit
fi

if [[ $targetName == *"Editor"* ]]; then
    echo "BugSplat [INFO]: Editor build detected, skipping symbol upload..."
    exit
fi

if [ ! -f "$uploaderPath" ]; then
    echo "BugSplat [INFO]: File $uploaderPath does not exist - downloading..."
    mkdir -p $uploaderFolderPath
    curl -sL "https://app.bugsplat.com/download/symbol-upload-macos" -o $uploaderPath
    chmod +x $uploaderPath
fi

if [ $targetPlatform == "Mac" ] && [ -f "$scriptsPath/upload-symbols-mac.sh" ]; then
    echo "BugSplat [INFO]: Running upload-symbols-mac.sh"
    sh "$scriptsPath/upload-symbols-mac.sh" -f "$binariesPath/$targetName.zip" -u "$uploaderPath"
    exit
elif [ $targetPlatform == "Mac" ]; then
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

echo "BugSplat [INFO]: Copy ${targetName}.app to binaries root directory"
cp -r $binariesPath/Payload/$targetName.app $binariesPath

echo "BugSplat [INFO]: Archive required iOS build artifacts"
pushd $binariesPath
zip -r $targetName.zip $targetName.app $targetName.dSYM
popd

echo "BugSplat [INFO]: Copy user credentials config file template to home directory"
cp $scriptsPath/.bugsplat.conf $HOME/.bugsplat.conf

echo "BugSplat [INFO]: Set actual user credentials"

export bugSplatDatabase=$(awk -F "=" '/BugSplatDatabase/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatClientId=$(awk -F "=" '/BugSplatClientId/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatClientSecret=$(awk -F "=" '/BugSplatClientSecret/ {print $2}' ${configPath}/DefaultEngine.ini)

if [ -z "$bugSplatDatabase" ]; then
    echo "BugSplat [INFO]: bugSplatDatabase variable is empty"
fi

if [ -z "$bugSplatClientId" ]; then
    echo "BugSplat [INFO]: bugSplatClientId variable is empty"
fi

if [ -z "$bugSplatClientSecret" ]; then
    echo "BugSplat [INFO]: bugSplatClientSecret variable is empty"
fi

export bugSplatClientSecretEsc=$(echo "$bugSplatClientSecret" | sed 's/\//\\\//g')

sed -i .backup 's/database/'$bugSplatDatabase'/g' $HOME/.bugsplat.conf
sed -i .backup 's/clientId/'$bugSplatClientId'/g' $HOME/.bugsplat.conf
sed -i .backup 's/clientSecret/'$bugSplatClientSecretEsc'/g' $HOME/.bugsplat.conf

echo "BugSplat [INFO]: Run debug symbols upload script"
$scriptsPath/upload-symbols-ios.sh -f $binariesPath/$targetName.zip -u $uploaderPath

echo "BugSplat [INFO]: Clean up temporaries"

rm $HOME/.bugsplat.conf
rm $HOME/.bugsplat.conf.backup

echo "BugSplat [INFO]: Completed"
