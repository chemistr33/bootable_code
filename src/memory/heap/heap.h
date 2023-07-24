/**
 * @file heap.h
 * @brief Heap management interface constants, typedefs, structs, and function
 * prototypes.
 */
#ifndef HEAP_H
#define HEAP_H
#include "../../config.h"
#include <stddef.h>
#include <stdint.h>

/**
 * @def HEAP_BLOCK_TABLE_ENTRY_TAKEN
 * Represents a block entry that is currently occupied.
 */
#define HEAP_BLOCK_TABLE_ENTRY_TAKEN 0x01

/**
 * @def HEAP_BLOCK_TABLE_ENTRY_FREE
 * Represents a block entry that is currently free.
 */
#define HEAP_BLOCK_TABLE_ENTRY_FREE 0x00

/**
 * @def HEAP_BLOCK_HAS_NEXT
 * Bitmask indicating that the current block has a subsequent block. This is
 * 128 decimal, or 0x80 hexadecimal.
 */
#define HEAP_BLOCK_HAS_NEXT 0b10000000

/**
 * @def HEAP_BLOCK_IS_FIRST
 * Bitmask indicating that the block is the first in a series of blocks. This
 * is 64 decimal, or 0x40 hexadecimal.
 */
#define HEAP_BLOCK_IS_FIRST 0b01000000

/**
 * @typedef HEAP_BLOCK_TABLE_ENTRY
 * Defines a typedef for heap block table entries as unsigned chars (1 byte).
 */
typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

/**
 * @struct heap_table
 * @brief Defines the allocation table and available memory blocks of the heap.
 * Loaded into memory at 32KB. 
 * @details Possible entry values:
 *  0xC1 (193) - Taken, First, Has-Next
 *  0x81 (129) - Taken, Has-Next
 *  0x01   (1) - Taken (Implicit Last)
 *  0x00   (0) - Free
 * @see kernel_heap_table
 * @note Allocated in the .bss section
 */
struct heap_table
{
  HEAP_BLOCK_TABLE_ENTRY *entries; /**Pointer to the entries array.*/
  size_t total; /**Total blocks in the heap, initialized later to 25600.*/
};

/**
 * @struct heap
 * @brief Defines the physical structure of the heap. Contains a pointer to its
 * heap table and the starting address of the physical heap memory.
 * @see kernel_heap
 * @note Allocated in the .bss section
 */
struct heap
{
  struct heap_table *table; /**Pointer to the heap table.*/
  void *saddr;
};

int heap_create (struct heap *heap, void *ptr, void *end,
                 struct heap_table *table);

void *heap_malloc (struct heap *heap, size_t size);

void heap_free (struct heap *heap, void *ptr);

#endif