#!/bin/bash
# Makes and runs LEngine

rm ./bin/LEngine
colormake --file ./Makefile

#if file exists, clear output and run
if [ -f ./bin/LEngine ]; then
	clear
	./bin/LEngine -debug true
fi
