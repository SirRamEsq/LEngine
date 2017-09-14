#!/bin/bash

find ./Source ./Data/Resources/Scripts -path ./Source/Engine/gui -prune -o -type f \( -iname \*.lua -o -iname \*.cpp -o -iname \*.h \) | xargs cat | wc -l
