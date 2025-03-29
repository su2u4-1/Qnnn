from os import system, listdir
from os.path import isfile, isdir, abspath, split
from sys import argv

if len(argv) > 2:
    files: list[str] = []
    for i in argv[1:-1]:
        if isfile(i) and (i.endswith(".cpp") or i.endswith(".h")):
            files.append(i)
        elif isdir(i):
            for j in listdir(i):
                if j.endswith(".cpp") or j.endswith(".h"):
                    files.append(f"{i}/{j}")
    output = abspath(argv[-1])
    output_dir = split(output)[0]
    if not isdir(output_dir):
        system(f"echo mkdir {output_dir}")
        system(f"mkdir {output_dir}")
    system(f"echo g++ {' '.join(files)} -o {output}")
    system(f"g++ {' '.join(files)} -o {output}")
else:
    print("Usage: python build.py <file1> <file2> ... <output>")
