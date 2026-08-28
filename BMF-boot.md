# BMF Boot Words Reference

This document describes the higher-level Forth words defined in `boot.f` and emitted into `boot.h`. These words are the user-facing runtime vocabulary layered on top of the VM primitives in `dwc-vm.c`.

Unlike the primitive document, this file focuses on the words a program will actually use when writing or extending the system.

## Core model

The boot vocabulary adds several foundational concepts:

- dictionary creation and word definition
- variables and constants
- loop and branch control flow
- string builders and formatted output
- stack manipulation helpers
- memory helpers and simple debug tools

The system is still a small, compact Forth; it is not ANSI-standard Forth, but it is very usable for kernel-level work.

## Hardware boundary

The boot words are the higher-level runtime vocabulary, but they sit on top of the VM's primitive layer. The disk driver itself is intentionally kept at the raw hardware boundary and is not described as a filesystem vocabulary here.

The actual block-device interface is:

- `ata_read_block(block_number, buffer)`
- `ata_write_block(block_number, buffer)`

and the VM exposes the equivalent primitive words:

- `blk-r` ( addr blockNumber-- ) - Read block
- `blk-w` ( addr blockNumber-- ) - Write block

These words are part of the primitive layer in `dwc-vm.c`, not part of the boot dictionary generated from `boot.f`.

## Dictionary and code-generation words

These words manipulate the VM dictionary and compiled code stream.

### `last`

- Stack: `-- addr`
- Returns the address of the most recent dictionary entry.

### `here`

- Stack: `-- addr`
- Returns the current code pointer.

### `inline`

- Stack: `--`
- Marks the most recent word as inline.

### `immediate`

- Stack: `--`
- Marks the most recent word as immediate.

### `cells`

- Stack: `n -- n'`
- Multiplies by the cell size.

### `cell+`

- Stack: `a -- a1`
- Adds one cell size to the address.

### `->code`

- Stack: `off -- addr`
- Convert a code offset to an address in memory.

### `code@`

- Stack: `off -- dw`
- Fetch the cell at the given code offset.

### `code!`

- Stack: `dw off --`
- Store a cell into the compiled code region.

### `,`

- Stack: `dw --`
- Append one cell to the compiled code stream.

### `const`

- Stack: `n --`
- Creates a constant word whose value is `n` and compiles a literal followed by `exit`.

### `val`

- Stack: `--`
- Creates a zero-initialized constant-like value.

### `vhere`

- Stack: `-- a`
- Returns the current variable-allocation pointer.

### `allot`

- Stack: `n --`
- Advances the variable area by `n` bytes.

### `var`

- Stack: `n --`
- Reserve a variable with the given initial value.

### `variable`

- Stack: `--`
- Reserve a new cell-sized variable.

## Control-flow words

These are the structured building blocks used to compile conditionals and loops.

### `if`

- Immediate word.
- Compiles a conditional jump and leaves a patch site.

### `-if`

- Immediate word.
- Compiles a negative-conditional jump.

### `if0`

- Immediate word.
- Compiles a jump when the value is nonzero.

### `-if0`

- Immediate word.
- Compiles a jump when the value is zero.

### `then`

- Immediate word.
- Patches the deferred forward jump target.

### `begin`

- Immediate word.
- Marks the start of a loop.

### `again`

- Immediate word.
- Emits an unconditional jump back to the loop start.

### `while`

- Immediate word.
- Compiles a conditional loop-back jump.

### `-while`

- Immediate word.
- Conditional loop-back on a nonzero check.

### `until`

- Immediate word.
- Compiles a loop-back jump until a condition becomes true.

## Variable and local-variable helpers

The VM exposes `x`, `y`, and `z` as local variables, and the boot vocabulary adds convenience words around them.

### `+L1`

- Stack: `x --`
- Prepares local variable stack state for one item.

### `+L2`

- Stack: `x y --`
- Prepares local variable stack state for two items.

### `+L3`

- Stack: `x y z --`
- Prepares local variable stack state for three items.

### `x++`, `x--`, `x@-`

- Various local-variable increment and decrement helpers.

### `c@x`, `c@x+`, `c@x-`

- Read bytes via `x` variable pointer.

### `c!x`, `c!x+`, `c!x-`

- Write bytes via `x` variable pointer.

The same pattern exists for `y` and `z`:

- `y++`, `y--`, `y@-`
- `c@y`, `c@y+`, `c@y-`
- `c!y`, `c!y+`, `c!y-`
- `z++`, `z--`, `z@-`
- `c@z`, `c@z+`, `c@z-`
- `c!z`, `c!z+`, `c!z-`

## String words

These words build and manipulate strings in the VM memory model.

### `compiling?`

- Stack: `-- n`
- Returns true if the system is in compile state.

### `z"`

- Immediate word.
- Parses a NUL-terminated string literal and creates a compile-time string literal.

### `."`

- Immediate word.
- Compiles a NUL-terminated string literal for `ztype` when in compile mode; otherwise emits the string immediately.

### `.f"`

- Immediate word.
- Compiles a NUL-terminated string literal for `ftype` when in compile mode; otherwise emits the string with escape processing.

### `s-end`

- Stack: `str -- end`
- Returns the address of the terminating NUL byte in a NUL-terminated string.

### `s-cpy`

- Stack: `dst src -- dst`
- Copy one string into another.

### `s-cat`

- Stack: `dst src -- dst`
- Concatenate one string onto another.

### `s-catc`

- Stack: `dst ch -- dst`
- Append a single character to a string.

### `s-catn`

- Stack: `dst num -- dst`
- Append a numeric string representation.

### `s-scat`

- Stack: `src dst -- dst`
- Concatenate `src` into `dst` with the arguments reversed from `s-cat`.

### `s-eqn`

- Stack: `s1 s2 n -- f`
- Compare two NUL-terminated strings for the first `n` bytes.

### `s-eq`

- Stack: `s1 s2 -- f`
- Compare two NUL-terminated strings for equality.

## Core stack words

These are the everyday stack helpers added by the boot vocabulary.

### `rdrop`

- Stack: `--`
- Drop the top item from the return stack.

### `tdrop`

- Stack: `--`
- Drop the top item from the temporary stack.

### `tuck`

- Stack: `a b -- b a b`
- Copy the second item under the first.

### `nip`

- Stack: `a b -- b`
- Remove the second item.

### `?dup`

- Stack: `n -- n n | 0`
- Duplicate only if nonzero.

### `2+`

- Stack: `n -- n'`
- Add 2.

### `2dup`

- Stack: `a b -- a b a b`
- Duplicate the top two values.

### `2drop`

- Stack: `a b --`
- Drop the top two values.

### `-rot`

- Stack: `a b c -- c a b`
- Rotate the top three values.

### `0<`, `<=`, `>=`

- Comparison helpers.

### `type`

- Stack: `a n --`
- Emit `n` characters starting at address `a`.

### `ascii?`

- Stack: `c -- f`
- True if `c` is printable ASCII.

### `com`

- Stack: `n -- n'`
- Bitwise ones complement.

### `negate`

- Stack: `n -- n'`
- Two’s-complement negation.

### `abs`

- Stack: `n -- n1`
- Absolute value.

## Numeric output and formatting

These words handle formatting, printing, and number conversion.

### `timer`

- Stack: `-- n`
- Returns the current timer tick count.

### `ms`

- Stack: `n --`
- Delay for `n` milliseconds.

### `cr`

- Stack: `--`
- Print carriage-return + newline.

### `tab`

- Stack: `--`
- Print a tab character.

### `space`

- Stack: `--`
- Print a space.

### `spaces`

- Stack: `n --`
- Print `n` spaces.

### `/`

- Stack: `a b -- q`
- Integer division.

### `mod`

- Stack: `a b -- r`
- Modulo.

### `*/`

- Stack: `n m q -- n'`
- Multiply then divide.

### `decimal`, `hex`, `binary`

- Stack: `--`
- Set the numeric base.

### `?neg`

- Stack: `n -- n'`
- Return the absolute value and store the sign in the internal sign flag.

### `hold`

- Stack: `c --`
- Push a character into the numeric print buffer.

### `#.`

- Stack: `--`
- Add a decimal point to the formatted string buffer.

### `#n`

- Stack: `n --`
- Convert a digit to text and store it.

### `#`

- Stack: `n -- m`
- Divide by the current base and append a digit.

### `#s`

- Stack: `n -- 0`
- Repeatedly convert digits until zero.

### `<#`

- Stack: `n -- n'`
- Start numeric conversion state.

### `#>`

- Stack: `n -- a`
- Finish numeric conversion and return the buffer address.

### `(.)`

- Stack: `n --`
- Print a number in the current base.

### `.`

- Stack: `n --`
- Print a number, then a space.

### `.hex`, `.bin`, `.dec`

- Stack: `n --`
- Print using hexadecimal, binary, or decimal formatting.

### `.s`

- Stack: `--`
- Print the current data-stack contents.

### `.word`

- Stack: `de --`
- Print the name of a dictionary entry.

### `words`

- Stack: `--`
- Print a list of dictionary words.

### `words-n`

- Stack: `n --`
- Print the most recently defined `n` words.

### `accept`

- Stack: `addr sz -- len`
- Read a line of input into a buffer and return the length.

## Memory and string inspection

### `pad`

- Stack: `-- a`
- Returns a scratch area pointer.

### `fill`

- Stack: `a num ch --`
- Fill memory with a byte pattern.

### `dump`

- Stack: `addr num --`
- Dump memory as hex bytes.

### `.c`

- Stack: `c --`
- Print a character, or a dot if it is non-printable.

### `t1`

- Stack: `addr --`
- Dump a block of 16 bytes using `.c`.

## System bootstrap and dictionary reset

### `[[`

- Immediate word.
- Saves the current code pointer and variable pointer and enters compile mode.

### `]]`

- Immediate word.
- Restores the saved state and resets compile mode.

### `marker`

- Stack: `--`
- Saves the current dictionary and variable state so it can be restored later.

### `forget`

- Stack: `--`
- Restores the dictionary state to the marker.

## Build-time startup output

The final block in `boot.f` prints a startup banner:

- `.si` prints the project banner
- `.f" \n\nHello."` prints the welcome message

This boot banner is the last thing executed during initialization.

## Source file relationship

The relationship is:

- `boot.f` defines the high-level boot vocabulary
- `fwc` translates it to generated C code in `boot.h`
- `boot.h` is included by the kernel build
- the VM executes the compiled words from the generated source
- alternatively, you can edit the `boot.h` file directly without needing `fwc`

## Summary

The boot vocabulary contains the practical programming language for the OS: control flow, variables, strings, printer helpers, memory access, and startup code.

It sits above the VM primitives and is the layer users actually write against when extending the system.
