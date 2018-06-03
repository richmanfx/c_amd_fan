#!/bin/bash

if [[ ! -v PKG_CONFIG_PATH ]]; then
    echo "PKG_CONFIG_PATH is not set"
    echo "Set PKG_CONFIG_PATH"
    export PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig/
elif [[ -z "$PKG_CONFIG_PATH" ]]; then
    echo "PKG_CONFIG_PATH is set to the empty string"
else
    echo "PKG_CONFIG_PATH has the value: $PKG_CONFIG_PATH"
fi



/usr/bin/gcc `pkg-config --cflags libconfig` c_amd_fan.c -o c_amd_fan `pkg-config --libs libconfig`