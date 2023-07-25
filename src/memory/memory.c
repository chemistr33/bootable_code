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

int
memcmp (void *s1, void *s2, int count)
{
  char *c1 = s1;
  char *c2 = s2;

  while (count-- > 0)
    {
      if (*c1++ != *c2++)
        return c1[-1] < c2[-1] ? -1 : 1;
    }

  return 0;
}