#include "heap.h"
#include "kernel.h"
#include "memory/memory.h"
#include "status.h"
#include <stdbool.h>

/**
 * @brief Check heap table validity.
 * Called by heap_create() to check if the heap table is valid. Calculates the
 * total number of blocks in the heap and compares it with the total number of
 * blocks in the heap table. If they are not equal, returns -EINVARG. Otherwise,
 * returns 0. 
 * @param ptr Pointer to the start of the heap.
 * @param end Pointer to the end of the heap.
 * @param table Pointer to the heap table.
 * @return int 0 if the heap table is valid, -EINVARG otherwise.
 * @see heap_create() 
 */
static int
heap_check_table (void *ptr, void *end, struct heap_table *table)
{
  int res = 0;
  size_t table_size = (size_t)(end - ptr);
  size_t total_blocks = table_size / LAMEOS_HEAP_BLOCK_SIZE;
  if (table->total != total_blocks)
    {
      res = -EINVARG;
      goto out;
    }
out:
  return res;
}

/**
 * @brief Check pointer for alignment to the heap block size.
 * Called by heap_create() to check if the pointer is aligned to the heap block
 * size. If it is not, returns false. Otherwise, returns true.
 * @param ptr the pointer to check for alignment
 * @return true if the pointer is aligned to the heap block size.
 * @return false if the pointer is not aligned to the heap block size.
 * @see heap_create().
 */
static bool
heap_check_alignment (void *ptr)
{
  return ((unsigned int)ptr % LAMEOS_HEAP_BLOCK_SIZE) == 0;
}

/**
 * @brief Create a heap object.
 * Called by kheap_init() to create a heap object. Checks if the start and end
 * pointers are aligned to the heap block size. If they are not, returns
 * -EINVARG. Otherwise, initializes the heap object and the heap table returning
 * 0.
 * @param heap The heap object to initialize.
 * @param ptr The start of the heap. 
 * @param end The end of the heap.
 * @param table The heap table.
 * @return int 0 if the heap object was initialized successfully, -EINVARG
 * otherwise.
 * @see kheap_init().
 */
int
heap_create (struct heap *heap, void *ptr, void *end, struct heap_table *table)
{
  int res = 0;
  if (!heap_check_alignment (ptr) || !heap_check_alignment (end))
    {
      res = -EINVARG;
      goto out;
    }

  memset (heap, 0, sizeof (struct heap));
  heap->saddr = ptr;
  heap->table = table;

  res = heap_check_table (ptr, end, table);
  if (res < 0)
    {
      goto out;
    }

  size_t table_size = sizeof (HEAP_BLOCK_TABLE_ENTRY) * table->total;
  memset (table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
  return res;
}

/**
 * @brief Rounds end pointer up to the next heap block size.
 * Called by heap_malloc() to round the end pointer up to the next heap block
 * size. If the end pointer is already aligned to the heap block size, returns
 * the end pointer. Otherwise, returns the end pointer rounded up to the next
 * heap block size.
 * @param val The end pointer to round up, if necessary.
 * @return uint32_t The end pointer aligned to the heap block size.
 * @see heap_malloc().
 */
static uint32_t
heap_align_value_to_upper (uint32_t val)
{
  if ((val % LAMEOS_HEAP_BLOCK_SIZE) == 0)
    {
      return val;
    }

  val = (val - (val % LAMEOS_HEAP_BLOCK_SIZE));
  val += LAMEOS_HEAP_BLOCK_SIZE;
  return val;
}

/**
 * @brief Helper function returning type of heap block table entry.
 * Returns the lower 4 bits of the heap block table entry by performing logical
 * AND with 0x0f. Possible values are 0x00 for free and 0x01 for taken.
 * @param entry Address of the heap block table entry.
 * @return int 0 if entry is free, 1 if entry is taken.
 * @see heap_get_start_block().
 */
static int
heap_get_entry_type (HEAP_BLOCK_TABLE_ENTRY entry)
{
  return entry & 0x0f;
}

/**
 * @brief Function returning index of the first, sufficiently large, free block.
 * Called by heap_malloc_blocks(). Iterates over the heap block table entries
 * and tests if the entry is free. If it is, increments the counter. If the
 * counter is equal to the number of blocks requested, returns the index of the
 * first block. Otherwise, returns -ENOMEM.
 * @param heap The heap object to allocate on.
 * @param total_blocks The requested number of blocks to allocate.
 * @return int Index of the first block if the allocation was successful,
 * -ENOMEM otherwise.
 * @see heap_malloc_blocks().
 */
int
heap_get_start_block (struct heap *heap, uint32_t total_blocks)
{
  struct heap_table *table = heap->table;
  int bc = 0;
  int bs = -1;

  for (size_t i = 0; i < table->total; i++)
    {
      if (heap_get_entry_type (table->entries[i])
          != HEAP_BLOCK_TABLE_ENTRY_FREE)
        {
          bc = 0;
          bs = -1;
          continue;
        }
      if (bs == -1)
        {
          bs = i;
        }
      bc++;
      if (bc == total_blocks)
        {
          break;
        }
    }
  if (bs == -1)
    {
      return -ENOMEM;
    }
  return bs;
}

/**
 * @brief Helper function returning the absolute address of the block.
 * Takes the start address of the heap and adds the block index multiplied by
 * the heap block size. Returns the absolute address of the block. Called by
 * heap_malloc_blocks().
 * @param heap The heap object whose start address is used.
 * @param block The index of the block whose address is returned.
 * @return void* The absolute address of the block.
 * @see heap_malloc_blocks().
 */
void *
heap_block_to_address (struct heap *heap, uint32_t block)
{
  return heap->saddr + (block * LAMEOS_HEAP_BLOCK_SIZE);
}

/**
 * @brief 
 * 
 * @param heap 
 * @param start_block 
 * @param total_blocks 
 */
void
heap_mark_blocks_taken (struct heap *heap, int start_block, int total_blocks)
{
  int end_block = (start_block + total_blocks) - 1;

  HEAP_BLOCK_TABLE_ENTRY entry
      = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
  if (total_blocks > 1)
    {
      entry |= HEAP_BLOCK_HAS_NEXT;
    }

  for (int i = start_block; i <= end_block; i++)
    {
      heap->table->entries[i] = entry;
      entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
      if (i != end_block - 1)
        {
          entry |= HEAP_BLOCK_HAS_NEXT;
        }
    }
}

/**
 * @brief 
 * 
 * @param heap 
 * @param total_blocks 
 * @return void* 
 */
void *
heap_malloc_blocks (struct heap *heap, uint32_t total_blocks)
{
  void *address = 0;

  int start_block = heap_get_start_block (heap, total_blocks);
  if (start_block < 0)
    {
      goto out;
    }

  address = heap_block_to_address (heap, start_block);

  // Mark the blocks as taken
  heap_mark_blocks_taken (heap, start_block, total_blocks);

out:
  return address;
}

/**
 * @brief 
 * 
 * @param heap 
 * @param start_block 
 */
void
heap_mark_blocks_free (struct heap *heap, int start_block)
{
  struct heap_table *table = heap->table;
  for (int i = start_block; i < (int)table->total; i++)
    {
      HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
      table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
      if (!(entry & HEAP_BLOCK_HAS_NEXT))
        {
          break;
        }
    }
}

/**
 * @brief 
 * 
 * @param heap 
 * @param address 
 * @return int 
 */
int
heap_address_to_block (struct heap *heap, void *address)
{
  return ((int)(address - heap->saddr)) / LAMEOS_HEAP_BLOCK_SIZE;
}

/**
 * @brief 
 * 
 * @param heap 
 * @param size 
 * @return void* 
 */
void *
heap_malloc (struct heap *heap, size_t size)
{
  size_t aligned_size = heap_align_value_to_upper (size);
  uint32_t total_blocks = aligned_size / LAMEOS_HEAP_BLOCK_SIZE;
  return heap_malloc_blocks (heap, total_blocks);
}

/**
 * @brief 
 * 
 * @param heap 
 * @param ptr 
 */
void
heap_free (struct heap *heap, void *ptr)
{
  heap_mark_blocks_free (heap, heap_address_to_block (heap, ptr));
}
