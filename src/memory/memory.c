#include "memory.h"

/**
 * @brief Generic memset implementation.
 * Takes a void pointer ptr to a memory location, an int c to fill each byte
 * with, and a size_t size to fill to. (size_t is the loop parameter).
 * @param ptr
 * @param c
 * @param size
 * @return void * A pointer to the base address of the memory location after
 * being filled.
 */
void *
memset (void *ptr, int c, size_t size)
{
  char *c_ptr = (char *)ptr;
  for (int i = 0; i < size; i++)
    {
      c_ptr[i] = (char)c;
    }
  return ptr;
}