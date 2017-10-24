#!/bin/bash
#Runs ctags with correct cpp arguments

#see https://stackoverflow.com/questions/1932396/c-source-tagging
find ./Source/ -iname *.h -o -iname *.cpp | xargs clang-format -i
