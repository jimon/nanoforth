
: cr ( -- ) 0 syscall0 ;
: . ( n -- ) 1 syscall1 ;
\ : r@ ( -- n ) r> r> dup >r swap >r ;
\ : i ( -- n ) r> r> dup >r swap >r ;
\ : 2dup ( ab -- abab ) >r swap dup >r swap r> r> ;

\ 15 0 do i . cr loop

\ 33 2 !

\ 1 @ . cr
\ 2 @ . cr

variable asd1
variable asd2
variable asd3
1 asd1 ! asd1 @ . cr
2 asd2 ! asd2 @ . cr
3 asd3 ! asd3 @ . cr
