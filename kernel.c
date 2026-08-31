/*
 * Bare Metal OS Kernel
 * A minimal kernel for QEMU that runs pure C code
 */

#include "dwc-vm.h"

/* Multiboot1 Header Definitions */
#define MULTIBOOT_MAGIC       0x1BADB002
#define MULTIBOOT_ALIGN       (1 << 2)
#define MULTIBOOT_MEMINFO     (1 << 1)
#define MULTIBOOT_FLAGS       (MULTIBOOT_ALIGN | MULTIBOOT_MEMINFO)
#define MULTIBOOT_CHECKSUM    (-(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS))

/* Multiboot header structure */
struct multiboot_header {
    uint32_t magic;
    uint32_t flags;
    uint32_t checksum;
} __attribute__((packed));

/* Place the Multiboot header in a special section that must come early */
__attribute__((section(".multiboot"))) __attribute__((aligned(4)))
const struct multiboot_header multiboot_header = {
    .magic = MULTIBOOT_MAGIC,
    .flags = MULTIBOOT_FLAGS,
    .checksum = MULTIBOOT_CHECKSUM,
};

/* VGA text mode constants */
#define VGA_MEMORY 0xB8000
#define VGA_COLOR_WHITE_ON_BLACK 0x0F

/* GDT and IDT constants */
#define GDT_SIZE 3
#define IDT_SIZE 256

/* PIC ports */
#define PIC_MASTER_CMD 0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD 0xA0
#define PIC_SLAVE_DATA 0xA1

/* ATA/IDE primary channel (PIO mode) */
#define ATA_PRIMARY_IO_BASE   0x1F0
#define ATA_PRIMARY_CTRL_BASE 0x3F6
#define ATA_SECTOR_SIZE      512
#define ATA_POLL_TIMEOUT     100000

/* ICW initialization command words */
#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x04
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

/*VGA cursor position and text color */
int cursor_x = 0;
int cursor_y = 0;
int text_color = VGA_COLOR_WHITE_ON_BLACK;

/* Keyboard state */
#define KEYBOARD_BUFFER_SIZE 256
static char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
int kbd_head = 0;
int kbd_tail = 0;
int shift_pressed = 0;
int ctrl_pressed = 0;

/* Timer tick counter */
volatile uint32_t sys_ticks = 0;

/* Helper function to write a byte to port */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Helper function to read a byte from port */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Helper function to write a word to port */
static inline void outw(uint16_t port, uint16_t val) {
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* Helper function to read a word from port */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Enable interrupts */
static inline void sti(void) {
    asm volatile("sti");
}

/* Disable interrupts */
static inline void cli(void) {
    asm volatile("cli");
}

/* Load GDT */
static inline void lgdt(void *gdt_ptr) {
    asm volatile("lgdt (%0)" : : "r"(gdt_ptr));
}

/* Load IDT */
static inline void lidt(void *idt_ptr) {
    asm volatile("lidt (%0)" : : "r"(idt_ptr));
}

/* Forward declarations for VGA functions */
void vga_set_cursor(int x, int y);

/* GDT Entry */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

/* GDT Pointer */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* IDT Entry */
struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed));

/* IDT Pointer */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/* GDT and IDT storage */
static struct gdt_entry gdt[GDT_SIZE];
static struct idt_entry idt[IDT_SIZE];
static struct gdt_ptr gdt_ptr_val;
static struct idt_ptr idt_ptr_val;

/* Initialize GDT (Global Descriptor Table) */
void gdt_init(void) {
    /* Null descriptor */
    gdt[0].limit_low = 0;
    gdt[0].base_low = 0;
    gdt[0].base_mid = 0;
    gdt[0].access = 0;
    gdt[0].granularity = 0;
    gdt[0].base_high = 0;
    
    /* Code descriptor (kernel mode) */
    gdt[1].limit_low = 0xFFFF;
    gdt[1].base_low = 0;
    gdt[1].base_mid = 0;
    gdt[1].access = 0x9A;      /* Present, Ring 0, Code */
    gdt[1].granularity = 0xCF; /* 4K granularity, 32-bit */
    gdt[1].base_high = 0;
    
    /* Data descriptor (kernel mode) */
    gdt[2].limit_low = 0xFFFF;
    gdt[2].base_low = 0;
    gdt[2].base_mid = 0;
    gdt[2].access = 0x92;      /* Present, Ring 0, Data */
    gdt[2].granularity = 0xCF; /* 4K granularity, 32-bit */
    gdt[2].base_high = 0;
    
    /* Set GDT pointer */
    gdt_ptr_val.limit = (sizeof(struct gdt_entry) * GDT_SIZE) - 1;
    gdt_ptr_val.base = (uint32_t)&gdt[0];
    
    /* Load GDT */
    lgdt(&gdt_ptr_val);
    
    /* Reload segment registers */
    asm volatile(
        "ljmp $0x08, $1f\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        : : : "eax"
    );
}

/* Keyboard scan code to ASCII conversion table */
static const char scancode_to_ascii[] = {
     0 ,  27, '1', '2', '3', '4', '5', '6',  '7', '8', '9',  '0', '-', '=',   8,   9,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',  'o', 'p', '[',  ']',  10,  0 , 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0 , '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/',  0 , '*',   0 , ' ',  0 ,   0 ,  0 ,  0 ,  0 ,  0 ,
     0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '7',  '8', '9', '-',  '4', '5', '6', '+', '1',
    '2', '3', '0', '.',  0 ,  0 ,  0 ,  0 ,   0 ,  0 ,  0 ,   0 ,  0 ,  0 ,  0 ,  0 ,
     0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,   0 ,  0 ,  0 ,   0 ,  0 ,  0 ,  0 ,   0
};

/* Translate a scancode to a character, honoring Shift and Ctrl state. */
static char keyboard_translate_scancode(uint8_t scancode) {
    if (scancode >= sizeof(scancode_to_ascii)) {
        return 0;
    }

    char c = scancode_to_ascii[scancode];

    if (ctrl_pressed) {
        if (c >= 'a' && c <= 'z') {
            return c - 'a' + 1;
        }
        if (c >= 'A' && c <= 'Z') {
            return c - 'A' + 1;
        }
        if (c == '[') return 27;
        if (c == ']') return 29;
        if (c == '\\') return 28;
        if (c == ';') return 29;
        if (c == '\'') return 0;
    }

    if (c == 0 || !shift_pressed) {
        return c;
    }

    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }

    if (c >= '0' && c <= '9') {
        static const char shifted_digits[] = {
            ')', '!', '@', '#', '$', '%', '^', '&', '*', '('
        };
        return shifted_digits[c - '0'];
    }

    switch (c) {
        case '`': return '~';
        case '-': return '_';
        case '=': return '+';
        case '[': return '{';
        case ']': return '}';
        case '\\': return '|';
        case ';': return ':';
        case '\'': return '"';
        case ',': return '<';
        case '.': return '>';
        case '/': return '?';
        default: return c;
    }
}

/* Timer interrupt handler */
void __attribute__((interrupt)) timer_handler(void *frame) {
    (void)frame;
    sys_ticks += 20;  /* Increment ticks by 20 for 50Hz */
    asm volatile("outb %0, %1" : : "a"((uint8_t)0x20), "Nd"(PIC_MASTER_CMD));
}

/* Keyboard interrupt handler */
void __attribute__((interrupt)) keyboard_handler(void *frame) {
    (void)frame;  /* Unused parameter - the interrupt frame */

    uint8_t scancode;
    asm volatile("inb $0x60, %0" : "=a"(scancode));

    int next = (kbd_head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next != kbd_tail) {
        keyboard_buffer[kbd_head] = (char)scancode;
        kbd_head = next;
    }

    asm volatile("outb %0, %1" : : "a"((uint8_t)0x20), "Nd"(PIC_MASTER_CMD));
}

/* Initialize the programmable interval timer (PIT) */
void pit_init(uint32_t hz) {
    uint32_t divisor = 1193182 / hz;
    if (divisor == 0) {
        divisor = 1;
    }

    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);

    uint8_t mask = inb(PIC_MASTER_DATA);
    mask &= ~0x01;
    outb(PIC_MASTER_DATA, mask);
}

/* Initialize IDT (Interrupt Descriptor Table) */
void idt_init(void) {
    /* Clear all IDT entries */
    for (int i = 0; i < IDT_SIZE; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }
    
    /* Set IDT pointer */
    idt_ptr_val.limit = (sizeof(struct idt_entry) * IDT_SIZE) - 1;
    idt_ptr_val.base = (uint32_t)&idt[0];
    
    /* Load IDT */
    lidt(&idt_ptr_val);
}

/* Register interrupt handler */
void register_interrupt_handler(uint8_t vector, void (*handler)(void)) {
    uint32_t handler_addr = (uint32_t)handler;
    
    idt[vector].offset_low = handler_addr & 0xFFFF;
    idt[vector].offset_high = (handler_addr >> 16) & 0xFFFF;
    idt[vector].selector = 0x08;           /* Code segment in GDT */
    idt[vector].zero = 0;
    idt[vector].type_attr = 0x8E;          /* Present, Ring 0, Interrupt Gate */
}

/* Initialize PIC (Programmable Interrupt Controller) */
void pic_init(void) {
    /* ICW1: initialize */
    outb(PIC_MASTER_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC_SLAVE_CMD, ICW1_INIT | ICW1_ICW4);
    
    /* ICW2: interrupt vector offsets */
    outb(PIC_MASTER_DATA, 0x20);  /* Master PIC vectors: 0x20-0x27 */
    outb(PIC_SLAVE_DATA, 0x28);   /* Slave PIC vectors: 0x28-0x2F */
    
    /* ICW3: cascade mode */
    outb(PIC_MASTER_DATA, 0x04);  /* Master has slave on IRQ2 */
    outb(PIC_SLAVE_DATA, 0x02);   /* Slave is on Master's IRQ2 */
    
    /* ICW4: environment */
    outb(PIC_MASTER_DATA, ICW4_8086);
    outb(PIC_SLAVE_DATA, ICW4_8086);
    
    /* Mask all interrupts initially */
    outb(PIC_MASTER_DATA, 0xFF);
    outb(PIC_SLAVE_DATA, 0xFF);
    
    /* Unmask IRQ0 (timer) and IRQ1 (keyboard) on master PIC */
    uint8_t mask = inb(PIC_MASTER_DATA);
    mask &= ~0x03;  /* Clear bits 0 and 1 */
    outb(PIC_MASTER_DATA, mask);
}

/* Read character from keyboard buffer (non-blocking) */
int keyboard_get_char(void) {
    while (kbd_head != kbd_tail) {
        uint8_t scancode = (uint8_t)keyboard_buffer[kbd_tail];
        kbd_tail = (kbd_tail + 1) % KEYBOARD_BUFFER_SIZE;

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            continue;
        }

        if (scancode == 0xAA || scancode == 0xB6) {
            shift_pressed = 0;
            continue;
        }

        if (scancode == 0x1D) {
            ctrl_pressed = 1;
            continue;
        }

        if (scancode == 0x9D) {
            ctrl_pressed = 0;
            continue;
        }

        if ((scancode & 0x80) != 0) {
            continue;
        }

        if (scancode >= sizeof(scancode_to_ascii)) {
            continue;
        }

        char c = keyboard_translate_scancode(scancode);
        if (c != 0) {
            return c;
        }
    }

    return -1;  /* No character available */
}

/* Initialize serial port for debugging */
void serial_init(void) {
    uint16_t port = 0x3F8; /* COM1 */
    
    outb(port + 1, 0x00); /* Disable interrupts */
    outb(port + 3, 0x80); /* Enable divisor latch */
    outb(port + 0, 0x01); /* Set divisor to 1 (115200 baud) */
    outb(port + 1, 0x00);
    outb(port + 3, 0x03); /* 8 bits, no parity, 1 stop bit */
    outb(port + 2, 0xC7); /* Enable FIFO */
    outb(port + 4, 0x0B); /* Set DTR and RTS */
}

/* ATA/IDE PIO helper functions */
static int ata_wait_bsy(void) {
    for (int i = 0; i < ATA_POLL_TIMEOUT; i++) {
        if ((inb(ATA_PRIMARY_IO_BASE + 7) & 0x80) == 0) {
            return 0;
        }
    }
    return -1;
}

static int ata_wait_drq(void) {
    for (int i = 0; i < ATA_POLL_TIMEOUT; i++) {
        uint8_t status = inb(ATA_PRIMARY_IO_BASE + 7);
        if (status & 0x08) { return  0; } // DRQ
        if (status & 0x01) { return -1; } // ERROR
    }
    return -1;
}

/* LBA = “Logical Block Address” */
int ata_read_block(uint32_t LBA, void *buf) {
    if (!buf) { return -1; }
    if (ata_wait_bsy() != 0) { return -1; }

    outb(ATA_PRIMARY_IO_BASE + 6, 0xE0 | ((LBA >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO_BASE + 2, 1); /* sector count */
    outb(ATA_PRIMARY_IO_BASE + 3, (uint8_t)(LBA & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + 4, (uint8_t)((LBA >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + 5, (uint8_t)((LBA >> 16) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + 7, 0x20); /* READ SECTOR(S) */

    if (ata_wait_drq() != 0) { return -1; }

    uint16_t *dst = (uint16_t *)buf;
    for (int i = 0; i < ATA_SECTOR_SIZE / 2; i++) {
        dst[i] = inw(ATA_PRIMARY_IO_BASE);
    }

    return 0;
}

/* LBA = “Logical Block Address” */
int ata_write_block(uint32_t LBA, const void *buf) {
    if (!buf) { return -1; }
    if (ata_wait_bsy() != 0) { return -1; }

    outb(ATA_PRIMARY_IO_BASE + 6, 0xE0 | ((LBA >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO_BASE + 2, 1); /* sector count */
    outb(ATA_PRIMARY_IO_BASE + 3, (uint8_t)(LBA & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + 4, (uint8_t)((LBA >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + 5, (uint8_t)((LBA >> 16) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + 7, 0x30); /* WRITE SECTOR(S) */

    if (ata_wait_drq() != 0) { return -1; }

    const uint16_t *src = (const uint16_t *)buf;
    for (int i = 0; i < ATA_SECTOR_SIZE / 2; i++) {
        outw(ATA_PRIMARY_IO_BASE, src[i]);
    }

    /* Flush the write pipeline */
    inb(ATA_PRIMARY_IO_BASE + 7);
    return 0;
}

/* Write a character to serial port */
// void serial_emit(char c) {
//     uint16_t port = 0x3F8; /* COM1 */
//     
//     /* Wait for transmit buffer to be empty */
//     while ((inb(port + 5) & 0x20) == 0);
//     
//     outb(port, c);
// }

/* Write a string to serial port */
// void serial_zType(const char *str) {
//     for (int i = 0; str[i] != '\0'; i++) {
//         serial_emit(str[i]);
//         if (str[i] == '\n') {
//             serial_emit('\r');
//         }
//     }
// }

/* Set cursor position in VGA text mode */
void vga_set_cursor(int x, int y) {
    uint16_t pos = y * VGA_COLS + x;
    
    outb(0x3D4, 0x0F); /* Cursor position low byte */
    outb(0x3D5, pos & 0xFF);
    
    outb(0x3D4, 0x0E); /* Cursor position high byte */
    outb(0x3D5, (pos >> 8) & 0xFF);
}

/* Write a character to VGA text mode */
void emit(char c) {
    uint16_t *vga = (uint16_t *)VGA_MEMORY;
    uint16_t color = (uint16_t)text_color;
    
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = VGA_COLS - 1;
        }
    } else if (c == '\t') {
        ++cursor_x;
        while ((cursor_x < VGA_COLS) && ((cursor_x % 8) != 0)) {
            ++cursor_x;
        }
    } else {
        vga[cursor_y * VGA_COLS + cursor_x] = ((uint16_t)color << 8) | (uint8_t)c;
        cursor_x++;
    }
    
    if (cursor_x >= VGA_COLS) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_ROWS) {
        for (int y = 1; y < VGA_ROWS; y++) {
            for (int x = 0; x < VGA_COLS; x++) {
                vga[(y - 1) * VGA_COLS + x] = vga[y * VGA_COLS + x];
            }
        }

        for (int x = 0; x < VGA_COLS; x++) {
            vga[(VGA_ROWS - 1) * VGA_COLS + x] = ((uint16_t)color << 8) | ' ';
        }

        cursor_y = VGA_ROWS - 1;
        cursor_x = 0;
    }
    
    vga_set_cursor(cursor_x, cursor_y);
}

/* Write a string to VGA text mode */
void zType(const char *str) {
    while (*str != '\0') { emit(*(str++)); }
}

/* Clear the screen */
void vga_clear(void) {
    uint16_t *vga = (uint16_t *)VGA_MEMORY;
    uint16_t blank = ((uint16_t)VGA_COLOR_WHITE_ON_BLACK << 8) | ' ';
    
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        vga[i] = blank;
    }
    
    cursor_x = 0;
    cursor_y = 0;
    vga_set_cursor(0, 0);
}

/* Main kernel entry point */
void kernel_main(void) {
    /* Initialize hardware */
    vga_clear();
    serial_init();
    gdt_init();
    idt_init();
    pic_init();
    pit_init(50);
    
    /* Register interrupt handlers */
    register_interrupt_handler(32, (void (*)(void))timer_handler);  /* IRQ0 = vector 32 */
    register_interrupt_handler(33, (void (*)(void))keyboard_handler);  /* IRQ1 = vector 33 */
    
    /* Enable interrupts */
    sti();

    dwcRun();  /* Start the Forth-like interpreter */
}
