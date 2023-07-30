#include "kernel.h"
#include "disk/disk.h"
#include "disk/streamer.h"
#include "fs/file.h"
#include "fs/pparser.h"
#include "idt/idt.h"
#include "memory/heap/kheap.h"
#include "memory/paging/paging.h"
#include "string/string.h"
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

static struct paging_4gb_chunk *kernel_chunk = 0;
void
kernel_main ()
{
  // Clear BIOS text and print welcome message
  term_initialize ();
  print ("Welcome to LameOS!\nWork in progress...\n\n--> ");

  // Initialize the heap
  kheap_init ();

  // Initialize filesystems
  fs_init ();

  // Search and initialize the disk
  disk_search_and_init ();

  // Initialize the interrupt descriptor table
  idt_init ();

  // Setup paging
  kernel_chunk = paging_new_4gb (PAGING_IS_WRITEABLE | PAGING_IS_PRESENT
                                 | PAGING_ACCESS_FROM_ALL);

  // Switch to kernel paging chunk
  paging_switch (paging_4gb_chunk_get_directory (kernel_chunk));

  // Enable paging
  enable_paging ();

  // Enable the system interrupts
  enable_interrupts ();

  int fd = fopen("0:/no1.txt", "r");
  if(fd)
  {
    print("\nLameOS opened 'no1.txt'\n\n");
    char buf[61];
    fread(buf, 60, 1, fd);
    print(buf);
  }

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