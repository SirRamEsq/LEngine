#!/bin/bash
# Makes and runs LEngine

colormake --file ./Makefile
./bin/LEngine -map MAP1.tmx
