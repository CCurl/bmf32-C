#ifndef __KERNEL_H__
#define __KERNEL_H__

/* Keyboard state shared across kernel and OS support code. */
extern int shift_pressed;
extern int ctrl_pressed;

/* VGA cursor position and text color */
extern int cursor_x;
extern int cursor_y;
extern int text_color;

/* Keyboard API */
int keyboard_has_input(void);
int keyboard_getchar(void);
int keyboard_read_char(void);

/* Timer API */
void pit_init(uint32_t hz);
uint32_t get_ticks(void);

/* VGA API */
void emit(char c);
void zType(const char *str);
void vga_clear(void);
void vga_set_cursor(int x, int y);

/* Serial API */
void serial_init(void);
void serial_emit(char c);
void serial_zType(const char *str);

/* Interrupt registration */
void register_interrupt_handler(uint8_t vector, void (*handler)(void));

#endif /* __KERNEL_H__ */
