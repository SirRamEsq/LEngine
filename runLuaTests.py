#!/usr/bin/env python
from os import listdir, getcwd, chdir
from os.path import isfile, join
import subprocess

cwd = getcwd()
separator = "=========================================="
separator += separator


def main():
    chdir(cwd + "/Data/Resources/Scripts/Testing")
    newDir = getcwd()
    files = [f for f in listdir(newDir) if isfile(join(newDir, f))]
    print(separator)
    for f in files:
        if not f.startswith("luaTest"):
            continue
        bustedCall = 'busted' 
        argument = "./" + f
        subprocess.call([bustedCall, argument])
        print(separator)


if __name__ == "__main__":
    main()
