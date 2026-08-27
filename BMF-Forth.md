# BMF-Forth Primitive Reference

This document describes the primitive words implemented by the in-kernel Forth-like VM in `dwc-vm.c`.

The VM is a small stack-based interpreter with a data stack, return stack, and a small local/temporary stack. Values are 32-bit signed cells (`cell`), and strings are C-style NUL-terminated buffers.

## Stack model

- Data stack: `dstk[]`
- Return stack: `rstk[]`
- Loop stack: `lstk[]` (loop control state)
- Locals stack: `xstk[]` (x, y, z local variables)
- Temporary stack: `tstk[]`

The VM uses a classic prefix stack notation in its documentation:

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
| `ftype` | `addr --` | Interpret a Forth-style string literal with escape sequences like `\n`, `\r`, `\t`, `\b`. |
| `dup` | `x -- x x` | Duplicate the top item. |
| `drop` | `x --` | Drop the top item. |
| `swap` | `x1 x2 -- x2 x1` | Swap the top two items. |
| `over` | `x1 x2 -- x1 x2 x1` | Copy the second item to the top. |
| `!` | `addr value --` | Store a cell value at memory address `addr`. |
| `@` | `addr -- value` | Fetch a cell from memory. |
| `c!` | `addr byte --` | Store a byte at `addr`. |
| `c@` | `addr -- byte` | Fetch a byte from memory. |
| `>r` | `x --` | Move `x` from the data stack to the return stack. |
| `r@` | `-- x` | Copy the top return-stack item to the data stack. |
| `r>` | `-- x` | Move a value from the return stack to the data stack. |
| `+L` | `--` | Increment the local stack pointer by 3 slots. |
| `-L` | `--` | Decrement the local stack pointer by 3 slots. |
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
| `1-` | `n -- n-1` | Decrement by one. |
| `<` | `n1 n2 -- flag` | True if `n1 < n2`. |
| `=` | `n1 n2 -- flag` | True if `n1 == n2`. |
| `>` | `n1 n2 -- flag` | True if `n1 > n2`. |
| `0=` | `n -- flag` | True if `n == 0`. |
| `min` | `n1 n2 -- min(n1,n2)` | Minimum. |
| `max` | `n1 n2 -- max(n1,n2)` | Maximum. |
| `+!` | `addr value --` | Add `value` to the cell at `addr`. |
| `for` | `limit --` | Begin a counted loop; stores loop state in the local stack. |
| `i` | `-- index` | Push the current loop index. |
| `next` | `--` | Advance the loop counter; if not finished, jump back. |
| `and` | `n1 n2 -- n1&n2` | Bitwise AND. |
| `or` | `n1 n2 -- n1|n2` | Bitwise OR. |
| `xor` | `n1 n2 -- n1^n2` | Bitwise XOR. |
| `find` | `-- xt` | Search the dictionary for the next word and return its execution token. |
| `key` | `-- c` | Read one character from the keyboard buffer. |
| `key?` | `-- flag` | Return nonzero if a key is available. |
| `emit` | `c --` | Emit one character to the VGA console. |
| `add-word` | `--` | Add a new dictionary entry from the input stream. |
| `outer` | `addr --` | Interpret a string from memory as Forth input. |
| `cmove` | `count src dst --` | Copy `count` bytes from `src` to `dst` using `memmove()`. |
| `s-len` | `addr -- len` | Return the length of a C string at `addr`. |
| `.nwb` | `n width base --` | Print a number using a given width and base. |
| `see` | `--` | Display the definition of the most recent word found by the dictionary scanner. |
| `>t` | `x --` | Move `x` from the data stack to the temporary stack. |
| `t@` | `-- x` | Copy the top temporary-stack item to the data stack. |
| `t!` | `x --` | Store to temporary stack (replaces the current temporary stack item). |
| `t>` | `-- x` | Move a value from the temporary stack to the data stack. |

## Notes on control flow

The VM uses an instruction stream stored in memory. A word may be compiled as a sequence of primitive opcodes, where each primitive is represented by a numeric token. The `jmp`, `jmpz`, and `next` primitives are the main control-flow building blocks.

`for` / `next` implement simple counted loops:

- `for` initializes loop counters with the loop limit and current instruction pointer
- `i` pushes the loop index
- `next` increments the loop counter and jumps back if the loop continues

## Number handling

Numbers are accepted in decimal, binary, octal, and hexadecimal formats depending on the current base variable:

- `%` prefix forces binary
- `#` prefix forces decimal
- `$` prefix forces hexadecimal
- `'c'` single-character literal syntax is also accepted

The `lit` primitive is used when a literal value fits into the 30-bit literal field; otherwise a two-cell literal sequence is emitted.

## Dictionary and word lookup

The VM stores definitions in a dictionary built from the input stream. The key operations are:

- `add-word` creates a dictionary entry
- `find` looks up a word and returns its execution token
- `see` shows a word definition or primitive description

## Current default behavior

The default console color is white text on black background, represented by the VGA attribute value `0x0F`. This is set in the kernel and is exposed in the VM through `text-color`.

## Source reference

The primitive list and dispatch logic are defined in:

- `dwc-vm.c`
- `dwc-vm.h`
