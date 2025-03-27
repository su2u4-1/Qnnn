from os import system, listdir
from os.path import isfile, isdir, abspath
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
    system(f"g++ {' '.join(files)} -o {output}")
    # print(f"g++ {' '.join(files)} -o {output}")
else:
    print("Usage: python build.py <file1> <file2> ... <output>")
