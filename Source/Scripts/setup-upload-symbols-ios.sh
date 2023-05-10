#!/bin/bash
# Copyright (C) BugSplat. All Rights Reserved.

export targetPlatform=$1
export targetName=$2
export projectPath=$3
export pluginPath=$4

export binariesPath=$projectPath/Binaries/IOS
export configPath=$projectPath/Config
export scriptsPath=$pluginPath/Source/Scripts

export uploaderPath=$pluginPath/Source/ThirdParty/SymUploader/symbol-upload-macos

echo "BugSplat postprocessing: Start debug symbols upload for iOS"

if [ $targetPlatform != "IOS" ]; then
    echo "BugSplat postprocessing: Unexpected platform ${targetPlatform}. Terminating..." 
    exit
fi

echo "BugSplat postprocessing: Input target name: $targetName" 
echo "BugSplat postprocessing: Input binaries path: $binariesPath"
echo "BugSplat postprocessing: Input config path: $configPath"
echo "BugSplat postprocessing: Input scripts path: $scriptsPath"
echo "BugSplat postprocessing: Symbol uploader path: $uploaderPath"

export reportCrashes=$(awk -F "=" '/bEnableCrashReportingIos/ {print $2}' ${configPath}/DefaultEngine.ini)

# By default crash reporting for iOS is enabled and hence not included in DefaultEngine.ini
if [ ! -z "$reportCrashes" ]; then
    if [ $reportCrashes != "True" ]; then
        echo "BugSplat postprocessing: Crash reporting is disabled in plugin settings. Terminating..." 
        exit
    fi
fi

export uploadSymbols=$(awk -F "=" '/bUploadDebugSymbolsIos/ {print $2}' ${configPath}/DefaultEngine.ini)

if [ ! -z "$uploadSymbols" ]; then
    if [ $uploadSymbols != "True" ]; then
        echo "BugSplat postprocessing: Automatic symbols upload is disabled in plugin settings. Terminating..." 
        exit
    fi
fi

echo "BugSplat postprocessing: Copy ${targetName}.app to binaries root directory"
cp -r $binariesPath/Payload/$targetName.app $binariesPath

echo "BugSplat postprocessing: Archive required iOS build artifacts"
pushd $binariesPath
zip -r $targetName.zip $targetName.app $targetName.dSYM
popd

echo "BugSplat postprocessing: Copy user credentials config file template to home directory"
cp $scriptsPath/.bugsplat.conf $HOME/.bugsplat.conf

echo "BugSplat postprocessing: Set actual user credentials"

export bugSplatDatabase=$(awk -F "=" '/BugSplatDatabase/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatClientId=$(awk -F "=" '/BugSplatClientId/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatClientSecret=$(awk -F "=" '/BugSplatClientSecret/ {print $2}' ${configPath}/DefaultEngine.ini)

if [ -z "$bugSplatDatabase" ]; then
    echo "BugSplat postprocessing: bugSplatDatabase variable is empty"
fi

if [ -z "$bugSplatClientId" ]; then
    echo "BugSplat postprocessing: bugSplatClientId variable is empty"
fi

if [ -z "$bugSplatClientSecret" ]; then
    echo "BugSplat postprocessing: bugSplatClientSecret variable is empty"
fi

export bugSplatClientSecretEsc=$(echo "$bugSplatClientSecret" | sed 's/\//\\\//g')

sed -i .backup 's/database/'$bugSplatDatabase'/g' $HOME/.bugsplat.conf
sed -i .backup 's/clientId/'$bugSplatClientId'/g' $HOME/.bugsplat.conf
sed -i .backup 's/clientSecret/'$bugSplatClientSecretEsc'/g' $HOME/.bugsplat.conf

echo "BugSplat postprocessing: Run debug symbols upload script"
$scriptsPath/upload-symbols-ios.sh -f $binariesPath/$targetName.zip -u $uploaderPath

echo "BugSplat postprocessing: Clean up temporaries"

rm $HOME/.bugsplat.conf
rm $HOME/.bugsplat.conf.backup

echo "BugSplat postprocessing: Completed"