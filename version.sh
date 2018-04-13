#!/bin/bash

echo "Version number? (format is X.Y)"

read NEW_VERSION

if [ -z $NEW_VERSION ]
then
  echo No version specified!
  exit
fi

MAJOR=$(echo $NEW_VERSION | awk -F"." '{print $1}')
MINOR=$(echo $NEW_VERSION | awk -F"." '{print $2}')
echo Major release number: $MAJOR
echo Minor release number: $MINOR
if [ -z $MAJOR ] || [ -z $MINOR ]
then
  echo Invalid release number!
  exit
fi

sed -i "s/FILEVERSION[ ]*[0-9]*,[0-9]*,[0-9]*,[0-9]*/FILEVERSION   $MAJOR,$MINOR,0,0/" */*.rc
sed -i "s/PRODUCTVERSION[ ]*[0-9]*,[0-9]*,[0-9]*,[0-9]*/PRODUCTVERSION  $MAJOR,$MINOR,0,0/" */*.rc
sed -i "s/[ ]*VALUE[ ]*\"FileVersion\",[ ]*\"[0-9]*.[0-9]*\"/    VALUE \"FileVersion\", \"$MAJOR.$MINOR\"/" */*.rc
sed -i "s/[ ]*VALUE[ ]*\"ProductVersion\",[ ]*\"[0-9]*.[0-9]*\"/    VALUE \"ProductVersion\", \"$MAJOR.$MINOR\"/" */*.rc

sed -i "s/#define[ ]*INFO_VERSION[ ]*\"[0-9]*.[0-9]*\"/#define INFO_VERSION \"$MAJOR.$MINOR\"/" info.h
sed -i "s/#define[ ]*INFO_YEAR[ ]*\"2010-[0-9]*\"/#define INFO_YEAR \"2010-$(date '+%Y')\"/" info.h

