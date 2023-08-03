#include "kernel.h"
#include "config.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "fs/file.h"
#include "fs/pparser.h"
#include "gdt/gdt.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "memory/memory.h"
#include "memory/paging/paging.h"
#include "status.h"
#include "string/string.h"
#include "task/process.h"
#include "task/task.h"
#include "task/tss.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Pointer to VGA Framebuffer.
 * The kernel uses the VGA framebuffer to display text on the screen. The
 * framebuffer is located at 0xB8000. The kernel writes to the framebuffer
 * using the term_putchar function.
 */
uint16_t *video_mem = 0;

/**
 * @brief VGA Framebuffer Width.
 * The VGA framebuffer is 80 characters wide.
 */
uint16_t term_row = 0;

/**
 * @brief VGA Framebuffer Height.
 * The VGA framebuffer is 25 characters high.
 */
uint16_t term_col = 0;

/**
 * @brief Decodes a character and color into a uint16_t.
 * The VGA framebuffer is a 2D array of uint16_t. Each uint16_t represents a
 * character and its color. The first 8 bits of the uint16_t are the character
 * and the last 8 bits are the color.
 * @param c The character to display.
 * @param color The color of the character.
 * @return uint16_t The character and color encoded into a uint16_t.
 */
uint16_t
term_make_char (char c, char color)
{
  return ((color << 8) | c);
}

/**
 * @brief Writes a character to the VGA framebuffer.
 * This function writes a character and color, given by c and color, to the VGA
 * framebuffer at the specified location, given by x and y. The function first
 * converts the x and y to a 1D index, then writes the character and color to
 * the framebuffer at that index.
 * @param x The x coordinate, column, range 0-79.
 * @param y The y coordinate, row, range 0-24.
 * @param c The character to display, range 0-255.
 * @param color The color of the character, range 0-15.
 */
void
term_putchar (int x, int y, char c, char color)
{
  video_mem[(y * VGA_WIDTH) + x] = term_make_char (c, color);
}

/**
 * @brief Initializes the VGA framebuffer.
 * This function initializes the VGA framebuffer by clearing the screen and
 * setting the video_mem pointer to 0xB8000. The screen is cleared by calling
 * term_putchar with space characters and a black background on position in the
 * framebuffer.
 * @note sets term_row and term_col to 0. Useful for related functions.
 */
void
term_initialize ()
{
  term_row = 0;
  term_col = 0;
  video_mem = (uint16_t *)(0xB8000);
  for (int y = 0; y < VGA_HEIGHT; y++)
    {
      for (int x = 0; x < VGA_WIDTH; x++)
        {
          term_putchar (x, y, ' ', 0);
        }
    }
}

/**
 * @brief Writes a character, advancing cursor, newline if necessary.
 * Writes a character, advancing the cursor. If the cursor is at the end of the
 * line, the cursor is moved to the next line.
 * @param c The character to write.
 * @param color The color of the character.
 */
void
term_writechar (char c, char color)
{
  // newline implementation
  if (c == '\n')
    {
      term_row += 1;
      term_col = 0;
      return;
    }
  term_putchar (term_col, term_row, c, color);
  term_col += 1;
  if (term_col >= VGA_WIDTH)
    {
      term_col = 0;
      term_row += 1;
    }
}

/**
 * @brief Writes a string using term_writechar.
 * This function writes a string by iterating through the string and writing
 * each character using term_writechar to the VGA framebuffer.
 * @param str The string to write.
 */
void
print (const char *str)
{
  size_t len = strlen (str);
  for (int i = 0; i < len; i++)
    {
      term_writechar (str[i], 15);
    }
}

/**
 * @brief This is what LameOS is all about.
 * This function iterates kaleidoscopically through all characters and colors
 * in the VGA framebuffer. It does this forever. EPILEPSY WARNING!
 */
void
lame_color_show ()
{
  volatile char *video_memory = (char *)0xb8000;
forever:
  for (int character = 0; character < 256; ++character)
    { // All Characters
      for (int color = 0; color < 16; ++color)
        { // All Colors
          for (int cell = 0; cell < 80 * 25; ++cell)
            {                                     // All Cells
              video_memory[cell * 2] = character; // Set character
              video_memory[cell * 2 + 1] = color; // Set color
            }
        }
    }
  goto forever;
}

void
panic (const char *msg)
{
  print (msg);
  while (1)
    {
    }
}

static struct paging_4gb_chunk *kernel_chunk = 0;

void
kernel_page ()
{
  kernel_registers ();
  paging_switch (kernel_chunk);
}

struct tss tss;
struct gdt gdt_real[LAMEOS_TOTAL_GDT_SEGMENTS];

struct gdt_structured gdt_structured[LAMEOS_TOTAL_GDT_SEGMENTS] = {
  { .base = 0x00, .limit = 0x00, .type = 0x00 },       // Null segment
  { .base = 0x00, .limit = 0xFFFFFFFF, .type = 0x9A }, // Kernel code segment
  { .base = 0x00, .limit = 0xFFFFFFFF, .type = 0x92 }, // Kernel data segment
  { .base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF8 }, // User code segment
  { .base = 0x00, .limit = 0xFFFFFFFF, .type = 0xF2 }, // User data segment
  { .base = (uint32_t)&tss, .limit = sizeof (tss), .type = 0xE9 }
}; // ^^^^^ TSS segment ^^^^^^

void
kernel_main ()
{
  // Clear BIOS text and print welcome message
  term_initialize ();
  print ("Welcome to LameOS!\n\n--> ");

  memset (gdt_real, 0x00, sizeof (gdt_real));
  gdt_structured_to_gdt (gdt_real, gdt_structured, LAMEOS_TOTAL_GDT_SEGMENTS);

  // Load the GDT
  gdt_load (gdt_real, sizeof (gdt_real));

  // Initialize the heap
  kheap_init ();

  // Initialize filesystems
  fs_init ();

  // Search and initialize the disk
  disk_search_and_init ();

  // Initialize the interrupt descriptor table
  idt_init ();

  // Setup TSS
  memset (&tss, 0x00, sizeof (tss));
  tss.esp0 = 0x600000;
  tss.ss0 = KERNEL_DATA_SELECTOR;

  // Load the TSS
  tss_load (0x28);

  // Setup paging
  kernel_chunk = paging_new_4gb (PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
                                 | PAGING_ACCESS_FROM_ALL);

  // Switch to kernel paging chunk
  paging_switch (kernel_chunk);

  // Enable paging
  enable_paging ();

  struct process *process = 0;
  int res = process_load ("0:/blank.bin", &process);
  if (res != LAMEOS_OK)
    {
      panic ("Failed to load blank.bin!\n");
    }

  task_run_first_ever_task ();

  while (1)
    {
    }
}

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