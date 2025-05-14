# command
## tier 0
`push $[]`  
`pop $[]`  
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(3)] $[r0] [value] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`[op(2)] [value] $[r1]`  
`[op(3)]`
`[op(2)]`  
`if_goto $[] [label]`  
`input [value]`  
`set [label]`
`call [subroutine name] [type signature]`  
`subroutine [subroutine name] [type signature]`  
`return`  
`get attr $[object pointer] [n]`  
`get index $[container] [n]`  
> \[op(2)\] assignment `= $[r0] $[r1]` equal `copy $[r0] $[r1]`  
> \[op(2)\] can pop a value from stack and push them back onto the stack after computation.  
> \[op(3)\] can pop two values from stack and push them back onto the stack after computation.
## tier 1
`push $[]`  
`pop $[]`  
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if_goto $[] [label]`  
`goto [label]`  
`input [value]`  
`set [label]`
`get [label]`
## tier 2
`copy $[] $[]`    
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if_goto $[] $[]`  
`goto $[]`  
`input [value] $[]`  
`set [label]`
`get [label]`
## tier 3
`copy $[] $[]`    
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if_goto $[] $[]`  
`goto $[]`  
`input [value] $[]`
# op
## op(2)
assignment: `r1 = r0` \[`asi`\]  
logic not: `r1 = !r0` \[`not`\]  
neg: `r1 = -r0` \[`neg`\]  
get address: `r1 = @r0` \[`ptr`\]  
deference: `r1 = ^r0` \[`def`\]
## op(3)
power: `r2 = r0 ** r1` \[`pow`\]  
mul: `r2 = r0 * r1` \[`mul`\]  
div: `r2 = r0 / r1` \[`div`\]  
rem: `r2 = r0 % r1` \[`rem`\]  
add: `r2 = r0 + r1` \[`add`\]  
sub: `r2 = r0 - r1` \[`sub`\]  
left shift: `r2 = r0 << r1` \[`lsh`\]  
right shift: `r2 = r0 >> r1` \[`rsh`\]  
lt: `r2 = r0 < r1` \[`lt`\]  
leq: `r2 = r0 <= r1` \[`leq`\]  
gt: `r2 = r0 > r1` \[`gt`\]  
geq: `r2 = r0 >= r1` \[`geq`\]  
eq: `r2 = r0 == r1` \[`eq`\]  
neq: `r2 = r0 != r1` \[`neq`\]  
bit and: `r2 = r0 & r1` \[`band`\]  
bit or: `r2 = r0 | r1` \[`bor`\]  
logic and: `r2 = r0 && r1` \[`land`\]  
logic or: `r2 = r0 || r1` \[`lor`\]
# register
## low level register
|name|mean|
|-|-|
|$A|address|
|$C|content segment start|
|$D|data|
|$F|frame base|
|$G|global segment start|
|$H|heap|
|$M|memory|
|$S|stack head|
|$T|temporary|
|$X|unknown number|
|$Y|unknown number|
|$Z|unknown number|
# translate
## init
```
input {memory size} $S
input 0 $A
input {memory size}
input 4 $A
input {code start} $M
input 8 $A
input {content start} $M
input 12 $A
input {global start} start $N
input 16 $A
input {heap start} start $N
```
## tier0 to tier1
### `call [subroutine name] [type signature]`
```
asi $S $D                           // save now stack head to $D
input {number of args}              // push {number of args}
push $F                             // push old $F
asi $D $F                           // set new $F from $D
get "call{n}"                       // get return address
input {number of local variable}    // push return address
push 0                              // set local variable scope
// ^ repent this {number of local variable} times
goto "{subroutine name}({type signature})"  // call subroutine
set "call{n}"                       // set label for return
```
### `subroutine [subroutine name] [type signature]`
```
set "{subroutine name}({type signature})" // set subroutine start label
```
### `return`
```
pop $D          // save return value to $D
asi $A $F       // load $F to $A
sub $F $M $S    // set $S = *$F, equal set stack head
push $D         // push return value to stack head from $D
sub $F 2 $A     // load $F - 2 to $A
asi $M $D       // save return address to $D
sub $F 1 $A     // load $F - 1 to $A
asi $M $F       // set $F to old $F from $M
goto $D         // return control to the parent subroutine
```
### `get attr $[object pointer] [n]`
```
add $[object pointer] {n} $A
push $M
```
### `get index $[container] [n]`
```
add $[container] {n} $A
push $M
```
### `[op(3)] $[r0] [value] $[r2]`
```
input [value] $T
[op(3)] $[r0] $T $[r2]
```
### `[op(2)] [value] $[r1]`
```
input [value] $T
[op(2)] $T $[r1]
```
### `[op(3)]`
```
pop $T
pop $D
[op(3)] $D $T $D
push $D
```
### `[op(2)]`
```
pop $T
[op(2)] $T $T
push $T
```
## tier1 to tier2
### `push $[]`
```
copy $S $A
copy $[] $M
sub $S 1 $S
```
### `pop $[]`
```
add $S 1 $S
copy $S $A
copy $M $[]
```
### `if goto $[] $[label]`
```
get label [label] $T
if goto $[] $T
```
### `input [value]`
```
input [value] $T
copy $S $A
copy $T $M
sub $S 1 $S
```
### `get [label]`
```
get [label] $T
copy $S $A
copy $T $M
sub $S 1 $S
```
## tier2 to tier3
### `set [label]`
```
label_table[label] = address
```
### `get [label] $[]`
```
input [label_table[label]] $[]
```