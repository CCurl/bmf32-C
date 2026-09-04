#define DWC_SRC "( NOTE: 'boot.f' is used to generate 'boot.h' ) \
: last (l) @ ; \
: here (h) @ ; \
: inline    ( -- ) $40 last cell + c! ; \
: immediate ( -- ) $80 last cell + c! ; \
: cells  ( n--n' ) cell * ; inline \
: cell+  ( a--a1 ) cell + ; inline \
: ->code ( off--addr ) cells mem + ; \
: code@  ( off--dw )  ->code @ ; \
: code!  ( dw off-- ) ->code ! ; \
: , ( dw-- ) here dup 1+ (h) ! code! ; \
 \
: (exit)   ( --n )  0 ; inline \
: (lit)    ( --n )  1 ; inline \
: (jmp)    ( --n )  2 ; inline \
: (jmpz)   ( --n )  3 ; inline \
: (jmpnz)  ( --n )  4 ; inline \
: (njmpz)  ( --n )  5 ; inline \
: (njmpnz) ( --n )  6 ; inline \
: (ztype)  ( --n )  7 ; inline \
: (ftype)  ( --n )  8 ; inline \
 \
: if   (jmpz)   , here 0 , ; immediate \
: -if  (njmpz)  , here 0 , ; immediate \
: if0  (jmpnz)  , here 0 , ; immediate \
: -if0 (njmpnz) , here 0 , ; immediate \
: then here swap code!     ; immediate \
 \
: begin here ; immediate \
: again (jmp)     , , ; immediate \
: while (jmpnz)   , , ; immediate \
: -while (njmpnz) , , ; immediate \
: until (jmpz)    , , ; immediate \
 \
( val and (val) define a very efficient variable mechanism ) \
( Usage:  val a@   (val) (a)   : a! (a) ! ; ) \
: const ( n-- ) add-word (lit) , , (exit) , ; \
: val   ( -- ) 0 const ;   ( runtime: --n ) \
: (val) ( -- ) here 2 - ->code const ;   ( runtime: --a ) \
: kb ( n--m ) 1024 * ; \
: mb ( n--m ) kb kb ; \
 \
mem mem-sz + const dict-end \
32 ->code const (vh) \
64 kb ->code const vars \
vars (vh) ! \
: vhere ( --a ) (vh) @ ; \
: allot ( n-- ) (vh) +! ; \
: var   ( n-- ) vhere const allot ; \
: variable   ( -- ) cell const allot ; \
: 1- 1 - ; inline \
 \
( variables x,y,z are built-in ) \
: +L1 ( x -- )    +L x! ; \
: +L2 ( x y-- )   +L y! x! ; \
: +L3 ( x y z-- ) +L z! y! x! ; \
 \
: x++ ( -- )  x@+ drop ;  : x--  ( -- )  x@ 1- x! ;  : x@-  ( --n ) x@ x-- ; \
: c@x ( --b ) x@ c@ ;     : c@x+ ( --b ) x@+ c@ ;    : c@x- ( --b ) x@- c@ ; \
: c!x ( b-- ) x@ c! ;     : c!x+ ( b-- ) x@+ c! ;    : c!x- ( b-- ) x@- c! ; \
 \
: y++ ( -- )  y@+ drop ;  : y--  ( -- )  y@ 1- y! ;  : y@-  ( --n ) y@ y-- ; \
: c@y ( --b ) y@ c@ ;     : c@y+ ( --b ) y@+ c@ ;    : c@y- ( --b ) y@- c@ ; \
: c!y ( b-- ) y@ c! ;     : c!y+ ( b-- ) y@+ c! ;    : c!y- ( b-- ) y@- c! ; \
 \
: z++ ( -- )  z@+ drop ;  : z--  ( -- )  z@ 1- z! ;  : z@-  ( --n ) z@ z-- ; \
: c@z ( --b ) z@ c@ ;     : c@z+ ( --b ) z@+ c@ ;    : c@z- ( --b ) z@- c@ ; \
: c!z ( b-- ) z@ c! ;     : c!z+ ( b-- ) z@+ c! ;    : c!z- ( b-- ) z@- c! ; \
 \
( Temporary stack ) \
32 cells var tstk \
val tsp  (val) (tsp) \
: t!  ( n-- ) tsp cells tstk + ! ; \
: t@  ( --n ) tsp cells tstk + @ ; \
: t@+ ( --n ) t@ dup 1+ t! ; \
: >t  ( n-- ) tsp 1+ 31 and (tsp) ! t! ; \
: t>  ( --n ) t@ tsp 1- 31 and (tsp) ! ; \
: tdrop ( -- ) t> drop ; inline \
: t++ ( -- )  t@ 1+ t! ; inline \
 \
( Strings ) \
: compiling? ( --n ) state @ 1 = ; \
: (\") ( --a ) +L vhere dup z! x! 1 >in +! \
    begin \
        >in @ c@ y! 1 >in +! \
        y@ 0 = y@ '\"' = or \
        if  0 c!x+  z@ \
            compiling? if (lit) , , x@ (vh) ! then \
            -L exit \
        then \
        y@ c!x+ \
    again ; \
 \
: z\" ( str--addr ) (\") ; immediate \
: .\" ( str-- ) (\") compiling? if (ztype) , exit then ztype ; immediate \
: .f\" ( str-- ) (\") compiling? if (ftype) , exit then ftype ; immediate \
 \
( More core words ) \
: [ ( -- ) 0 state ! ; immediate  ( 0 = INTERPRET ) \
: ] ( -- ) 1 state ! ;            ( 1 = COMPILE ) \
: rdrop ( -- ) r> drop ; inline \
: tuck  ( a b--b a b )   swap over ; inline \
: nip   ( a b--b )       swap drop ; inline \
: ?dup ( n--n n|0 )  -if dup then ; \
: timer ( --n ) (ticks) @ ; \
: ms ( n-- ) timer + >r begin timer r@ > until rdrop ; \
: 2+    ( n--n' )        1+ 1+ ; inline \
: 2*    ( n--n' )        dup + ; inline \
: 2dup  ( a b--a b a b ) over over ; inline \
: 2drop ( a b-- )        drop drop ; inline \
: -rot ( a b c--c a b )  swap >r swap r> ; \
: 0< ( n--f )   0 <  ; inline \
: <= ( a b--f ) > 0= ; inline \
: >= ( a b--f ) < 0= ; inline \
: <> ( a b--f ) = 0= ; inline \
: type ( a n-- ) for dup c@ emit 1+ next drop ; \
: btwi ( n l h--f ) >t over <= swap t> <= and ; \
: key? ( --f )  (kbd-i) @ (kbd-o) @ <> ; \
: ascii? ( c--f )  32 127 btwi ; \
: com    ( n--n' ) -1 xor ; \
: negate ( n--n' ) com 1+ ; \
: abs ( n--n1 ) dup 0< if negate then ; \
: cr  ( -- )     13 emit 10 emit ; \
: tab ( -- )      9 emit ; \
: space  ( -- )  32 emit ; \
: spaces ( n-- ) for space next ; \
: /   ( a b--q ) /mod nip  ; \
: mod ( a b--r ) /mod drop ; \
: */  ( n m q--n' ) >r * r> / ; \
: unloop  ( -- ) (lsp) @ 3 - 0 max (lsp) ! ; \
: execute ( xt-- ) ?dup if >r then ; \
: decimal  ( -- )  #10 base ! ; \
: hex      ( -- )  $10 base ! ; \
: binary   ( -- )  %10 base ! ; \
 \
   1 var (neg) \
  65 var buf \
cell var (buf) \
: ?neg ( n--n' ) dup 0< dup (neg) c! if negate then ; \
: hold ( c-- )   -1 (buf) +! (buf) @ c! ; \
: #.   ( -- )    '.' hold ; \
: #n   ( r-- )   '0' + dup '9' > if 7 + then hold ; \
: #    ( n--q )  base @ /mod swap #n ; \
: #s   ( n--0 )  # -if #s exit then ; \
: <#   ( n--n' ) ?neg buf 65 + (buf) ! 0 hold ; \
: #>   ( n--a )  drop (neg) @ if '-' hold then (buf) @ ; \
: (.)  ( n-- )   <# #s #> ztype ; \
: .    ( n-- )   (.) space ; \
: .hex ( n-- )   2 $10 .nwb ; \
: .bin ( n-- )   8 %10 .nwb ; \
: .dec ( n-- )   0 #10 .nwb ; \
 \
: 0sp 0 (sp) ! ; \
: depth ( --n ) (sp) @ 1- ; \
: .s '(' emit space depth ?dup if \
        stk swap for cell+ dup @ . next drop \
    then ')' emit ; \
 \
: .word ( de-- ) cell+ 2+ ztype ; \
: words ( -- ) cr +L last x! 0 y! 0 z! begin \
        x@ dict-end < if0 z@ .f\" (%d words)\" -L exit then \
        x@ .word tab z++ \
        x@ cell+ 1+ c@ 6 > if y++ then \
        y@+ 7 > if cr 0 y! then \
        x@ de-sz + x! \
    again ; \
 \
: words-n ( n-- ) cr +L last x! 0 y! for \
        x@ .word tab \
        y@+ 7 > if cr 0 y! then \
		x@ de-sz + x! \
    next -L ; \
 \
: accept ( addr sz -- len ) \
    >t +L y! 0 z! \
    begin \
        key x! \
        x@ 10 = if tdrop 0 c!y z@ -L exit then \
        x@ ascii? if x@ c!y+ z++ x@ emit then \
        x@ 8 = z@ and if y-- z-- .f\" \\b \\b\" then \
    again \
; \
 \
cell var t4   cell var t5 \
: [[ here t4 !  vhere t5 !  1 state ! ; \
: ]] (exit) , 0 state ! t4 @ dup >r (h) ! t5 @ (vh) ! ; immediate \
 \
cell var t4   cell var t5   cell var t6 \
: marker ( -- ) here t4 !   vhere t5 !   last t6 ! ; \
: forget ( -- ) t4 @ (h) !  t5 @ (vh) !  t6 @ (l) ! ; \
 \
( Strings / Memory ) \
: pad    ( --a ) vhere $100 + ; \
: fill   ( a num ch-- ) -rot for 2dup c! 1+ next 2drop ; \
: wfill  ( a num w-- )  -rot for 2dup w! 2+ next 2drop ; \
: s-end  ( str--end ) dup s-len + ;   ( end: address of the null ) \
: s-cpy  ( dst src--dst ) 2dup s-len 1+ cmove ; \
: s-cat  ( dst src--dst ) over s-end  over s-len 1+  cmove ; \
: s-catc ( dst ch--dst )  over s-end  +L1  c!x+  0 c!x+  -L ; \
: s-catn ( dst num--dst ) <# #s #> s-cat ; \
: s-scat ( src dst--dst ) swap s-cat ; \
: s-eqn  ( s1 s2 n--f ) +L3 z@ for c@x+ c@y+ = if0 -L 0 unloop exit then next -L 1 ; \
: s-eq   ( s1 s2--f ) dup s-len 1+ s-eqn ; \
 \
: .c ( c-- ) dup ascii? if emit exit then drop .\" .\" ; \
: t1 ( addr-- ) $10 for dup c@ .c 1+ next drop ; \
: dump ( addr num-- ) 0 +L3 \
    y@ for \
        z@+ 0= if cr x@ 8 $10 .nwb .\" : \" then \
        c@x+ 2 $10 .nwb emit \
        z@ 8 = if space then \
        z@ $10 = if x@ $10 - t1 0 z! then \
    next -L ; \
 \
( Screen ) \
: vga ( --a ) $B8000 ; \
: cls ( -- ) vga 2000 $0F20 wfill  0 0 ->xy ; \
 \
( test / temp ) \
: bm ( mb -- ) 1000 dup * * timer swap for next timer swap - . ; \
 \
( Disk blocks are 512 bytes ) \
( Forth blocks are 1024 bytes ) \
: blk-rd ( addr blk#-- ) 2* 2dup disk-rd 1+ >t 512 + t> disk-rd ; \
: blk-wt ( addr blk#-- ) 2* 2dup disk-wt 1+ >t 512 + t> disk-wt ; \
cell var block \
 \
14 mb mem + const ram-disk \
: load ( n-- ) +L1 x@ kb ram-disk + y! \
    y@ x@ blk-rd  0 y@ 1023 + c! \
    y@ -L outer ; \
 \
( Editor ) \
1 kb var ed-blk \
16 const rows       64 const cols \
1 var isShow \
: cx ( -- x ) cursor-x @ ;  : cx! cursor-x ! ; \
: cy ( -- y ) cursor-y @ ;  : cy! cursor-y ! ; \
: ed-pos ( --pos ) cy cols * cx + ed-blk + ; \
: ed-norm ( -- ) ed-blk >t 1024 for t@ c@ if0 32 t@ c! then t++ next ; \
: ?ed-show ( -- ) isShow c@ if0 exit then \
    cx cy ed-blk +L3  0 0 ->xy  0 isShow c! \
    rows for \
      cols for  c@z+ emit  next cr \
    next  x@ y@ ->xy  -L ; \
: ed-show! ( -- ) 1 isShow c! ?ed-show ; \
: ed->ftr  ( -- ) cy >t cx >t  0 rows ->xy ; \
: ed-.ftr  ( addr cy cx-- ) block @ .f\" Block %d (%d,%d) %s   \" ; \
: ed-ftr   ( addr-- ) cy cx ed->ftr  ed-.ftr  t> t> ->xy ; \
: ed-clr   ( -- ) z\"         \" ed-ftr ; \
: ed-x! ( -- )  cx 0 max cols 1- min cx! ; \
: ed-y! ( -- )  cy 0 max rows 1- min cy! ; \
: ed->xy ( -- ) ed-x!  ed-y!  cx cy ->xy ; \
: ed-mv ( dx dy -- ) cursor-y +!  cursor-x +! ed->xy ; \
: ed-rd ( -- ) ed-blk block @ blk-rd ed-norm ; \
: ed-sv ( -- ) ed-blk block @ blk-wt ; \
: ed-rep-one ( -- ) z\" -r-\" ed-ftr key x! ed-clr \
      x@ ascii? if x@ ed-pos c! x@ emit ed-x! then ; \
: ed-rep ( -- ) z\" -replace-\" ed-ftr begin \
      key x!  \
      x@ 27 = if ed-clr exit then ( ESC => exit ) \
      x@ 10 = if cy 1+ cy! 0 cx! ed->xy then \
      x@  8 = if x@ emit then \
      x@ ascii? if x@ ed-pos c! x@ emit ed-x! then \
    again ; \
: ed-ins-eob ( -- ) ed-pos ed-blk 1023 + +L2 y@- z! \
    begin c@y- c!z- y@ x@ < until \
    32 c!z -L ed-show! ; \
: ed-ins-eol ( -- ) +L ed-pos x! \
    cy 1+ cols * ed-blk + 1- y! y@- z! \
    begin c@y- c!z- y@ x@ < until \
    32 c!x -L ed-show! ; \
: ed-del-eob ( -- ) +L ed-pos x! ed-blk 1023 + y! \
    begin x@ 1+ c@ c!x+ x@ y@ < while \
    32 c!x -L ed-show! ; \
: ed-del-eol ( -- ) +L ed-pos x! cy 1+ cols * ed-blk + 1- y! \
    begin x@ 1+ c@ c!x+ x@ y@ < while \
    32 c!x -L ed-show! ; \
: ed-go ( -- ) \
    x@ 'h' = if -1  0 ed-mv exit then \
    x@ 'j' = if  0  1 ed-mv exit then \
    x@ 'k' = if  0 -1 ed-mv exit then \
    x@ 'l' = if  1  0 ed-mv exit then \
    x@  32 = if  1  0 ed-mv exit then \
    x@ 'I' = if ed-ins-eob exit then \
    x@ 'i' = if ed-ins-eol exit then \
    x@ 'r' = if ed-rep-one exit then \
    x@ 'R' = if ed-rep     exit then \
    x@ 'x' = if ed-del-eol exit then \
    x@ 'X' = if ed-del-eob exit then \
    x@  10 = if cy rows 1- < if cr then exit then \
    x@  19 = if ed-sv z\" -saved-\" ed-ftr 500 ms ed-clr exit then \
	x@ '+' = if ed-sv block @ 1+ 1023 min block ! ed-rd ed-show! exit then \
	x@ '-' = if ed-sv block @ 1-    0 max block ! ed-rd ed-show! exit then \
    x@ <# #s #> ed-ftr ; \
: edit ( n-- ) block ! cls  ed-rd  1 isShow c! \
    begin ?ed-show z\" \" ed-ftr  key x!  \
      x@ 17 = if 0 rows 1+ ->xy exit then ( ctrl-q => exit ) \
      ed-go \
    again ; \
: ed block @ edit ; \
 \
: .version ( -- ) version <# # # #. # # #. # # #s #> ztype ; \
: .si .\" bmf32-C v\" .version .f\" \\n\\nhttps://github.com/CCurl/bmf32-C\" ; \
marker .si .f\" \\n\\nHello.\" \
"
