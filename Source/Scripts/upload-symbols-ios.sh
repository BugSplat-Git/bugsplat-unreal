#!/bin/bash
#
# (Above line comes out when placing in Xcode scheme)
# Copyright (C) BugSplat. All Rights Reserved.

OPT="value"
opts=":hd:a:v:i:s:p:"

cmd(){ echo `basename $0`; }

usage(){
  echo "\
    `cmd` [OPTION...]
  -h  optional  Print this help message
  -d  required  BugSplat database name
  -a  required  Application name
  -v  required  Application version
  -i  required  BugSplat client ID
  -s  required  BugSplat client secret
  -p  required  Path to dSYM file or directory
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

while getopts ":d:a:v:i:s:p:u:h" opt; do
  case ${opt} in
    d )
      BUGSPLAT_DATABASE=$OPTARG
      ;;
    a )
      PRODUCT_NAME=$OPTARG
      ;;
    v )
      APP_VERSION=$OPTARG
      ;;
    i )
      BUGSPLAT_CLIENT_ID=$OPTARG
      ;;
    s )
      BUGSPLAT_CLIENT_SECRET=$OPTARG
      ;;
    p )
      DSYM_PATH=$OPTARG
      ;;
    u )
      UPLOADER=$OPTARG
      ;;
    h )
      usage
      exit 0
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

# Validate required parameters
if [ -z "$BUGSPLAT_DATABASE" ] || [ -z "$PRODUCT_NAME" ] || [ -z "$APP_VERSION" ] || 
   [ -z "$BUGSPLAT_CLIENT_ID" ] || [ -z "$BUGSPLAT_CLIENT_SECRET" ] || [ -z "$DSYM_PATH" ] || [ -z "$UPLOADER" ]; then
    echo "Missing required parameters"
    usage
    exit 1
fi

echo "Uploading symbols to BugSplat..."
echo "Database: $BUGSPLAT_DATABASE"
echo "Application: $PRODUCT_NAME"
echo "Version: $APP_VERSION"
echo "dSYM Path: $DSYM_PATH"

# Directly invoke the uploader with the provided parameters
$UPLOADER -b $BUGSPLAT_DATABASE -a $PRODUCT_NAME -v "$APP_VERSION" -f "$DSYM_PATH" -i $BUGSPLAT_CLIENT_ID -s "$BUGSPLAT_CLIENT_SECRET"

echo "Symbol upload complete"