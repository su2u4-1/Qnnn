from os import mkdir, system, listdir
from os.path import isfile, isdir, abspath, split
from sys import argv, platform


def parse_args(arg: list[str]) -> tuple[list[str], str, list[str]]:
    if len(arg) < 3:
        print("Usage: python build.py {(<file> | <dir>)} [-i <dir>] [-o <output>] [-a {<arg>}]")
        exit(1)
    args: list[str] = []
    files: list[str] = []
    state = -1
    output_file = "./build/output.exe"
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
    return files, output_file, args


def main(files: list[str], output_file: str, args: list[str]) -> None:
    compiler_path = "g++"
    output_dir = split(output_file)[0]
    bat_path = output_dir + "/run.bat"
    sh_path = output_dir + "/run.sh"

    if not isdir(output_dir):
        mkdir(output_dir)
    if platform == "win32" or platform == "cygwin":
        with open(bat_path, "w") as f:
            f.write(f"{compiler_path} {' '.join(files)} -o {output_file}\n")
            f.write(f"{abspath(output_file).replace('/', '\\')} {' '.join(args)}\n")
        system(abspath(bat_path))
    else:
        with open(sh_path, "w") as f:
            f.write(f"{compiler_path} {' '.join(files)} -o {output_file}\n")
            f.write(f"{output_file} {' '.join(args)}\n")
        system(sh_path)


if len(argv) > 1:
    main(*parse_args(argv[1:]))
else:
    print("Usage: {(<file> | <dir>)} [-i <dir>] [-o <output>] [-a {<arg>}]")
    main(*parse_args(input(": ").split()))
