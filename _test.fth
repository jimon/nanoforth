
: cr ( -- ) 0 syscall0 ;
: . ( n -- ) 1 syscall1 ;
: r@ ( -- n ) r> r> dup >r swap >r ;
: i ( -- n ) r> r> dup >r swap >r ;

5 0 do i . cr loop
