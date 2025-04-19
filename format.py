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


def _term(now: Any) -> str:
    t = "term"
    return t


def _expression(now: Any) -> str:
    t: list[str] = []
    for i in now["children"]:
        if i["type"] == "operator":
            t.append("op_" + i["value"]["value"])
        else:
            t.append(_term(i))
    stack: list[str] = []
    for token in t:
        if token.startswith("op_"):
            a = stack.pop()
            stack.append(f"{stack.pop()} {token[3:]} {a}")
        else:
            stack.append(token)
    return " ".join(stack)


def _type(now: Any) -> str:
    t = now["value"]["name"]
    if len(now["children"]) > 0:
        t += "<"
        t += ", ".join(_type(i) for i in now["children"])
        t += ">"
    return t


def _declare_var(now: Any) -> str:
    t = now["value"]["kind"] + " "
    if now["value"]["modifier"] == "public":
        t += "public "
    t += _type(now["children"][0])
    t += " " + now["value"]["name"]
    if len(now["children"]) == 2:
        t += " = " + _expression(now["children"][1])
    return t + ";"


def _pass(now: Any) -> str:
    return "pass"


def _if(now: Any) -> list[str]:
    t: list[str] = []
    t.append("if (" + _expression(now["children"][0]) + ") {")
    for i in _statements(now["children"][1]):
        t.append("    " + i)
    for i in range(int(now["value"]["elif_n"])):
        t.append("} elif (" + _expression(now["children"][i * 2 + 2]) + ") {")
        for j in _statements(now["children"][i * 2 + 3]):
            t.append("    " + j)
    if now["value"]["else"] == "true":
        t.append("} else {")
        for i in _statements(now["children"][-1]):
            t.append("    " + i)
    t.append("}")
    return t


def _for(now: Any) -> list[str]:
    t: list[str] = []
    t.append(f"for ({_type(now["children"][0])} {now["value"]["iterator"]} in {_expression(now["children"][1])}) {{")
    for i in _statements(now["children"][2]):
        t.append(f"    {i}")
    t.append("}")
    if now["value"]["else"] == "true":
        t.append("else {")
        for i in _statements(now["children"][3]):
            t.append(f"    {i}")
        t.append("}")
    return t


def _while(now: Any) -> list[str]:
    t: list[str] = []
    return t


def _func(now: Any) -> list[str]:
    t: list[str] = []
    return t


def _class(now: Any) -> list[str]:
    t: list[str] = []
    return t


def _statements(now: Any) -> list[str]:
    t: list[str] = []
    return t


def _import(now: Any) -> str:
    t = "import "
    if now["value"]["alias"] == "stdlib":
        t += now["value"]["name"]
    else:
        t += '"' + now["value"]["name"] + '" as ' + now["value"]["alias"]
    return t


def _program(now: Any) -> str:
    result: list[str] = []
    for i in now["children"]:
        if i["type"] == "declare_var":
            result.append(_declare_var(i))
        elif i["type"] == "import":
            result.append(_import(i))
        elif i["type"] == "if":
            result.extend(_if(i))
        elif i["type"] == "for":
            result.extend(_for(i))
        elif i["type"] == "while":
            result.extend(_while(i))
        elif i["type"] == "func":
            result.extend(_func(i))
        elif i["type"] == "class":
            result.extend(_class(i))
        elif i["type"] == "expression":
            result.append(_expression(i))
        elif i["type"] == "pass":
            result.extend(_pass(i))
    return "\n".join(result)


result = _program(data)
print(result)

# with open(data["value"]["name"], "w") as f:
#     f.write(result)
