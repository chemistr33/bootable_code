#include <stddef.h>
#include <stdint.h>
#include "idt/idt.h"
#include "kernel.h"

uint16_t *video_mem = 0;
uint16_t term_row = 0;
uint16_t term_col = 0;

uint16_t term_make_char(char c, char color) { return ((color << 8) | c); }

void term_putchar(int x, int y, char c, char color) {
  video_mem[(y * VGA_WIDTH) + x] = term_make_char(c, color);
}
void term_initialize() {
  term_row = 0;
  term_col = 0;
  video_mem = (uint16_t *)(0xB8000);
  for (int y = 0; y < VGA_HEIGHT; y++) {
    for (int x = 0; x < VGA_WIDTH; x++) {
      term_putchar(x, y, ' ', 0);
    }
  }
}

size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len]) {
    len++;
  }
  return len;
}

void term_writechar(char c, char color) {
  // newline implementation
  if (c == '\n') {
    term_row += 1;
    term_col = 0;
    return;
  }
  term_putchar(term_col, term_row, c, color);
  term_col += 1;
  if (term_col >= VGA_WIDTH) {
    term_col = 0;
    term_row += 1;
  }
}

void print(const char *str) {
  size_t len = strlen(str);
  for (int i = 0; i < len; i++) {
    term_writechar(str[i], 15);
  }
}

void kernel_main() {

  //term_initialize();

  // print("Welcome to LameOS!\nHello!\nNow with Newlines!\nGoodbyte and GoodNight.\n");
 volatile char *video_memory = (char *)0xb8000;

  
  forever:
  for (int character = 0; character < 256; ++character) { // All Characters
    for (int color = 0; color < 16; ++color) {            // All Colors
      for (int cell = 0; cell < 80 * 25; ++cell) {        // All Cells
        video_memory[cell * 2] = character;               // Set character
        video_memory[cell * 2 + 1] = color;               // Set color
      }
    }
  }
  goto forever;
}
  // idt_init();


#if 0
  // The OFFICIAL Color Show Comment :) 
  forever:
  for (int character = 0; character < 256; ++character) { // All Characters
    for (int color = 0; color < 16; ++color) {            // All Colors
      for (int cell = 0; cell < 80 * 25; ++cell) {        // All Cells
        video_memory[cell * 2] = character;               // Set character
        video_memory[cell * 2 + 1] = color;               // Set color
      }
    }
  }
  goto forever;
}
#endif