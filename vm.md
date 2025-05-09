# command
## tier 0
`push $[]`  
`pop $[]`  
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if_goto $[] [label]`  
`input [value]`  
`set [label]`
`call [function name] [type signature]`  
`function [function name] [type signature]`  
`return`  
`get attr $[object pointer] [n]`  
`get index $[container] [n]`
## tier 1
`push $[]`  
`pop $[]`  
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if_goto $[] [label]`  
`input [value]`  
`set [label]`
## tier 2
`copy $[] $[]`    
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if_goto $[] $[]`  
`input [value] $[]`  
`set [label]`
`get [label] $[]`
## tier 3
`copy $[] $[]`    
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if_goto $[] $[]`  
`input [value] $[]`
# op
## op(2)
logic not: `r1 = !r0`  
neg: `r1 = -r0`  
get address: `r1 = @r0`  
deference: `r1 = ^r0`
## op(3)
power: `r2 = r0 ** r1`  
mul: `r2 = r0 * r1`  
div: `r2 = r0 / r1`  
rem: `r2 = r0 % r1`  
add: `r2 = r0 + r1`  
sub: `r2 = r0 - r1`  
left shift: `r2 = r0 << r1`  
right shift: `r2 = r0 >> r1`  
lt: `r2 = r0 < r1`  
leq: `r2 = r0 <= r1`  
gt: `r2 = r0 > r1`  
geq: `r2 = r0 >= r1`  
eq: `r2 = r0 == r1`  
neq: `r2 = r0 != r1`  
bit and: `r2 = r0 & r1`  
bit or: `r2 = r0 | r1`  
logic and: `r2 = r0 && r1`  
logic or: `r2 = r0 || r1`
# register
## low level register
used when translate tier1 to tier2, and tier2 to tier3  
|name|mean|
|-|-|
|$A|address|
|$M|memory|
|$N|now stack head|
|$T|temporary|
## high level register
used when translate tier0 to tier1  
|name|mean|
|-|-|
|$T|temporary|
# translate
## tier0 to tier1
### `call [function name] [type signature]`  
```
```
### `function [function name] [type signature]`  
```
set "{function name}({type signature})"
```
### `return`  
```
```
### `get attr $[object pointer] [n]`  
```
```
### `get index $[container] [n]`
```
```
## tier1 to tier2
### `push $[]`
```
copy $N $A
copy $[] $M
input 1 $T
add $N $T $N
```
### `pop $[]`
```
input 1 $T
sub $N $T $N
copy $N $A
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
copy $N $A
copy $T $M
input 1 $T
add $N $T $N
```
### `get label [label]`
```
label [label] $T
copy $N $A
copy $T $M
input 1 $T
add $N $T $N
```
## tier2 to tier3
### `set label [label]`
```
label_table[label] = address
```
### `get label [label] $[]`
```
input [label_table[label]] $[]
```