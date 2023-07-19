/**
 * @file heap.h
 * @brief Heap management interface.
 *
 * This file declares the interface for the heap management functions.
 * These functions include memory allocation, memory deallocation,
 * heap initialization, and block address calculation, etc.
 * This header file is intended to be used by kernel modules
 * that need direct control over heap management.
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
 * Defines a type for heap block table entries.
 */
typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

/**
 * @struct heap_table
 * @brief Defines the logical structure of the heap table. (Allocation)
 * The heap table is loaded at 32KB in memory, and is 25,600 bytes in size.
 * Each index in the table represents a 4096B block in the heap. 0x00 means
 * free, 0x01 means taken, 0x80 means has next, 0x40 means block is first in
 * allocation series.
 * @see kheap.c
 * @note Allocated in the .bss section
 */
struct heap_table
{
  HEAP_BLOCK_TABLE_ENTRY *entries;
  size_t total;
};

/**
 * @struct heap
 * @brief Defines the physical structure of the heap.
 * The heap is loaded at 16MB and is a contiguous 100MB block of memory,
 * growing upwards towards 116MB. The heap data structure contains a pointer to
 * the heap table data structure, and a void pointer the start address of the
 * heap physical memory.
 * @see kheap.c
 * @note Allocated in the .bss section
 */
struct heap
{
  struct heap_table *table;
  void *saddr;
};

int heap_create (struct heap *heap, void *ptr, void *end,
                 struct heap_table *table);

void *heap_malloc (struct heap *heap, size_t size);

void heap_free (struct heap *heap, void *ptr);

#endif