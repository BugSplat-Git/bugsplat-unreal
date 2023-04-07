#!/bin/bash
#
# (Above line comes out when placing in Xcode scheme)
# Copyright (C) BugSplat. All Rights Reserved.

OPT="value"
opts=":hf:"

cmd(){ echo `basename $0`; }

usage(){
  echo "\
    `cmd` [OPTION...]
  -h    optional  Print this help message
  -f	required  Path to zip file containing .app and dSYM
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

while getopts ${opts} opt; do
  case ${opt} in
    f )
      FILE=$OPTARG
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

if [ -z "${BUGSPLAT_USER}" ]
then
    echo "BUGSPLAT_USER must be set in ~/.bugsplat.conf"
    exit
fi

if [ -z "${BUGSPLAT_PASS}" ]
then
    echo "BUGSPLAT_PASS must be set in ~/.bugsplat.conf"
    exit
fi

# Print the variables
echo "File: ${FILE}"

# extract to tmp directory for inspection
UNZIP_DIR=$(zipinfo -1 $FILE | grep -oE '^[^/]+' | uniq)

pwd

UNZIP_LOCATION="/tmp/Bugsplat"
UNZIP_PATH="${UNZIP_LOCATION}/${UNZIP_DIR}"
echo "UNZIP_PATH: ${UNZIP_PATH}"
mkdir -p ${UNZIP_LOCATION}
unzip -o $FILE -d ${UNZIP_LOCATION}

ls

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

BUGSPLAT_SERVER_URL=$(/usr/libexec/PlistBuddy -c "Print BugsplatServerURL" "${APP}/Info.plist")
BUGSPLAT_SERVER_URL=${BUGSPLAT_SERVER_URL%/}

UPLOAD_URL="${BUGSPLAT_SERVER_URL}/post/plCrashReporter/symbol/"

echo "App version: ${APP_VERSION}"

UUID_CMD_OUT=$(xcrun dwarfdump --uuid "${APP}/${PRODUCT_NAME}")
UUID_CMD_OUT=$([[ "${UUID_CMD_OUT}" =~ ^(UUID: )([0-9a-zA-Z\-]+) ]] && echo ${BASH_REMATCH[2]})
echo "UUID found: ${UUID_CMD_OUT}"

echo "Signing into bugsplat and storing session cookie for use in upload"

COOKIEPATH="/tmp/bugsplat-cookie.txt"
LOGIN_URL="${BUGSPLAT_SERVER_URL}/browse/login.php"
echo "Login URL: ${LOGIN_URL}"
rm "${COOKIEPATH}"
curl -b "${COOKIEPATH}" -c "${COOKIEPATH}" --data-urlencode "currusername=${BUGSPLAT_USER}" --data-urlencode "currpasswd=${BUGSPLAT_PASS}" "${LOGIN_URL}"

echo "Uploading ${FILE} to ${UPLOAD_URL}"

curl -i -b "${COOKIEPATH}" -c "${COOKIEPATH}" -F filedata=@"${FILE}" -F appName="${PRODUCT_NAME}" -F appVer="${APP_VERSION}" -F buildId="${UUID_CMD_OUT}" $UPLOAD_URL
