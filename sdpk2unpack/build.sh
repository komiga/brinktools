#!/bin/bash

ACTION="$1"
if [ "$ACTION" == "" ]; then 
	ACTION=debug
fi

function clean_output() {
	make config=debug clean
	make config=release clean
	rm -rf "out"
}

if [ "$ACTION" == "clean" ]; then
	clean_output
	premake4 clean
	exit
fi

if [ ! -f "Makefile" ]; then
	premake4 gmake
fi

make config=$ACTION

#clean_output
