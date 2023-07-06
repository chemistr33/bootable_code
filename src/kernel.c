#include "kernel.h"
#include <stddef.h>
#include <stdint.h>

uint16_t *video_mem = 0;

uint16_t term_make_char(char c, char color) { return ((color << 8) | c); }

void term_putchar(int x, int y, char c, char color) {
  video_mem[(y * VGA_WIDTH) + x] = term_make_char(c, color);
}
void term_initialize() {
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

void kernel_main() {

  term_initialize();

  video_mem[0] = 0x0F41;

  for(int y = 0; y<VGA_HEIGHT; y++){
    for(int x = 0; x<VGA_WIDTH; x++){
        if(x==0 && y==0) {continue;}
        if(x==VGA_WIDTH && y==VGA_HEIGHT) {break;}
        term_putchar(x, y, '.', 4);
    }
  }

  video_mem[1999] = term_make_char('Z', 15);
}

/*
 volatile char *video_memory = (char *)0xb8000;

 for (int character = 0; character < 256; ++character) { // All Characters
   for (int color = 0; color < 256; ++color) {           // All Colors
     for (int cell = 0; cell < 80 * 25; ++cell) {        // All Cells
       video_memory[cell * 2] = character;               // Set character
       video_memory[cell * 2 + 1] = color;               // Set color
     }
   }
 }
*/
