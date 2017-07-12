#!/bin/bash
# Makes and runs LEngine

rm ./bin/LEngine
colormake --file ./Makefile
./bin/LEngine -map MAP1.tmx
