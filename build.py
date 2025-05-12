from hashlib import sha256
from os import mkdir, system, listdir
from os.path import isfile, isdir, abspath, split
from sys import argv, platform


def parse_args(arg: list[str]) -> tuple[list[str], str, list[str], list[str]]:
    if len(arg) < 3:
        print("Usage: python build.py {(<file> | <dir>)} [-i <dir>] [-o <output>] [-a {<arg>}]")
        exit(1)
    args: list[str] = []
    files: list[str] = []
    flags: list[str] = []
    state = -1
    output_file = "./temp/output.exe"
    include_path = ""
    for i in arg:
        if state == 0:
            output_file = i
            state = -1
        elif state == 1:
            args.append(i)
        elif state == 2:
            include_path = i
            state = -1
        elif i == "-o":
            state = 0
        elif i == "-a":
            state = 1
        elif i == "-i":
            state = 2
        elif i.startswith("-"):
            flags.append(i)
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
    if include_path != "":
        for i in range(len(files)):
            files[i] = abspath(files[i])
        files.append(f"-I {include_path}")
    return files, output_file, args, flags


def main(files: list[str], output_file: str, args: list[str], flags: list[str], p_hash: str = "") -> str:
    compiler_path = "g++"
    output_dir = split(output_file)[0]
    bat_path = output_dir + "/run.bat"
    sh_path = output_dir + "/run.sh"
    source_code = ""
    for i in files:
        with open(i, "r") as f:
            source_code += f.read()
    a_hash = sha256(source_code.encode()).hexdigest()

    if not isdir(output_dir):
        mkdir(output_dir)
    if platform == "win32" or platform == "cygwin":
        with open(bat_path, "w") as f:
            if a_hash != p_hash:
                f.write(f"{compiler_path} {' '.join(flags)} {' '.join(files)} -o {output_file}\n")
            t = "\\"
            f.write(f"{abspath(output_file).replace('/', t)} {' '.join(args)}\n")
        system(abspath(bat_path))
    else:
        with open(sh_path, "w") as f:
            if a_hash != p_hash:
                f.write(f"{compiler_path} {' '.join(flags)} {' '.join(files)} -o {output_file}\n")
            f.write(f"{output_file} {' '.join(args)}\n")
        system("bash " + sh_path)

    return a_hash


p_hash = ""
if isfile("./temp/hash"):
    with open("./temp/hash", "r") as f:
        p_hash = f.read()

if len(argv) > 1:
    a_hash = main(*parse_args(argv[1:]), p_hash)
else:
    print("Usage: {(<file> | <dir>)} [-i <dir>] [-o <output>] [-a {<arg>}]")
    a_hash = main(*parse_args(input(": ").split()), p_hash)

if not isdir("./temp"):
    mkdir("./temp")
with open("./temp/hash", "w") as f:
    f.write(str(a_hash))
