from os import system
from os.path import abspath
from sys import argv
from typing import Any
import json

if len(argv) < 2:
    print("Usage: python format.py <qn_filename>")
    exit(1)
filename = abspath(argv[1])
if filename.endswith(".json"):
    with open(filename, "r", encoding="utf-8") as f:
        data = json.load(f)
elif filename.endswith(".qn"):
    system(f"python ./build.py ./src -o ./build/app.exe -a {filename} -oaj")
    with open(filename.split(".", 1)[0] + "_ast.json", "r", encoding="utf-8") as f:
        data = json.load(f)
else:
    print("File type not supported")
    exit(1)


def _expression(now: Any) -> str:
    pass


def _type(now: Any) -> str:
    t = now["value"]["name"]
    if len(now["children"]) > 0:
        t += "<"
        t += ", ".join(_type(i) for i in now["children"])
        t += ">"
    return t


def _declare_var(now: Any) -> str:
    t = now["value"]["type"]
    if now["value"]["modifier"] == "public":
        t += " public"
    t += _type(now["children"][0])
    t += " " + now["value"]["name"]
    if len(now["children"]) == 2:
        t += " = " + _expression(now["children"][1])
    return t + ";"


def _import(now: Any) -> str:
    pass


def _program(now: Any) -> str:
    result: list[str] = []
    for i in now["children"]:
        if i["type"] == "declare_var":
            result.append(_declare_var(i))
        elif i["type"] == "import":
            result.append(_import(i))
    return "\n".join(result)


result = _program(data)

with open(data["value"]["name"], "w") as f:
    f.write(result)
