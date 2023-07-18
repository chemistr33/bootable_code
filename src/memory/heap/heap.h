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
 * Bitmask indicating that the current block has a subsequent block.
 */
#define HEAP_BLOCK_HAS_NEXT 0b10000000

/**
 * @def HEAP_BLOCK_IS_FIRST
 * Bitmask indicating that the block is the first in a series of blocks.
 */
#define HEAP_BLOCK_IS_FIRST 0b01000000

/**
 * @typedef HEAP_BLOCK_TABLE_ENTRY
 * Defines a type for heap block table entries.
 */
typedef unsigned char HEAP_BLOCK_TABLE_ENTRY;

/**
 * @brief Defines the heap table structure.
 *
 * A structure to hold the entire metadata about heap memory. It keeps track
 * of each block of heap memory, represented by HEAP_BLOCK_TABLE_ENTRY,
 * and the total number of blocks available in the heap.
 *
 * @var entries An array of HEAP_BLOCK_TABLE_ENTRY that represents the state of
 * each block in the heap.
 * @var total The total number of blocks available in the heap.
 */
struct heap_table
{
  HEAP_BLOCK_TABLE_ENTRY *entries;
  size_t total;
};

/**
 * @brief Defines the heap structure.
 *
 * A structure that represents a heap memory region. It contains information
 * about the entire heap (the heap_table) and the start address of the heap
 * memory.
 *
 * @var table A pointer to the structure holding the metadata of heap memory.
 * @var saddr The start address of the heap memory.
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