#!/bin/bash

find ./Source -type f \( -iname \*.cpp -o -iname \*.h \) | xargs wc -l
