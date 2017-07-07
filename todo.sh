#/bin/bash
#Finds all lines with the TODO tag in the source code

grep --color="auto" -R "TODO" ./Source
