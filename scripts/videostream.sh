#!/bin/bash

file="$1"
host="$2"
port="$3"

gst-launch -v filesrc location=\"$1\" ! decodebin name=b \
	b.src0 ! ffmpegcolorspace ! videorate ! videoscale method=1 ! 'video/x-raw-rgb,bpp=(int)24,framerate=(fraction)24/1, width=(int)24, height=(int)24' ! tee name=t \
		t. ! queue ! udpsink host=$host port=$port \
                t. ! queue ! videoscale method=0 ! 'video/x-raw-rgb,  width=(int)240, height=(int)240' ! ffmpegcolorspace ! xvimagesink \
        b.src1 ! audioconvert ! alsasink

