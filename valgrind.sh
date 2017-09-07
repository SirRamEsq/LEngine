#!/bin/bash

#Run valgrind, send standard output and errors into file
valgrind --tool=memcheck --leak-check=full -v --track-origins=yes ./bin/LEngine > VALGRINDOutput.txt 2>&1
#valgrind --leak-check=summary -v --track-origins=yes ./bin/LEngine > VALGRINDOutput.txt 2>&1

