#!/bin/bash

if [ $EUID -ne 0 ]
then
  echo "This script must be run as root" 1>&2
  exit -1
fi

if [ $# -lt 6 ]
then
  echo "Not enough arguments supplied."
  exit -1
fi

DONGLE=$1
DONGLE_ADDRESS=$2

DS4_ADDRESS=$3
DS4_LINK_KEY=$4

PS4_ADDRESS=$5
PS4_LINK_KEY=$6

mkdir -p /var/lib/bluetooth/"$DONGLE_ADDRESS"

touch /var/lib/bluetooth/$DONGLE_ADDRESS/linkkeys
chmod go-r /var/lib/bluetooth/$DONGLE_ADDRESS/linkkeys

#set dongle link key for the DS4
sed "/$DS4_ADDRESS/d" -i /var/lib/bluetooth/$DONGLE_ADDRESS/linkkeys 2> /dev/null
echo $DS4_ADDRESS $DS4_LINK_KEY 4 0 >> /var/lib/bluetooth/$DONGLE_ADDRESS/linkkeys

#set dongle link key for the PS4
sed "/$PS4_ADDRESS/d" -i /var/lib/bluetooth/$DONGLE_ADDRESS/linkkeys 2> /dev/null
echo $PS4_ADDRESS $PS4_LINK_KEY 4 0 >> /var/lib/bluetooth/$DONGLE_ADDRESS/linkkeys

#stop the bluetooth service
service bluetooth stop 2&> /dev/null

#make sure the bluetooth dongle is up
hciconfig $DONGLE up pscan

#send link keys
hciconfig $DONGLE putkey $DS4_ADDRESS
hciconfig $DONGLE putkey $PS4_ADDRESS

#enable authentication and encryption
hciconfig $DONGLE auth encrypt
