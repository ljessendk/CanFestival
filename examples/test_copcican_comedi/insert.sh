#!/bin/sh

module="test_copcican_comedi"
device="test_copcican_comedi"
mode="664"

# insert neccessary modules
/sbin/insmod ../../drivers/can_copcican_comedi/can_copcican_comedi.ko
/sbin/insmod ../../src/canfestival.ko

# insert module with all arguments we got
# and use a pathname, as newer modutils don't look in . by default
/sbin/insmod ./$module.ko $* || exit 1

# stale nodes and device files are created by COMEDI

rmmod $module
rmmod canfestival
rmmod can_copcican_comedi
