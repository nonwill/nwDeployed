#!/bin/sh

CURDIR="$PWD"
export LD_LIBRARY_PATH=$CURDIR/lib:$LD_LIBRARY_PATH
export PATH=$CURDIR:$PATH
./GoldenDict &
