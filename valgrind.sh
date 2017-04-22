#!/bin/bash

#Run valgrind, send standard output and errors into file
valgrind --leak-check=full -v --track-origins=yes ./LEngineUbuntuDEBUG_BUILD -map=mmap2.tmx > VALGRINDOutput.txt 2>&1

