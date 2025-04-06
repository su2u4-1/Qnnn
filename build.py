from os import system, listdir
from os.path import isfile, isdir, abspath, split
from sys import argv

if len(argv) > 2:
    bat_path = abspath("./build/run.bat")
    files: list[str] = []
    for i in argv[1:-2]:
        if isfile(i) and (i.endswith(".cpp") or i.endswith(".h")):
            files.append(i)
        elif isdir(i):
            for j in listdir(i):
                if j.endswith(".cpp") or j.endswith(".h"):
                    files.append(f"{i}/{j}")
    output = abspath(argv[-2])
    runfile = abspath(argv[-1])
    output_dir = abspath(split(output)[0])
    with open(bat_path, "w") as f:
        if not isdir(output_dir):
            f.write(f"mkdir {output_dir}\n")
        f.write(f"g++ {' '.join(files)} -o {output}\n")
        f.write(f"{output} {runfile}\n")
    system(bat_path)
else:
    print("Usage: python build.py <file1> <file2> ... <output> <arg>")
