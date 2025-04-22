# logic not (`!`) (3 byte)
r1 = !r0
# neg (`-`) (3 byte)
r1 = -r0
# get address (`@`) (3 byte)
r1 = @r0
# deference (`^`) (3 byte)
r1 = ^r0
# power (`**`) (4 byte)
r2 = r0 ** r1
# mul (`*`) (4 byte)
r2 = r0 * r1
# div (`/`) (4 byte)
r2 = r0 / r1
# rem (`%`) (4 byte)
r2 = r0 % r1
# add (`+`) (4 byte)
r2 = r0 + r1
# sub (`-`) (4 byte)
r2 = r0 - r1
# left shift (`<<`) (4 byte)
r2 = r0 << r1
# right shift (`>>`) (4 byte)
r2 = r0 >> r1
# lt (`<`) (4 byte)
r2 = r0 < r1
# leq (`<=`) (4 byte)
r2 = r0 <= r1
# gt (`>`) (4 byte)
r2 = r0 > r1
# geq (`>=`) (4 byte)
r2 = r0 >= r1
# eq (`==`) (4 byte)
r2 = r0 == r1
# neq (`!=`) (4 byte)
r2 = r0 != r1
# bit and (`&`) (4 byte)
r2 = r0 & r1
# bit or (`|`) (4 byte)
r2 = r0 | r1
# logic and (`&&`) (4 byte)
r2 = r0 && r1
# logic or (`||`) (4 byte)
r2 = r0 || r1
# goto (`goto`) (2 byte)
goto r0
# if-goto (`if-goto`) (3 byte)
if r1 goto r0
# index (`[]`) (3 byte)
r0[r1]
# attribute (`.`) (3 byte)
r0.r1
# call (`call`) (3 byte)
call r0 n
# assignment (`=`) (2 + N byte)
r0 = n