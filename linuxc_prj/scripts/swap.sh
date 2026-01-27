#!/bin/sh

#swap.sh /path/swapfile 1G

SWAPFILE=$1
SWAPSIZE=$2

enable_swap() {
    if [ -e $SWAPFILE ]; then
        rm -f $SWAPFILE
    fi

    fallocate -l $SWAPSIZE $SWAPFILE
	
    chmod 600 $SWAPFILE
    mkswap $SWAPFILE
    swapon $SWAPFILE

    sed -i '/^vm.swappiness=.*/d' /etc/sysctl.conf
    echo "vm.swappiness=10" >> /etc/sysctl.conf
    sysctl -p
	
    sed -i '/^\/swapfile.*/d' /etc/fstab
    echo "/swapfile   none    swap    sw    0   0" >> /etc/fstab
}

HASSWAP=`swapon -s`

if [ "x$HASSWAP" = "x" ]; then
    enable_swap
    exit 0
fi
