#ifndef __KERNEL_H__
#define __KERNEL_H__

/* Keyboard state shared across kernel and OS support code. */
extern int shift_pressed;
extern int ctrl_pressed;

/* VGA cursor position and text color */
#define VGA_COLS   80
#define VGA_ROWS   25
extern int cursor_x;
extern int cursor_y;
extern int text_color;

extern volatile uint32_t sys_ticks;

/* Keyboard API */
extern int kbd_head;
extern int kbd_tail;
extern int keyboard_get_char(void);

/* Timer API */
void pit_init(uint32_t hz);

/* VGA API */
void emit(char c);
void zType(const char *str);
void vga_clear(void);
void vga_set_cursor(int x, int y);

/* Serial API */
void serial_init(void);
void serial_emit(char c);
void serial_zType(const char *str);

/* ATA/IDE block device API */
int ata_read_block(uint32_t block_number, void *buf);
int ata_write_block(uint32_t block_number, const void *buf);

/* Interrupt registration */
void register_interrupt_handler(uint8_t vector, void (*handler)(void));

#endif /* __KERNEL_H__ */
