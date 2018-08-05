#!/bin/sh
dmesg -c > before.log
insmod fake_pata_driver.ko
insmod fake_pata.ko
sleep 5
dmesg
rmmod fake_pata
rmmod fake_pata_driver
