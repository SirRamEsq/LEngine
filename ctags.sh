#!/bin/bash
#Runs ctags with correct cpp arguments

#see https://stackoverflow.com/questions/1932396/c-source-tagging
ctags -R ./Source --c++-kinds=+p --fields=+ias --extra=+q --language-force=C++ --exclude=doxygen

