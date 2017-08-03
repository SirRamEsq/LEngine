#!/bin/bash

#Run valgrind, send standard output and errors into file
valgrind --leak-check=full -v --track-origins=yes ./bin/LEngine > VALGRINDOutput.txt 2>&1

