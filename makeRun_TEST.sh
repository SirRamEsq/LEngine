#!/bin/bash
# Makes and runs LEngine

RUN=0
if [[ -n "$1" ]]  #  If command-line argument present,
then
	if [[ "$1" == "cpp" ]]
	then
		RUN=1
	elif [[ "$1" == "lua" ]]
	then
		RUN=2		
	fi
fi

if [[ ($RUN == 0)  || ($RUN == 1) ]]
then
	printf "Removing old LEngine_TEST\r\n"
	rm ./bin/LEngine_TEST

	printf "Building...\r\n"
	colormake --file ./Makefile_TEST

	printf "Running CPP Tests...\r\n"
	./bin/LEngine_TEST
fi

if [[ ($RUN == 0) || ($RUN == 2) ]]
then
	printf "Running Lua Tests...\r\n"
	./runLuaTests.py
fi
