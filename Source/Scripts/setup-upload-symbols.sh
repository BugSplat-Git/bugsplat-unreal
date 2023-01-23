#!/bin/bash
# Copyright (C) BugSplat. All Rights Reserved.

export targetPlatform=$1
export targetName=$2
export projectPath=$3
export pluginPath=$4

export binariesPath=$projectPath/Binaries/IOS
export configPath=$projectPath/Config
export scriptsPath=$pluginPath/Source/Scripts

echo "BugSplat postprocessing: Start debug symbols upload for iOS"

if [ $targetPlatform != "IOS" ]; then
    echo "BugSplat postprocessing: Unexpected platform ${targetPlatform}. Terminating..." 
    exit
fi

echo "BugSplat postprocessing: Input target name: $targetName" 
echo "BugSplat postprocessing: Input binaries path: $binariesPath"
echo "BugSplat postprocessing: Input config path: $configPath"
echo "BugSplat postprocessing: Input scripts path: $scriptsPath"

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

export bugSplatUser=$(awk -F "=" '/BugSplatUser/ {print $2}' ${configPath}/DefaultEngine.ini)
export bugSplatPassword=$(awk -F "=" '/BugSplatPassword/ {print $2}' ${configPath}/DefaultEngine.ini)

if [ -z "$bugSplatUser" ]; then
    echo "BugSplat postprocessing: bugSplatUser variable is empty"
fi

if [ -z "$bugSplatPassword" ]; then
    echo "BugSplat postprocessing: bugSplatPassword variable is empty"
fi

sed -i .backup 's/username/'$bugSplatUser'/g' $HOME/.bugsplat.conf
sed -i .backup 's/password/'$bugSplatPassword'/g' $HOME/.bugsplat.conf

echo "BugSplat postprocessing: Run debug symbols upload script"
$scriptsPath/upload-symbols.sh -f $binariesPath/$targetName.zip

echo "BugSplat postprocessing: Clean up temporaries"

rm -r $binariesPath/$targetName.app
rm -r $binariesPath/$targetName.zip
rm $HOME/.bugsplat.conf
rm $HOME/.bugsplat.conf.backup

echo "BugSplat postprocessing: Completed"