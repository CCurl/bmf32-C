# BMF Boot Words Reference

This is a reference for the words defined in `boot.f`.

| Word | Stack effect | Meaning |
| --- | --- | --- |
| `last` | `-- addr` | Return the address of the most recent dictionary entry. |
| `here` | `-- addr` | Return the current code pointer. |
| `inline` | `--` | Mark the most recent word as inline. |
| `immediate` | `--` | Mark the most recent word as immediate. |
| `cells` | `n -- n'` | Multiply by the cell size. |
| `cell+` | `a -- a1` | Add one cell size to the address. |
| `->code` | `off -- addr` | Convert a code offset to an address in memory. |
| `code@` | `off -- dw` | Fetch the cell at the given code offset. |
| `code!` | `dw off --` | Store a cell into the compiled code region. |
| `,` | `dw --` | Append one cell to the compiled code stream. |
| `(exit)` | `-- n` | Compile-time marker for the primitive exit opcode. |
| `(lit)` | `-- n` | Compile-time marker for the primitive literal opcode. |
| `(jmp)` | `-- n` | Compile-time marker for the primitive jump opcode. |
| `(jmpz)` | `-- n` | Compile-time marker for the primitive conditional-jump-if-zero opcode. |
| `(jmpnz)` | `-- n` | Compile-time marker for the primitive conditional-jump-if-nonzero opcode. |
| `(njmpz)` | `-- n` | Compile-time marker for the primitive non-destructive jump-if-zero opcode. |
| `(njmpnz)` | `-- n` | Compile-time marker for the primitive non-destructive jump-if-nonzero opcode. |
| `(ztype)` | `-- n` | Compile-time marker for the primitive string-print opcode. |
| `(ftype)` | `-- n` | Compile-time marker for the primitive formatted-string-print opcode. |
| `if` | immediate | Compile a conditional jump and leave a patch site. |
| `-if` | immediate | Compile a negative-conditional jump. |
| `if0` | immediate | Compile a jump when the value is nonzero. |
| `-if0` | immediate | Compile a jump when the value is zero. |
| `then` | immediate | Patch the deferred forward jump target. |
| `begin` | immediate | Mark the start of a loop. |
| `again` | immediate | Emit an unconditional jump back to the loop start. |
| `while` | immediate | Compile a conditional loop-back jump. |
| `-while` | immediate | Conditional loop-back on a nonzero check. |
| `until` | immediate | Compile a loop-back jump until a condition becomes true. |
| `const` | `n --` | Create a constant word whose value is `n` and compile a literal followed by `exit`. |
| `val` | `--` | Create a zero-initialized constant-like value. |
| `(val)` | `--` | Compile a constant-like value from the current code pointer. |
| `kb` | `n -- m` | Multiply by 1024. |
| `mb` | `n -- m` | Multiply by 1024 * 1024. |
| `dict-end` | `-- addr` | Constant marking the end of the dictionary area. |
| `(vh)` | `-- addr` | Internal variable holding the variable-allocation pointer. |
| `vars` | `-- addr` | Constant giving the base of the variable area. |
| `vhere` | `-- a` | Return the current variable-allocation pointer. |
| `allot` | `n --` | Advance the variable area by `n` bytes. |
| `var` | `n --` | Reserve a variable with the given initial value. |
| `variable` | `--` | Reserve a new cell-sized variable. |
| `1-` | `n -- n-1` | Decrement by one. |
| `+L1` | `x --` | Prepare the local-variable stack state for one item. |
| `+L2` | `x y --` | Prepare the local-variable stack state for two items. |
| `+L3` | `x y z --` | Prepare the local-variable stack state for three items. |
| `x++` | `--` | Increment the `x` local variable. |
| `x--` | `--` | Decrement the `x` local variable. |
| `x@-` | `-- n` | Fetch `x` and then decrement it. |
| `c@x` | `-- b` | Read a byte from the address in `x`. |
| `c@x+` | `-- b` | Read a byte from `x` then increment `x`. |
| `c@x-` | `-- b` | Read a byte from `x` then decrement `x`. |
| `c!x` | `b --` | Write a byte to the address in `x`. |
| `c!x+` | `b --` | Write a byte to `x` and increment `x`. |
| `c!x-` | `b --` | Write a byte to `x` and decrement `x`. |
| `y++` | `--` | Increment the `y` local variable. |
| `y--` | `--` | Decrement the `y` local variable. |
| `y@-` | `-- n` | Fetch `y` and then decrement it. |
| `c@y` | `-- b` | Read a byte from the address in `y`. |
| `c@y+` | `-- b` | Read a byte from `y` then increment `y`. |
| `c@y-` | `-- b` | Read a byte from `y` then decrement `y`. |
| `c!y` | `b --` | Write a byte to the address in `y`. |
| `c!y+` | `b --` | Write a byte to `y` and increment `y`. |
| `c!y-` | `b --` | Write a byte to `y` and decrement `y`. |
| `z++` | `--` | Increment the `z` local variable. |
| `z--` | `--` | Decrement the `z` local variable. |
| `z@-` | `-- n` | Fetch `z` and then decrement it. |
| `c@z` | `-- b` | Read a byte from the address in `z`. |
| `c@z+` | `-- b` | Read a byte from `z` then increment `z`. |
| `c@z-` | `-- b` | Read a byte from `z` then decrement `z`. |
| `c!z` | `b --` | Write a byte to the address in `z`. |
| `c!z+` | `b --` | Write a byte to `z` and increment `z`. |
| `c!z-` | `b --` | Write a byte to `z` and decrement `z`. |
| `compiling?` | `-- n` | Return true if the system is in compile state. |
| `("` | `-- a` | Build a counted string literal in the variable area. |
| `z"` | immediate | Parse a NUL-terminated string literal and create a compile-time string literal. |
| `."` | immediate | Compile a NUL-terminated string literal for `ztype`; otherwise emit the string immediately. |
| `.f"` | immediate | Compile a NUL-terminated string literal for `ftype`; otherwise emit the string with escape processing. |
| `[` | `--` | Switch to interpret mode. |
| `]` | `--` | Switch to compile mode. |
| `rdrop` | `--` | Drop the top item from the return stack. |
| `tdrop` | `--` | Drop the top item from the temporary stack. |
| `tuck` | `a b -- b a b` | Copy the second item under the first. |
| `nip` | `a b -- b` | Remove the second item. |
| `?dup` | `n -- n n | 0` | Duplicate only if nonzero. |
| `timer` | `-- n` | Return the current timer tick count. |
| `ms` | `n --` | Delay for `n` milliseconds. |
| `2+` | `n -- n'` | Add 2. |
| `2*` | `n -- n'` | Multiply by 2. |
| `2dup` | `a b -- a b a b` | Duplicate the top two values. |
| `2drop` | `a b --` | Drop the top two values. |
| `-rot` | `a b c -- c a b` | Rotate the top three values. |
| `0<` | `n -- f` | True if `n < 0`. |
| `<=` | `a b -- f` | True if `a <= b`. |
| `>=` | `a b -- f` | True if `a >= b`. |
| `<>` | `a b -- f` | True if `a != b`. |
| `type` | `a n --` | Emit `n` characters starting at address `a`. |
| `btwi` | `n l h -- f` | True if `n` lies between `l` and `h`. |
| `key?` | `-- f` | Return true if a key is available. |
| `ascii?` | `c -- f` | True if `c` is printable ASCII. |
| `com` | `n -- n'` | Bitwise ones complement. |
| `negate` | `n -- n'` | Two’s-complement negation. |
| `abs` | `n -- n1` | Absolute value. |
| `cr` | `--` | Print carriage return and newline. |
| `tab` | `--` | Print a tab character. |
| `space` | `--` | Print a space. |
| `spaces` | `n --` | Print `n` spaces. |
| `/` | `a b -- q` | Integer division. |
| `mod` | `a b -- r` | Modulo. |
| `*/` | `n m q -- n'` | Multiply and divide. |
| `unloop` | `--` | Reset loop state. |
| `execute` | `xt --` | Execute a dictionary entry if it exists. |
| `decimal` | `--` | Set the numeric base to decimal. |
| `hex` | `--` | Set the numeric base to hexadecimal. |
| `binary` | `--` | Set the numeric base to binary. |
| `(neg)` | `-- addr` | Internal sign flag for number formatting. |
| `buf` | `-- addr` | Numeric-format buffer pointer. |
| `(buf)` | `-- addr` | Internal buffer address used by numeric formatting. |
| `?neg` | `n -- n'` | Return the absolute value and store the sign flag. |
| `hold` | `c --` | Insert a character into the numeric print buffer. |
| `#.` | `--` | Add a decimal point to the formatted string buffer. |
| `#n` | `n --` | Convert a digit to text and store it. |
| `#` | `n -- m` | Divide by the current base and append a digit. |
| `#s` | `n -- 0` | Repeatedly convert digits until zero. |
| `<#` | `n -- n'` | Start numeric conversion state. |
| `#>` | `n -- a` | Finish numeric conversion and return the buffer address. |
| `(.)` | `n --` | Print a number in the current base. |
| `.` | `n --` | Print a number and then a space. |
| `.hex` | `n --` | Print using hexadecimal format. |
| `.bin` | `n --` | Print using binary format. |
| `.dec` | `n --` | Print using decimal format. |
| `0sp` | `--` | Clear the data stack pointer. |
| `depth` | `-- n` | Return the current stack depth. |
| `.s` | `--` | Print the current data-stack contents. |
| `.word` | `de --` | Print the name of a dictionary entry. |
| `words` | `--` | Print a list of dictionary words. |
| `words-n` | `n --` | Print the most recently defined `n` words. |
| `accept` | `addr sz -- len` | Read a line of input into a buffer and return the length. |
| `[[` | immediate | Save the current code pointer and variable pointer and enter compile mode. |
| `]]` | immediate | Restore the saved state and reset compile mode. |
| `marker` | `--` | Save the current dictionary and variable state so it can be restored later. |
| `forget` | `--` | Restore the dictionary state to the marker. |
| `pad` | `-- a` | Return a scratch area pointer. |
| `fill` | `a num ch --` | Fill memory with a byte pattern. |
| `s-end` | `str -- end` | Return the address of the terminating NUL byte in a string. |
| `s-cpy` | `dst src -- dst` | Copy one string into another. |
| `s-cat` | `dst src -- dst` | Concatenate one string onto another. |
| `s-catc` | `dst ch -- dst` | Append one character to a string. |
| `s-catn` | `dst num -- dst` | Append a numeric string representation. |
| `s-scat` | `src dst -- dst` | Concatenate `src` into `dst` with the arguments reversed from `s-cat`. |
| `s-eqn` | `s1 s2 n -- f` | Compare two strings for the first `n` bytes. |
| `s-eq` | `s1 s2 -- f` | Compare two strings for equality. |
| `.c` | `c --` | Print a character, or a dot if it is non-printable. |
| `t1` | `addr --` | Dump a block of 16 bytes using `.c`. |
| `dump` | `addr num --` | Dump memory as hex bytes. |
| `vga` | `-- addr` | Constant address of the VGA text buffer. |
| `bm` | `mil --` | An empty loop benchmark in milliseconds. |
| `blk-rd` | `addr blk --` | Read a 1024-byte Forth block from disk. |
| `blk-wt` | `addr blk --` | Write a 1024-byte Forth block to disk. |
| `.version` | `--` | Print the version number. |
| `.si` | `--` | Print some system information. |
