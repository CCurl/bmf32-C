# Bare Metal OS - QEMU

A minimal bare-metal x86 kernel written in pure C and booted under QEMU. It includes a simple text-mode VGA console, serial output, interrupt-driven keyboard input, a PIT-based tick counter, and a Forth VM (not ANSI-Standard).

## Features

- **Pure C kernel**: written without assembly for the main runtime logic
- **Multiboot1 support**: kernel entry is recognized by a multiboot header
- **Single-kernel flat image**: the linked ELF is the OS image for this project
- **VGA text mode**: console output with scrolling support
- **Serial output**: debug messages through COM1
- **Interrupts**: GDT, IDT, PIC, keyboard, and timer handling
- **Keyboard input**: buffered PS/2 keyboard support
- **Tick counter**: PIT-driven 50 Hz timer used by the VM and timing helpers
- **DWC VM**: the Forth interpreter built into the kernel
- **QEMU compatible**: boots directly with the `-kernel` flag or via the generated ISO

## Architecture

```text
block-01.fth    - Translates boot.f -> boot.h using fwc
boot.fth        - Forth source code for the OS
boot.h          - Auto-generated C header file of 'boot.fth'
dwc-vm.c        - Forth-style VM implementation
dwc-vm.h        - VM interface and memory layout declarations
kernel.c        - Kernel core: VGA, serial, PIC, keyboard, timer, IRQ setup
kernel.h        - Extern functions for kernel.c
LICENSE         - MIT license
linker.ld       - Memory layout and ELF placement
Makefile        - Build system
lib.c           - OS/runtime support layer and freestanding compatibility helpers
README.md       - This file
```

## VGA palette

The console uses the standard VGA text-mode attribute byte format:

- low nibble = foreground color
- high nibble = background color

The standard 16 foreground colors are:

| Value | Color |
| --- | --- |
| 0x0 | black |
| 0x1 | blue |
| 0x2 | green |
| 0x3 | cyan |
| 0x4 | red |
| 0x5 | magenta |
| 0x6 | brown |
| 0x7 | light gray |
| 0x8 | dark gray |
| 0x9 | light blue |
| 0xA | light green |
| 0xB | light cyan |
| 0xC | light red |
| 0xD | light magenta |
| 0xE | yellow |
| 0xF | white |

The 8 base background colors are the same values shifted into the high nibble:

| Background value | Color |
| --- | --- |
| 0x00 | black |
| 0x10 | blue |
| 0x20 | green |
| 0x30 | cyan |
| 0x40 | red |
| 0x50 | magenta |
| 0x60 | brown |
| 0x70 | light gray |

The bright background variants follow the same pattern with the high-intensity bit set, e.g. `0xF0` for white background.

The default kernel text color is white-on-black, which is stored as `0x0F` in [kernel.c](kernel.c#L31-L37) and [kernel.c](kernel.c#L61-L65).

## Building

### Prerequisites

You will need fwc, a 32-bit toolchain and QEMU.

fwc is located here: https://github.com/CCurl/fwc

```bash
# Ubuntu/Debian
sudo apt-get install build-essential qemu-system-x86 gcc-multilib grub-pc-bin

# Fedora/RHEL
sudo dnf install gcc gcc-multilib glibc-devel.i686 qemu-system-x86 grub2-tools
```

### Build the kernel

```bash
make
```

This produces `build/kernel.elf`, which is the complete kernel/OS image for this project. There is no separate `boot.bin` in this build flow.

## Running

### Direct boot

```bash
make run
```

This builds the kernel and runs it in QEMU.

### ISO boot

```bash
make iso
make run-iso
```

### Debug with GDB

```bash
make debug
```

Then in another terminal:

```bash
gdb build/kernel.elf
(gdb) target remote :1234
(gdb) continue
```

## Project structure

### kernel.c

The kernel implements:

- **Multiboot header**
- **GDT / IDT initialization**
- **PIC setup**
- **Keyboard interrupt processing**
- **PIT timer tick counting**
- **Serial I/O**
- **VGA text-mode console**
- **kernel_main()** entry point

### dwc-vm.c and dwc-vm.h

This is the Forth VM used by the project. It includes:

- A dictionary and primitive table
- stack operations and compiled words
- VM entry points like `outer()`, `inner()`, and `dwcInit()`
- Primitive hooks for `emit`, `ztype`, and `key`

### lib.c

This file provides the minimal runtime glue needed for a freestanding build, including:

- libc-like string/memory helpers
- Keyboard and timer wrappers used by the VM
- `emit` / `ztype` output support

### Raw disk / block device

The kernel includes a raw ATA/IDE block interface at the hardware boundary. The public API is:

- `int ata_read_block(uint32_t block_number, void *buf);`
- `int ata_write_block(uint32_t block_number, const void *buf);`

This is a simple 512-byte sector API. The block number is a zero-based sector index, and the buffer must be at least 512 bytes. There is no filesystem layer in this driver; it is intentionally a low-level device primitive for higher-level Forth code to build on top of.

### linker.ld

Defines the memory layout:

- The Kernel code starts at `0x100000` (1 MB)
- A flat single-segment kernel image is used

## Important notes

- **No full standard library**: the kernel is built with `-ffreestanding`
- **No dynamic memory**: the project is intentionally minimal
- **Interrupts are active**: GDT/IDT, keyboard, and timer are implemented
- **Timer rate**: the PIT is configured for 50 Hz, so `system_ticks` advances at roughly 20 ms per tick
- **The VM is linked into the kernel**: the build includes [dwc-vm.c](dwc-vm.c)

## Troubleshooting

### QEMU shows nothing

- ensure QEMU is installed
- check that VGA and serial output are enabled
- run `make run` for the direct boot path

### Compiler errors about stdint.h

- install 32-bit development headers:

```bash
sudo apt-get install gcc-multilib
```

### `grub-mkrescue: command not found`

- install GRUB tools:

```bash
sudo apt-get install grub-pc-bin xorriso
```

### QEMU hangs after boot

- This may happen if the kernel is waiting for input or interrupts are not configured properly
- Use `Ctrl+A` then `X` to exit QEMU

## References

- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/)
- [OSDev Wiki](https://wiki.osdev.org/)
- [x86 I/O Ports](https://wiki.osdev.org/I/O_Ports)
- [VGA Text Mode](https://wiki.osdev.org/Text_mode)

## License

Public Domain - use freely for educational purposes.
