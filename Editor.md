# Editor

This file documents the editor logic implemented in boot.fth.

The block editor has a somewhat 'VI like' feel to it.

## Normal mode

Key bindings are handled by `ed-go`:

| Key      | Description |
| :--      | :-- |
| `h`      | left 1 char (also `Backspace`) |
| `j`      | down 1 char |
| `k`      | up 1 char |
| `l`      | right 1 char (also `Space`) |
| `i`      | insert 1 blank at cursor to end of line |
| `I`      | insert 1 blank at cursor to end of buffer |
| `r`      | replace one char |
| `R`      | replace mode until escape |
| `x`      | delete 1 char to end of line |
| `X`      | delete 1 char to end of buffer |
| `D`      | delete the current line |
| `C`      | clear current line |
| `O`      | open line above the current line |
| `P`      | put - replace current line with yank buffer |
| `Y`      | yank - copy the current line |
| `+`      | save and switch to next block |
| `-`      | save and switch to previous block |
| `Enter`  | go to next line, column 0 (ctrl-j) |
| `ctrl-s` | saves the block to disk |
| `ctrl-q` | exits the editor |


## Replace mode

| Key     | Description |
| :--     | :-- |
| `R`     | puts the editor into "Replace mode" |
| `ESC`   | exits Replace Mode |
| `Enter` | go to next line, column 0 (also `ctrl-j`) |
| `BS`    | left 1 char (also `ctrl-h`) |
| 32-126  | printable ascii chars are written to the block |

## Top-level editor entry point

The main editor entry is:

- `edit ( n -- )` loads a block and enters the main loop
- `ed ( -- )` edits the last edited block
