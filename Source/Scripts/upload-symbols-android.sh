#!/bin/bash
# Copyright (C) BugSplat. All Rights Reserved.

export BUGSPLAT_DATABSE=$1
export BUGSPLAT_APP=$2
export APP_VERSION=$3
export BUGSPLAT_CLIENT_ID=$4
export BUGSPLAT_CLIENT_SECRET=$5

export SYM_PATH_ARMV7=$6
export SYM_PATH_ARM64=$7
export LIBUE_PATH_ARMV7=$8
export LIBUE_PATH_ARM64=$9

LOGIN_RESPONSE=$(curl -b --location https://app.bugsplat.com/oauth2/authorize \
  --form grant_type=client_credentials \
  --form client_id=$BUGSPLAT_CLIENT_ID \
  --form client_secret=$BUGSPLAT_CLIENT_SECRET \
  --form scope=restricted)

echo "Authorization result: $LOGIN_RESPONSE"

ACCESS_TOKEN=$(echo $LOGIN_RESPONSE | grep -o '"access_token":"[^"]*' | grep -o '[^"]*$' )

if [ -z "$ACCESS_TOKEN" ]; then
    echo "Access token variable is empty"
    exit
fi

declare -a symbols=($SYM_PATH_ARMV7 $SYM_PATH_ARM64 $LIBUE_PATH_ARMV7 $LIBUE_PATH_ARM64)

for sym in "${symbols[@]}"; do
    if [ ! -f "$sym" ]; then
        echo "$sym doesn't exist. Skipping upload..."
        continue
    fi

    UPLOAD_RESPONSE=$(curl --location https://${BUGSPLAT_DATABSE}.bugsplat.com/post/android/symbols \
      --header "Authorization: Bearer $ACCESS_TOKEN" \
      --form database=$BUGSPLAT_DATABSE \
      --form appName=$BUGSPLAT_APP \
      --form appVersion=$APP_VERSION \
      --form file=@$sym)

    UPLOAD_RESULT_MSG=$(echo $UPLOAD_RESPONSE | grep -o '"message":"[^"]*' | grep -o '[^"]*$' )

    echo "Upload result: $UPLOAD_RESULT_MSG"
done