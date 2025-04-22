## other
### assignment
byte: 2 + N byte  
symbol: `=`  
statement: `r0 = n`  
binary: `00000000 r0 n`
### goto
byte: 2 byte  
symbol: `goto`  
statement: `goto r0`  
binary: `00000001 r0`
## 3 byte
### logic not
symbol: `!`  
statement: `r1 = !r0`  
binary: `00000010 r0 r1`
### neg
symbol: `-`  
statement: `r1 = -r0`  
binary: `00000011 r0 r1`
### get address
symbol: `@`  
statement: `r1 = @r0`  
binary: `00000100 r0 r1`
### deference
symbol: `^`  
statement: `r1 = ^r0`  
binary: `00000101 r0 r1`
### if-goto
symbol: `if-goto`  
statement: `if r1 goto r0`  
binary: `00000110 r0 r1`
### index
symbol: `[]`  
statement: `r0[r1]`  
binary: `00000111 r0 r1`
### attribute
symbol: `.`  
statement: `r0.r1`  
binary: `00001000 r0 r1`
### call
symbol: `call`  
statement: `call r0 n`  
binary: `00001001 r0 r1`
## 4byte
### power
symbol: `**`  
statement: `r2 = r0 ** r1`  
binary: `00001010 r0 r1 r2`
### mul
symbol: `*`  
statement: `r2 = r0 * r1`  
binary: `00001011 r0 r1 r2`
### div
symbol: `/`  
statement: `r2 = r0 / r1`  
binary: `00001100 r0 r1 r2`
### rem
symbol: `%`  
statement: `r2 = r0 % r1`  
binary: `00001101 r0 r1 r2`
### add
symbol: `+`  
statement: `r2 = r0 + r1`  
binary: `00001110 r0 r1 r2`
### sub
symbol: `-`  
statement: `r2 = r0 - r1`  
binary: `00001111 r0 r1 r2`
### left shift
symbol: `<<`  
statement: `r2 = r0 << r1`  
binary: `00010000 r0 r1 r2`
### right shift
symbol: `>>`  
statement: `r2 = r0 >> r1`  
binary: `00010001 r0 r1 r2`
### lt
symbol: `<`  
statement: `r2 = r0 < r1`  
binary: `00010010 r0 r1 r2`
### leq
symbol: `<=`  
statement: `r2 = r0 <= r1`  
binary: `00010011 r0 r1 r2`
### gt
symbol: `>`  
statement: `r2 = r0 > r1`  
binary: `00010100 r0 r1 r2`
### geq
symbol: `>=`  
statement: `r2 = r0 >= r1`  
binary: `00010101 r0 r1 r2`
### eq
symbol: `==`  
statement: `r2 = r0 == r1`  
binary: `00010110 r0 r1 r2`
### neq
symbol: `!=`  
statement: `r2 = r0 != r1`  
binary: `00010111 r0 r1 r2`
### bit and
symbol: `&`  
statement: `r2 = r0 & r1`  
binary: `00011000 r0 r1 r2`
### bit or
symbol: `|`  
statement: `r2 = r0 | r1`  
binary: `00011001 r0 r1 r2`
### logic and
symbol: `&&`  
statement: `r2 = r0 && r1`  
binary: `00011010 r0 r1 r2`
### logic or
symbol: `||`  
statement: `r2 = r0 || r1`  
binary: `00011011 r0 r1 r2`