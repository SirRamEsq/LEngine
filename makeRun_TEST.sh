#!/bin/bash
# Makes and runs LEngine

printf "Removing old LEngine_TEST\r\n"
rm ./bin/LEngine_TEST

printf "Building...\r\n"
colormake --file ./Makefile_TEST

printf "Running CPP Tests...\r\n"
./bin/LEngine_TEST

printf "Running Lua Tests...\r\n"
./runLuaTests.py

