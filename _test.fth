
: cr ( -- ) 0 syscall0 ;
: . ( n -- ) 1 syscall1 ;

\ 2 2 - if 2 else 1 if 4 else 5 then then . cr

0 if 2 . cr then


\ 1 if 2 . else 3 . then

\ : abc ( asd ) 1 if 2 else 3 then ;

\ abc 1