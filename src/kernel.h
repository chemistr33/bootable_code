#ifndef KERNEL_H
#define KERNEL_H

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

void print(const char* str);
void kernel_main();
void term_initialize();

#endif

