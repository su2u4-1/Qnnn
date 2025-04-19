from os import system
from os.path import abspath
from sys import argv
import json
from typing import Any


class Node:
    def __init__(self, type: str, value: dict[str, str], children: list["Node"]):
        self.type = type
        self.value = value
        self.children = children

    def __repr__(self) -> str:
        return f"Node({self.type}, {self.value}, {self.children})"


def json2node(data: Any) -> Node:
    return Node(data["type"], data["value"], [json2node(i) for i in data["children"]])


if len(argv) < 2:
    print('Usage: python ./format.py <qn_filename> "[ident]"')
    exit(1)
if len(argv) >= 3:
    ident = argv[2]
else:
    ident = "    "
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

data = json2node(data)


def _term(now: Node) -> str:
    t = "term"
    return t


def _expression(now: Node) -> str:
    t: list[str] = []
    for i in now.children:
        if i.type == "operator":
            t.append("op_" + i.value["value"])
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


def _type(now: Node) -> str:
    t = now.value["name"]
    if len(now.children) > 0:
        t += f"<{", ".join(_type(i) for i in now.children)}>"
    return t


def _declare_var(now: Node) -> str:
    t = f"{now.value["kind"]} {"public " if now.value["modifier"] == "public" else ""}{_type(now.children[0])} {now.value["name"]}"
    if len(now.children) == 2:
        t += f" = {_expression(now.children[1])}"
    return t + ";"


def _declare_attr(now: Node) -> str:
    t = f"{now.value["kind"]} {"static " if now.value["modifier"] == "static" else ""}{_type(now.children[0])} {now.value["name"]}"
    if len(now.children) == 2:
        t += f" = {_expression(now.children[1])}"
    return t + ";"


def _declare_args(now: Node) -> str:
    t: list[str] = []
    for i in now.children:
        t.append(f"{_type(i.children[0])} {"*" if i.value["tuple"] == "true" else ""}{i.value["name"]}")
    return ", ".join(t)


def _pass(now: Node) -> str:
    return "pass;"


def _break(now: Node) -> str:
    t = "break"
    if now.value["label"] != "break":
        t += " " + now.value["label"]
    return t + ";"


def _return(now: Node) -> str:
    t = "return"
    if len(now.children) == 1:
        t += " " + _expression(now.children[0])
    return t + ";"


def _continue(now: Node) -> str:
    return "continue;"


def _if(now: Node) -> list[str]:
    t = ["if (" + _expression(now.children[0]) + ") {"]
    for i in _statements(now.children[1]):
        t.append(ident + i)
    for i in range(int(now.value["elif_n"])):
        t.append("} elif (" + _expression(now.children[i * 2 + 2]) + ") {")
        for j in _statements(now.children[i * 2 + 3]):
            t.append(ident + j)
    if now.value["else"] == "true":
        t.append("} else {")
        for i in _statements(now.children[-1]):
            t.append(ident + i)
    t.append("}")
    return t


def _for(now: Node) -> list[str]:
    t = [f"for {now.value["label"] + " " if now.value["label"] else ""}({_type(now.children[0])} {now.value["iterator"]} in {_expression(now.children[1])}) {{"]
    for i in _statements(now.children[2]):
        t.append(ident + i)
    t.append("}")
    if now.value["else"] == "true":
        t.append("else {")
        for i in _statements(now.children[3]):
            t.append(f"    {i}")
        t.append("}")
    return t


def _while(now: Node) -> list[str]:
    t = [f"while ({_expression(now.children[0])}) {{"]
    for i in _statements(now.children[1]):
        t.append(ident + i)
    if now.value["else"] == "true":
        t.append("} else {")
        for i in _statements(now.children[2]):
            t.append(ident + i)
    t.append("}")
    return t


def _func(now: Node) -> list[str]:
    t: list[str] = []
    tt = f"func {"const " if now.value["const"] == "true" else ""}{_type(now.children[0])} {now.value["name"]}"
    if len(now.children[1].children) > 0:
        tt += f"<{', '.join(i.value["name"] for i in now.children[1].children)}>"
    t.append(tt + f"({_declare_args(now.children[2])}) {{")
    for i in _statements(now.children[3]):
        t.append(ident + i)
    t.append("}")
    return t


def _class(now: Node) -> list[str]:
    t = [f"class {now.value["name"]} {{"]
    for i in now.children:
        if i.type == "declare_attr":
            t.append(ident + _declare_attr(i))
        elif i.type == "method":
            for j in _method(i):
                t.append(ident + j)
        elif i.type == "declare_func":
            for j in _func(i):
                t.append(ident + j)
    return t


def _method(now: Node) -> list[str]:
    t: list[str] = []
    tt = f"method {now.value["kind "] if now.value["kind"] != "private" else ""}{_type(now.children[0])} {now.value["name"]}"
    if len(now.children[1].children) > 0:
        tt += f"<{', '.join(i.value["name"] for i in now.children[1].children)}>"
    t.append(tt + f"({_declare_args(now.children[2])}) {{")
    for i in _statements(now.children[3]):
        t.append(ident + i)
    t.append("}")
    return t


def _statements(now: Node) -> list[str]:
    t: list[str] = []
    for i in now.children:
        if i.type == "declare_var":
            t.append(_declare_var(i))
        elif i.type == "declare_attr":
            t.append(_declare_attr(i))
        elif i.type == "if":
            t.extend(_if(i))
        elif i.type == "for":
            t.extend(_for(i))
        elif i.type == "while":
            t.extend(_while(i))
        elif i.type == "break":
            t.append(_break(i))
        elif i.type == "return":
            t.append(_return(i))
        elif i.type == "continue":
            t.append(_continue(i))
        elif i.type == "func":
            t.extend(_func(i))
        elif i.type == "class":
            t.extend(_class(i))
        elif i.type == "expression":
            t.extend(_expression(i))
        elif i.type == "pass":
            t.append(_pass(i))
    return t


def _import(now: Node) -> str:
    t = "import "
    if now.value["alias"] == "stdlib":
        t += now.value["name"]
    else:
        t += '"' + now.value["name"] + '" as ' + now.value["alias"]
    return t


def _program(now: Node) -> str:
    t: list[str] = []
    for i in now.children:
        if i.type == "declare_var":
            t.append(_declare_var(i))
        elif i.type == "import":
            t.append(_import(i))
        elif i.type == "if":
            t.extend(_if(i))
        elif i.type == "for":
            t.extend(_for(i))
        elif i.type == "while":
            t.extend(_while(i))
        elif i.type == "func":
            t.extend(_func(i))
        elif i.type == "class":
            t.extend(_class(i))
        elif i.type == "expression":
            t.append(_expression(i))
        elif i.type == "pass":
            t.extend(_pass(i))
    return "\n".join(t)


result = _program(data)
print(result)

# with open(data["value"]["name"], "w") as f:
#     f.write(result)
