#!/bin/bash
# Makes and runs LEngine

rm ./bin/LEngine_DEBUG
colormake --file ./MakefileDebug

#if file exists, clear output and run
if [ -f ./bin/LEngine_DEBUG ]; then
	clear
	./bin/LEngine_DEBUG -debug true
fi
