#!/usr/bin/env python
from os import listdir, getcwd, chdir
from os.path import isfile, join
import subprocess

cwd = getcwd()
separator = "======================================="
separator += separator + "="


def main():
    chdir(cwd + "/Data/Resources/Scripts/System/Testing")
    newDir = getcwd()
    files = [f for f in listdir(newDir) if isfile(join(newDir, f))]
    print(separator)
    for f in files:
        if not f.startswith("luaTest"):
            continue

        # Right align filename
        whitespace = ""
        for x in range(0, len(separator) - len(f) - 6):
            whitespace += " "
        print(whitespace + "|| " + f + " ||")

        bustedCall = 'busted' 
        argument = "./" + f
        subprocess.call([bustedCall, argument])
        print(separator)


if __name__ == "__main__":
    main()
