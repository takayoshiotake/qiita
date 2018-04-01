#!/bin/bash

MJPEG_STREAMER_PATH=/home/pi/mjpg-streamer-master/mjpg-streamer-experimental

export LD_LIBRARY_PATH=$MJPEG_STREAMER_PATH
$MJPEG_STREAMER_PATH/mjpg_streamer -o "output_http.so -p 8081" -i "input_raspicam.so -x 640 -y 480 -quality 20"
