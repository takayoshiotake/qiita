#!/bin/bash

echo Start Button Trigger

echo 6 > /sys/class/gpio/export 2> /dev/null
echo 5 > /sys/class/gpio/export 2> /dev/null
sleep 0.1
echo in > /sys/class/gpio/gpio6/direction
echo out > /sys/class/gpio/gpio5/direction

gpio_in=/sys/class/gpio/gpio6/value
led_out=/sys/class/gpio/gpio5/value

while :
do
    echo 1 > $led_out
    
    while [ `cat $gpio_in` = "0" ]
    do
        sleep 0.01
    done
    while [ `cat $gpio_in` = "1" ]
    do
        sleep 0.01
    done
    
    echo Falling Edge Detected
    echo 0 > $led_out
    sleep 0.5
    echo 1 > $led_out
    sleep 0.1
    echo 0 > $led_out
    sleep 0.1
    echo 1 > $led_out
    sleep 0.1
    echo 0 > $led_out
    sleep 0.1
    echo 1 > $led_out
    sleep 0.1
    echo 0 > $led_out
done
