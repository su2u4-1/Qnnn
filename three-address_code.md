### assignment 
symbol: `=`  
statement: `r0 = n`
### goto
symbol: `goto`  
statement: `goto r0`
### return
symbol: `return`  
statement: `return r0`
### label
symbol: `label`  
statement: `label L`
### logic not
symbol: `!`  
statement: `r1 = !r0`
### neg
symbol: `-`  
statement: `r1 = -r0`
### get address
symbol: `@`  
statement: `r1 = @r0`
### deference
symbol: `^`  
statement: `r1 = ^r0`
### if-goto
symbol: `if-goto`  
statement: `if r0 goto L`
### index
symbol: `[]`  
statement: `r0[r1]`
### attribute
symbol: `.`  
statement: `r0.r1`
### call
symbol: `call`  
statement: `call r0 n`
### power
symbol: `**`  
statement: `r2 = r0 ** r1`
### mul
symbol: `*`  
statement: `r2 = r0 * r1`
### div
symbol: `/`  
statement: `r2 = r0 / r1`
### rem
symbol: `%`  
statement: `r2 = r0 % r1`
### add
symbol: `+`  
statement: `r2 = r0 + r1`
### sub
symbol: `-`  
statement: `r2 = r0 - r1`
### left shift
symbol: `<<`  
statement: `r2 = r0 << r1`
### right shift
symbol: `>>`  
statement: `r2 = r0 >> r1`
### lt
symbol: `<`  
statement: `r2 = r0 < r1`
### leq
symbol: `<=`  
statement: `r2 = r0 <= r1`
### gt
symbol: `>`  
statement: `r2 = r0 > r1`
### geq
symbol: `>=`  
statement: `r2 = r0 >= r1`
### eq
symbol: `==`  
statement: `r2 = r0 == r1`
### neq
symbol: `!=`  
statement: `r2 = r0 != r1`
### bit and
symbol: `&`  
statement: `r2 = r0 & r1`
### bit or
symbol: `|`  
statement: `r2 = r0 | r1`
### logic and
symbol: `&&`  
statement: `r2 = r0 && r1`
### logic or
symbol: `||`  
statement: `r2 = r0 || r1`