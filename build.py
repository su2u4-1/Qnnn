from os import mkdir, system, listdir
from os.path import isfile, isdir, abspath, split
from sys import argv, platform

if len(argv) < 3:
    print("Usage: python build.py {(<file> | <dir>)} [-o <output>] [-a {<arg>}]")
    exit(1)

state = -1
args: list[str] = []
files: list[str] = []
output_file = "o.exe"
compiler_path = "g++"

for i in argv[1:]:
    if state == 0:
        output_file = i
        state = -1
    elif state == 1:
        args.append(i)
    elif i == "-o":
        state = 0
    elif i == "-a":
        state = 1
    elif isfile(i):
        files.append(i)
    elif isdir(i):
        for j in listdir(i):
            j = i + "/" + j
            if isfile(j):
                files.append(j)
            else:
                print(f"{j} is not file")
                exit(1)
    else:
        print(f"{i} is not file or directory")
        exit(1)

output_file = output_file
output_dir = split(output_file)[0]
bat_path = output_dir + "/run.bat"
sh_path = output_dir + "/run.sh"

if not isdir(output_dir):
    mkdir(output_dir)
if platform == "win32" or platform == "cygwin":
    with open(bat_path, "w") as f:
        f.write(f"{compiler_path} {' '.join(files)} -o {output_file}\n")
        f.write(f"{abspath(output_file).replace('/', '\\')} {' '.join(args)}\n")
    system(abspath(output_dir + "/run.bat"))
else:
    with open(sh_path, "w") as f:
        f.write(f"{compiler_path} {' '.join(files)} -o {output_file}\n")
        f.write(f"{output_file} {' '.join(args)}\n")
    system(sh_path)
