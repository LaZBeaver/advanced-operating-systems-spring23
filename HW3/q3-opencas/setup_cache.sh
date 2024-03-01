#!/bin/bash

casadm --start-cache --force --cache-device /dev/disk/by-id/ata-ADATA_SSD_S511_120GB_12022629000000001701-part1 --cache-mode wb

casadm --add-core --cache-id 1 --core-device /dev/disk/by-id/ata-WDC_WD10EARS-00Y5B1_WD-WCAV5F076925-part3

casadm --start-cache --force --cache-device /dev/disk/by-id/ramdisk1 --cache-mode wt

casadm --add-core --cache-id 2 --core-device /dev/cas1-1
