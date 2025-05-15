from hashlib import sha256
from os import mkdir, system, listdir
from os.path import isfile, isdir, abspath
from sys import argv, platform


def parse_args(arg: list[str]) -> tuple[list[str], list[str], list[str]]:
    if len(arg) < 3:
        print("Usage: python build.py {(<file> | <dir>)} [-i <dir>] [-o <output>] [-a {<arg>}]")
        exit(1)
    args: list[str] = []
    files: list[str] = []
    flags: list[str] = []
    state = -1
    include_path = ""
    for i in arg:
        if state == 1:
            args.append(i)
        elif state == 2:
            include_path = i
            state = -1
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
    return files, args, flags


def main(files: list[str], args: list[str], flags: list[str]) -> None:
    compiler_path = "g++"
    output_dir = "./temp/hash/"
    output_file = "/app.exe"
    bat_path = "./temp/run.bat"
    sh_path = "./temp/run.sh"
    source_code = ""

    for i in files:
        with open(i, "r") as f:
            source_code += f.read()
    hash = sha256(source_code.encode()).hexdigest()

    exe_exists = False
    output_dir += "/" + hash
    if isdir(output_dir):
        exe_exists = True
    else:
        mkdir(output_dir)

    s_path = bat_path if platform == "win32" or platform == "cygwin" else sh_path
    with open(s_path, "w") as f:
        if not exe_exists:
            f.write(f"{compiler_path} {' '.join(flags)} {' '.join(files)} -o {output_dir + output_file}\n")
        f.write(f"{abspath(output_dir + output_file).replace("\\", "/")} {' '.join(args)}\n")

    command = abspath(bat_path) if platform == "win32" or platform == "cygwin" else "bash " + sh_path
    system(command)


if not isdir("./temp/hash"):
    mkdir("./temp/hash")

if len(argv) > 1:
    main(*parse_args(argv[1:]))
else:
    print("Usage: {(<file> | <dir>)} [-i <dir>] [-o <output>] [-a {<arg>}]")
    main(*parse_args(input(": ").split()))
