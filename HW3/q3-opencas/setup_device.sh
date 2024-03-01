#!/bin/bash

modprobe brd rd_nr=2 rd_size=1048576

ln -s /dev/ram1 /dev/disk/by-id/ramdisk1
