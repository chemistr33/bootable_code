#ifndef KERNEL_H
#define KERNEL_H
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Macro Constant for VGA framebuffer width
 *
 */
#define VGA_WIDTH 80

/**
 * @brief Macro Constant for VGA framebuffer height
 *
 */
#define VGA_HEIGHT 25

void kernel_main ();
void term_initialize ();
size_t strlen (const char *str);
uint16_t term_make_char (char c, char color);
void term_putchar (int x, int y, char c, char color);
void term_writechar (char c, char color);
void print (const char *str);
void lame_color_show ();

#endif
