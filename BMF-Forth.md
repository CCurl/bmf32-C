# BMF-Forth Primitive Reference

This document describes the primitive words implemented by the in-kernel Forth VM in `dwc-vm.c`.

The VM is a small stack-based interpreter with a data stack, a return stack, a loop stack, a temporary stack, and a local-variable stack. Values are 32-bit signed cells (`cell`), and strings are C-style NUL-terminated buffers.

## Stack model

- Data stack: `dstk[]` with `dsp`
- Return stack: `rstk[]` with `rsp`
- Loop stack: `lstk[]` with `lsp`
- Local variable stack: `xstk[]` with `xsp` for `x`, `y`, and `z`
- Temporary stack: `tstk[]` with `tsp`

The VM uses classic Forth-style stack notation:

- `x -- y` means consume `x` and produce `y`
- `x1 x2 -- y1 y2` means consume two stack items

## Primitive dictionary

| Word | Stack effect | Meaning |
| --- | --- | --- |
| `exit` | `--` | Return from the current execution path. If the return stack is empty, stop execution. |
| `lit` | `-- n` | Push the next inline literal cell. This is used for immediate numeric literals. |
| `jmp` | `--` | Jump to the address stored in the following cell. |
| `jmpz` | `x --` | If `x == 0`, jump to the address stored in the next cell; otherwise skip it. |
| `jmpnz` | `x --` | If `x != 0`, jump to the address in the next cell; otherwise skip it. |
| `njmpz` | `x --` | Like `jmpz`, but tests the top-of-data-stack value without removing it. |
| `njmpnz` | `x --` | Like `jmpnz`, but tests the top-of-data-stack value without removing it. |
| `ztype` | `addr --` | Print a NUL-terminated string from the address on the stack. |
| `ftype` | `addr --` | This is the string-printing primitive used for embedded format-style text. |
| `dup` | `x -- x x` | Duplicate the top item. |
| `drop` | `x --` | Drop the top item. |
| `swap` | `x1 x2 -- x2 x1` | Swap the top two items. |
| `over` | `x1 x2 -- x1 x2 x1` | Copy the second item to the top. |
| `!` | `addr value --` | Store a cell value at memory address `addr`. |
| `@` | `addr -- value` | Fetch a cell from `addr`. |
| `c!` | `addr byte --` | Store a byte at `addr`. |
| `c@` | `addr -- byte` | Fetch a byte from `addr`. |
| `w!` | `addr u16 --` | Store a 16-bit word at `addr`. |
| `w@` | `addr -- u16` | Fetch a 16-bit word from `addr`. |
| `>r` | `x --` | Move `x` from the data stack to the return stack. |
| `r@` | `-- x` | Copy the top return-stack item to the data stack. |
| `r>` | `-- x` | Move the top of the return stack to the data stack. |
| `+L` | `--` | Increment the local stack pointer by 3 slots (creates new x, y, z). |
| `-L` | `--` | Decrement the local stack pointer by 3 slots (restores old x, y, z). |
| `x!` | `value --` | Store to local variable `x`. |
| `y!` | `value --` | Store to local variable `y`. |
| `z!` | `value --` | Store to local variable `z`. |
| `x@` | `-- value` | Fetch local variable `x`. |
| `y@` | `-- value` | Fetch local variable `y`. |
| `z@` | `-- value` | Fetch local variable `z`. |
| `x@+` | `-- value` | Fetch local variable `x` and increment it. |
| `y@+` | `-- value` | Fetch local variable `y` and increment it. |
| `z@+` | `-- value` | Fetch local variable `z` and increment it. |
| `*` | `n1 n2 -- n1*n2` | Multiply. |
| `+` | `n1 n2 -- n1+n2` | Add. |
| `-` | `n1 n2 -- n1-n2` | Subtract. |
| `/mod` | `n1 n2 -- q r` | Divide `n1` by `n2`, returning quotient and remainder. |
| `1+` | `n -- n+1` | Increment by one. |
| `<` | `n1 n2 -- flag` | True if `n1 < n2`; the VM returns `-1` for true and `0` for false. |
| `=` | `n1 n2 -- flag` | True if `n1 == n2`; returns `-1` or `0`. |
| `>` | `n1 n2 -- flag` | True if `n1 > n2`; returns `-1` or `0`. |
| `0=` | `n -- flag` | True if `n == 0`; returns `-1` or `0`. |
| `min` | `n1 n2 -- min(n1,n2)` | Minimum. |
| `max` | `n1 n2 -- max(n1,n2)` | Maximum. |
| `+!` | `addr value --` | Add `value` to the cell at `addr`. |
| `for` | `limit --` | Begin a counted loop; stores loop state in the local stack. |
| `i` | `-- index` | Push the current loop index. |
| `next` | `--` | Advance the loop counter; if not finished, jump back. |
| `and` | `n1 n2 -- n1&n2` | Bitwise AND. |
| `or` | `n1 n2 -- n1|n2` | Bitwise OR. |
| `xor` | `n1 n2 -- n1^n2` | Bitwise XOR. |
| `find` | `-- de` | Search the dictionary for the next word and return a pointer to the dictionary entry. (xt, flags, len, name) |
| `key` | `-- c` | Read one character from the keyboard buffer (blocking). |
| `emit` | `c --` | Emit one character to the VGA console. |
| `add-word` | `--` | Add a new dictionary entry from the input stream. |
| `outer` | `addr --` | Interpret a string from memory as Forth input. |
| `cmove` | `src dst count --` | Copy `count` bytes from `src` to `dst` using `memcpy()`. |
| `s-len` | `addr -- len` | Return the length of a C string at `addr`. |
| `s-eqi` | `s1 s2 -- flag` | Preform a case insensitive string comparison of `s1` and `s2`. |
| `.nwb` | `n width base --` | Print a number using a given width and base. |
| `see` | `--` | Display the definition of the most recent word found by the dictionary scanner. |
| `>t` | `x --` | Move `x` from the data stack to the temporary stack. |
| `t@` | `-- x` | Copy the top temporary-stack item to the data stack. |
| `t!` | `x --` | Store `x` to temporary stack TOS |
| `t>` | `-- x` | Move `x`` from the temporary stack to the data stack. |
| `disk-rd` | `addr n --` | Read the nth 512-byte sector from disk into `addr`. |
| `disk-wt` | `addr n --` | Write the nth 512-byte sector from `addr` to disk. |
| `->xy` | `x y --` | Set the VGA cursor position to the given `x, y` coordinates. |

## `ftype` details

FTYPE is the VM’s escape-aware, stack-driven string emitter, while `ztype` is the plain output primitive for raw strings.

`ftype` parses a string and emits it one character at a time. Unlike `ztype`, which prints a plain NUL-terminated string verbatim, `ftype` interprets escape sequences as control characters.

The implementation handles these escapes:

- `\b` -> ASCII 8 (backspace)
- `\e` -> ASCII 27 (ESC)
- `\n` -> ASCII 10 (newline)
- `\r` -> ASCII 13 (carriage return)
- `\t` -> ASCII 9 (tab)

The `%` operator is handled by `ftype` as a formatting escape, not a literal percent sign. The recognized format codes are:

- `%b` -> pop a value and print it in binary (`iToA(pop(), 2, 0)`) 
- `%c` -> pop a value and print it as an ascii character (`emit(pop())`)
- `%d` -> pop a value and print it in decimal (`iToA(pop(), 10, 0)`) 
- `%i` -> pop a value and print it using the current `base` (`iToA(pop(), base, 0)`) 
- `%q` -> emit a double-quote character (`"`) 
- `%s` -> pop an address and print the NUL-terminated string at that address (`zType((char *)pop())`) 
- `%x` -> pop a value and print it in hexadecimal (`iToA(pop(), 16, 0)`) 
- `%%` -> prints a `%`

Example:
- `12 .f" count=%d\n"` prints `count=12` followed by a carriage return

## Number handling

Numbers are accepted in decimal, binary, octal, and hexadecimal formats depending on the current base variable:

- `%` prefix forces binary
- `#` prefix forces decimal
- `$` prefix forces hexadecimal
- `'c'` single-character literal syntax is also accepted

When compiling a literal number, if the number does not fit into a 29-bit value (0x3FFFFFFF), `lit` `(number)` is compiled; otherwise bit 30 (0x40000000) is turned on and the value is compiled. This saves space for many literals.

## Dictionary and word lookup

The VM stores definitions in a dictionary built from the input stream. The key operations are:

- `add-word` creates a dictionary entry
- `find` looks up a word and returns its execution token
- `see` shows a word definition or primitive description

## Current default behavior

The default console color is white text on a black background, represented by the VGA attribute value `0x0F`. This is defined in the kernel and exposed through the VM as `text-color`.

## Source reference

The primitive list and dispatch logic are defined in:

- `dwc-vm.c`
- `dwc-vm.h`

The system's built-in Forth words are defined in:

- `boot.f`, which is translated into `boot.h` and compiled into the kernel.
