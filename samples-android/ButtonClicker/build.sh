#!/bin/sh

#setup build mode
mode=$1
if [ -z $1 ] ; then
  echo “Default build mode: debug”
  mode=debug
fi

#build library project
echo “Switching to google-play-services_lib”

pushd ../../google-play-services_lib/
android update lib-project --path .
ant $mode
popd

#Create ant
echo “Updating ANT build settings”
android update project --path .

#Execute ndk-build && ant
ndk-build && ant $mode

