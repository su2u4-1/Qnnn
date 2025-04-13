# use
run 1 or 2
## 1
```sh
$ python ./build.py ./src -o ./build/app.exe -a ./<file_name>.qn --<flag>
```
## 2
```sh
$ g++ ./src/lexer.cpp ./src/main.cpp ./src/parser.cpp ./src/qlib.cpp -o ./build/app.exe
$ ./build/app.exe ./<file_name>.qn --<flag>
```
# info
language name: `Qnnn`  
file extension: `.qn`
## environment
### compiler
gcc version: `13.2.0`  
cpp version: `201703L`
