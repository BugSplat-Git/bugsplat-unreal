#!/bin/bash
#
# (Above line comes out when placing in Xcode scheme)
# Copyright (C) BugSplat. All Rights Reserved.

OPT="value"
opts=":hfu:"

cmd(){ echo `basename $0`; }

usage(){
  echo "\
    `cmd` [OPTION...]
  -h  optional  Print this help message
  -f	required  Path to zip file containing .app and dSYM
  -u  required  Path to symbol uploader executable
" | column -t -s ";"
}

error() {
    echo "`cmd`: invalid option -- '$1'";
    echo "Try '`cmd` -h' for more information.";
    exit 1;
}

if [ $# -eq 0 ]; then
    usage;
    exit 1
fi

while getopts ":u:f:h" opt; do
  case ${opt} in
    f )
      ZIPFILE=$OPTARG
      echo "argument -f called with parameter $OPTARG" >&2
      ;;
    u )
      UPLOADER=$OPTARG
      echo "argument -u called with parameter $OPTARG" >&2
      ;;
    h )
      usage
      exit 1
      ;;
    \? )
      error
      ;;
    : )
      echo "Option: $OPTARG requires an argument" 1>&2
      usage
      exit 1
      ;;
  esac
done
shift $((OPTIND -1))

if [ ! -f "${HOME}/.bugsplat.conf" ]
then
    echo "Missing bugsplat config file: ~/.bugsplat.conf"
    exit
fi

source "${HOME}/.bugsplat.conf"

if [ -z "${BUGSPLAT_DATABASE}" ]
then
    echo "BUGSPLAT_DATABASE must be set in ~/.bugsplat.conf"
    exit
fi

if [ -z "${BUGSPLAT_CLIENT_ID}" ]
then
    echo "BUGSPLAT_CLIENT_ID must be set in ~/.bugsplat.conf"
    exit
fi

if [ -z "${BUGSPLAT_CLIENT_SECRET}" ]
then
    echo "BUGSPLAT_CLIENT_SECRET must be set in ~/.bugsplat.conf"
    exit
fi

# Print the variables
echo "ZIP File: $ZIPFILE"
echo "UPLOADER File: $UPLOADER"

# extract to tmp directory for inspection
UNZIP_DIR=$(zipinfo -1 $ZIPFILE | grep -oE '^[^/]+' | uniq)

pwd

UNZIP_LOCATION="/tmp/Bugsplat"
UNZIP_PATH="${UNZIP_LOCATION}/${UNZIP_DIR}"
echo "UNZIP_PATH: ${UNZIP_PATH}"
mkdir -p ${UNZIP_LOCATION}
unzip -o $ZIPFILE -d ${UNZIP_LOCATION}

APP=$(find $UNZIP_PATH -name *.app -type d -maxdepth 1 -print | head -n1)
echo "App: ${APP}"

# Changed Info.plist path
APP_MARKETING_VERSION=$(/usr/libexec/PlistBuddy -c "Print CFBundleShortVersionString" "${APP}/Info.plist")
APP_BUNDLE_VERSION=$(/usr/libexec/PlistBuddy -c "Print CFBundleVersion" "${APP}/Info.plist")

if [ -z "${APP_MARKETING_VERSION}" ]
then
	echo "CFBundleShortVersionString not found in app Info.plist"
    exit
fi

echo "App marketing version: ${APP_MARKETING_VERSION}"
echo "App bundle version: ${APP_BUNDLE_VERSION}"

APP_VERSION="${APP_MARKETING_VERSION}"

if [ -n "${APP_BUNDLE_VERSION}" ]
then
    APP_VERSION="${APP_VERSION} (${APP_BUNDLE_VERSION})"
fi

# Changed CFBundleName to CFBundleExecutable and Info.plist path
PRODUCT_NAME=$(/usr/libexec/PlistBuddy -c "Print CFBundleExecutable" "${APP}/Info.plist")

echo "App version: ${APP_VERSION}"

$UPLOADER -b $BUGSPLAT_DATABASE -a $PRODUCT_NAME -v "$APP_VERSION" -f "**/*.{app,dSYM}" -d $UNZIP_LOCATION  -i $BUGSPLAT_CLIENT_ID -s $BUGSPLAT_CLIENT_SECRET