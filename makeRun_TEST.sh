#!/bin/bash
# Makes and runs LEngine

rm ./bin/LEngine_TEST
colormake --file ./Makefile_TEST
./bin/LEngine_TEST
