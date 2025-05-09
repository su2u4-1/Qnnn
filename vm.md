# command
## tier 0
`push $[]`  
`pop $[]`  
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if goto $[] [label]`  
`input [value]`  
`label [label]`  
`call [number of args]`  
`function [function name] [type signature]`  
`return`  
`get attr $[object pointer] [n]`  
`get index $[container] [n]`
## tier 1
`push $[]`  
`pop $[]`  
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if goto $[] [label]`  
`input [value]`  
`label [label]`
## tier 2
`copy $[] $[]`    
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if goto $[] $[]`  
`input [value] $[]`  
`label [label] $[]`
## tier 3
`copy $[] $[]`    
`[op(3)] $[r0] $[r1] $[r2]`  
`[op(2)] $[r0] $[r1]`  
`if goto $[] $[]`  
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
|name|mean|
|-|-|
|$A|address|
|$B|base|
|$C|condition, constant|
|$D|data|
|$E||
|$F||
|$G|global|
|$H|heap|
|$I|integer, index|
|$J||
|$K||
|$L|local, label|
|$M|memory|
|$N|now|
|$O|output|
|$P|pointer|
|$Q||
|$R|return, result|
|$S|stack|
|$T|temporary|
|$U||
|$V|value|
|$W||
|$X|x|
|$Y|y|
|$Z|z|