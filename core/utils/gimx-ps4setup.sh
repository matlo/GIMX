#!/bin/bash

DONGLE_ADDRESS=$1

DS4_ADDRESS=$2
DS4_LINK_KEY=$3

PS4_ADDRESS=$4
PS4_LINK_KEY=$5

LK_DIR=~/.gimx/bluetooth/"$DONGLE_ADDRESS"

mkdir -p $LK_DIR

#set dongle link key for the DS4
echo $DS4_ADDRESS $DS4_LINK_KEY 4 0 > $LK_DIR/linkkeys

#set dongle link key for the PS4
echo $PS4_ADDRESS $PS4_LINK_KEY 4 0 >> $LK_DIR/linkkeys
