/**
 * @file heap.c
 * @brief Heap management implementation.
 *
 * This file implements the interface defined in heap.h. It provides
 * functionality for heap management,  memory allocation, memory and memory
 * deallocation, heap initialization, block address calculation, and so forth.
 */
#include "heap.h"
#include "kernel.h"
#include "memory/memory.h"
#include "status.h"
#include <stdbool.h>

/**
 * @brief Checks if the heap table has a valid block count.
 * Calculates the total number of blocks in the heap and compares it with the
 * total number of blocks in the heap table. If the two values are equal, the
 * test passes and returns 0, otherwise it returns -EINVARG.
 *
 * @param ptr Pointer to the start of the heap. It provides the base address
 * for the heap memory.
 * @param end Pointer to the end of the heap. It serves as the boundary of the
 * heap memory.
 * @param table Pointer to the heap table. The heap table keeps track of the
 * total number of blocks in the heap.
 * @return int Returns 0 if the heap table is valid, -EINVARG otherwise.
 * @note -EINVARG is a flag indicating that an invalid argument was
 * encountered.
 * @note -EINVARG = -2
 * @see heap_create()
 */
static int
heap_check_table (void *ptr, void *end, struct heap_table *table)
{
  int res = 0;

  // Calculate total bytes in heap.
  size_t table_size = (size_t)(end - ptr);

  // Calculate total blocks in heap.
  size_t total_blocks = table_size / LAMEOS_HEAP_BLOCK_SIZE;

  // Compare total calculated blocks in heap with total blocks in heap table.
  if (table->total != total_blocks)
    {
      res = -EINVARG;
      goto out;
    }

// return 0 if heap table is valid, -EINVARG otherwise
out:
  return res;
}

/**
 * @brief Validates if a given pointer is correctly aligned to the heap block
 * size.
 * Invoked by heap_create(), this function checks whether the supplied pointer
 * adheres to the alignment requirements of the heap block size. If the
 * pointer, cast to unsigned int, mod 4096 is equal to 0, the pointer is
 * aligned, return true (1). Otherwise, the pointer is not aligned, return
 * false (0).
 *
 * @param ptr The pointer whose alignment is to be verified. It could point to
 * any arbitrary location within the heap.
 * @return true (1) if the pointer is aligned correctly with respect to the
 * heap block size, ensuring that it points to the start of a block.
 * @return false (0) if the pointer is not aligned, indicating it may point to
 * the middle of a block or some other misaligned location.
 * @see heap_create()
 */
static bool
heap_check_alignment (void *ptr)
{
  return ((unsigned int)ptr % LAMEOS_HEAP_BLOCK_SIZE) == 0;
}

/**
 * @brief Initializes a heap object and its corresponding heap table.
 *
 * This function is invoked by kheap_init() to setup a heap object. It conducts
 * a series of validation checks and initializations to ensure the heap is
 * ready for use.
 *
 * Begins by verifying the alignment of the start (ptr) and end
 * pointers of the heap. If either pointer isn't aligned, abort and return
 * -EINVARG.
 *
 * If both pointers are correctly aligned, proceeds with initialization.
 * Wipes the heap object's memory using memset, setting all bytes to zero.
 * Then, it sets the start address of the heap (saddr) and associates the heap
 * object with its heap table.
 *
 * After initializing the heap object, the function validates the heap table by
 * calling heap_check_table().
 *
 * If the heap table is valid, the function then initializes the heap table. It
 * calculates the size of the table in bytes and sets all entries in the heap
 * table to indicate they're free.
 *
 * @param heap The heap object to initialize. This will house all the essential
 * data about the heap.
 * @param ptr The start address of the heap. It must be aligned to the heap
 * block size.
 * @param end The end address of the heap. It also must be aligned to the heap
 * block size.
 * @param table The heap table associated with the heap. It keeps track of the
 * state of each block in the heap.
 * @return int Returns 0 if the heap object and table are successfully
 * initialized. Returns -EINVARG if an alignment or heap table check fails.
 * @see kheap_init()
 */
int
heap_create (struct heap *heap, void *ptr, void *end, struct heap_table *table)
{
  int res = 0;

  //  if ( !0 or !0) return -EINVARG;
  if (!heap_check_alignment (ptr) || !heap_check_alignment (end))
    {
      res = -EINVARG;
      goto out;
    }

  // wipe heap object, set .saddr and .table to `ptr` and `table` respectively.
  memset (heap, 0, sizeof (struct heap));
  heap->saddr = ptr;
  heap->table = table;

  // check if newly minted heap table is valid.
  res = heap_check_table (ptr, end, table);
  if (res < 0)
    {
      goto out;
    }

  // calculate size of heap table in bytes (25.6KB).
  size_t table_size = sizeof (HEAP_BLOCK_TABLE_ENTRY) * table->total;

  // init heap table of heap object, set all entries to 0x00 (entry free).
  memset (table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table_size);

out:
  return res;
}

/**
 * @brief Adjusts a provided value to align with the next upper heap block
 * boundary.
 * @param val The end pointer value to be rounded up to the next heap block
 * boundary. This value should be within the heap memory range.
 * @return uint32_t Returns the end pointer value adjusted to align with the
 * heap block size. This will be the original value if it's already a multiple
 * of the block size, or the next upper block boundary otherwise.
 * @see heap_malloc()
 */
static uint32_t
heap_align_value_to_upper (uint32_t val)
{
  // If val is already aligned, return val.
  if ((val % LAMEOS_HEAP_BLOCK_SIZE) == 0)
    {
      return val;
    }
  // Else, return val rounded up to next upper block boundary.
  val = (val - (val % LAMEOS_HEAP_BLOCK_SIZE));
  val += LAMEOS_HEAP_BLOCK_SIZE;
  return val;
}

/**
 * @brief Retrieves the status of a heap block table entry.
 * Performs bitwise AND operation with 0x0F mask isolating the lower 4 bits of
 * the entry. If the result is 0, the block is free, return 0. If the result is
 * 1, the block is taken, return 1.
 * @param entry The heap table entry to examine. This value corresponds
 * to a single block within the heap.
 * @return int Returns 0 if the heap block is free, or 1 if the block is taken.
 * @see heap_get_start_block()
 */
static int
heap_get_entry_type (HEAP_BLOCK_TABLE_ENTRY entry)
{
  return entry & 0x0f;
}

/**
 * @brief Finds a contiguous sequence of free blocks in the heap.
 * Starts by naming a struct pointer to the global heap table and two ints
 * bc = consecutive free block count, bs = start index of free block sequence.
 *
 * Then iterates over
 *
 * @param heap Pointer to the heap object. This heap contains the block table
 * to search.
 * @param total_blocks The total number of contiguous blocks needed.
 * @return int Returns the start index of the free block sequence if
 * successful. If unable to find sufficient contiguous free blocks, it returns
 * -ENOMEM.
 */
int
heap_get_start_block (struct heap *heap, uint32_t total_blocks)
{
  // make a dummy heap table in local-scope.
  struct heap_table *table = heap->table;

  // bc = consecutive free block count, bs = start index of free block sequence
  int bc = 0;
  int bs = -1;

  //########################### BEGIN LOOP ####################################
  for (size_t i = 0; i < table->total; i++)
    {
      // if entry != 0, reset bc to 0 and bs to -1, and continue.
      if (heap_get_entry_type (table->entries[i])
          != HEAP_BLOCK_TABLE_ENTRY_FREE)
        {
          bc = 0;
          bs = -1;
          continue;
        }

      // if entry == 0 (is free), set bs to i, increment bc.
      if (bs == -1)
        {
          bs = i;
        }

      bc++;

      // if bc == total_blocks you found enough free blocks, break.
      if (bc == total_blocks)
        {
          break;
        }
    }
  //############################# END LOOP ####################################

  if (bs == -1)
    {
      return -ENOMEM;
    }

  return bs;
}

/**
 * @brief Converts a heap block index into its corresponding memory address.
 * @param heap Pointer to the heap object. The base address for the heap memory
 * resides in this structure.
 * @param block The block index within the heap to be translated into a memory
 * address.
 * @return void* The absolute memory address that corresponds to the given
 * block index within the heap.
 * @see heap_malloc_blocks()
 */
void *
heap_block_to_address (struct heap *heap, uint32_t block)
{
  // absolute address = offset + (block index * block_size)
  return heap->saddr + (block * LAMEOS_HEAP_BLOCK_SIZE);
}

/**
 * @brief Marks a range of blocks in the heap as taken.
 * Starts by calculating end block index and preparing a valid first entry
 * value in a sequence or standalone block.
 * @param heap Pointer to the heap object. This heap contains the block table
 * to be updated.
 * @param start_block The index of the first block to be marked as taken.
 * @param total_blocks The total number of contiguous blocks to be marked as
 * taken.
 * @see heap_malloc_blocks()
 * @note possible values for HEAP_BLOCK_TABLE_ENTRY:
 *       -> 0x00 = free block.
 *       -> 0x81 = taken, has next.
 *       -> 0x41 = taken, first in series, could be a standalone block.
 *       -> 0x01 = taken, implicitly last in series.
 */
void
heap_mark_blocks_taken (struct heap *heap, int start_block, int total_blocks)
{
  // Because we start counting arrays from 0, subtract 1 from total...
  int end_block = (start_block + total_blocks) - 1;

  // The first entry is both block-taken and block-first.
  HEAP_BLOCK_TABLE_ENTRY entry
      = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;

  // If >1 blocks, set has-next flag as well...
  if (total_blocks > 1)
    {
      entry |= HEAP_BLOCK_HAS_NEXT;
    }

  // A standalone block is marked first, taken. If >1 blocks, first block is
  // marked first, taken, and has-next.
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
 * @brief Allocates a specified number of blocks in the heap.
 *
 * This function is tasked with finding and allocating a contiguous sequence of
 * free blocks in the heap. The number of blocks required is specified as an
 * input parameter. It follows a three-step process:
 *
 * 1. Locate the start of a sufficient sequence of free blocks: It does so by
 * calling the helper function heap_get_start_block(). This function returns
 * the index of the first block in a sufficient sequence of free blocks.
 *
 * 2. Compute the memory address corresponding to the first block: This step
 * involves calling the helper function heap_block_to_address() with the heap
 * object and the first block index obtained in the previous step.
 *
 * 3. Mark the found blocks as taken: In the final step, the function calls
 * heap_mark_blocks_taken() to mark the blocks as taken in the heap's block
 * table.
 *
 * The function then returns the memory address computed in step 2. This
 * address points to the start of the allocated memory block in the heap.
 *
 * @param heap Pointer to the heap object from which the memory is to be
 * allocated.
 * @param total_blocks The number of contiguous blocks to be allocated.
 * @return void* If the allocation was successful, a pointer to the start of
 * the allocated memory. If not, the function returns NULL.
 */
void *
heap_malloc_blocks (struct heap *heap, uint32_t total_blocks)
{
  void *address = 0;

  // Call heap_get_start_block() to return valid integral block index.
  int start_block = heap_get_start_block (heap, total_blocks);

  // Check if the block index is valid (note -ENOMEM is -3)
  if (start_block < 0)
    {
      goto out;
    }

  // Calculate absolute memory address of given block index.
  address = heap_block_to_address (heap, start_block);

  // Mark the blocks as taken. Taken block value (0x1).
  heap_mark_blocks_taken (heap, start_block, total_blocks);

out:
  return address;
}

/**
 * @brief Marks a block or sequence of blocks in the heap table as free.
 * Starts by naming a struct pointer to the global heap table. Then
 * loops over the heap table entries from `start_block` to a maximum of
 * `table->total` entries. For each entry, it stores 0x00 (free). If the block
 * in question is marked as has-next, it continues looping. If not, it breaks.
 * @param heap
 * @param start_block
 */
void
heap_mark_blocks_free (struct heap *heap, int start_block)
{
  // Name a struct pointer to the global heap table.
  struct heap_table *table = heap->table;

  for (int i = start_block; i < (int)table->total; i++)
    {
      // store table value at index i into 'entry'.
      HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];

      // assign heap table entry at index i to 0x00 (free).
      table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;

      // if block is marked as has-next, loop over.
      if (!(entry & HEAP_BLOCK_HAS_NEXT))
        {
          break;
        }
    }
}

/**
 * @brief Converts a memory address into its corresponding heap block index.
 * @param heap Pointer to the heap object, used for the base address of the
 * heap "heap->saddr".
 * @param address The memory address to be converted into a block index.
 */
int
heap_address_to_block (struct heap *heap, void *address)
{
  // Quotient of the range and 0x1000 (4096) is the block index...
  return ((int)(address - heap->saddr)) / LAMEOS_HEAP_BLOCK_SIZE;
}

/**
 * @brief Allocates a block of memory from the heap.
 *
 * Begins by aligning the requested size to the valid heap block size with the
 * helper function 'heap_align_value_to_upper()'. This aligned size is then
 * divided by the block size to calculate the total number of blocks needed and
 * stored in 'total_blocks'. 'heap_malloc_blocks()' is then called to allocate
 * these blocks from the heap. If successful, 'heap_malloc_blocks()' returns a
 * pointer to the start of the allocated memory. Otherwise, it returns NULL.
 *
 * @param heap Pointer to the heap object from which the memory is to be
 * allocated.
 * @param size The number of bytes to allocate.
 * @return void* If the allocation is successful, a pointer to the allocated
 * memory is returned. If the allocation fails, the return value is NULL.
 * @see heap_malloc_blocks().
 */
void *
heap_malloc (struct heap *heap, size_t size)
{
  // Align size-byte argument to upper block size as bytes.
  size_t aligned_size = heap_align_value_to_upper (size);

  // Convert upper-aligned size in bytes to number of blocks.
  uint32_t total_blocks = aligned_size / LAMEOS_HEAP_BLOCK_SIZE;

  // Call heap_malloc_blocks() to allocate the blocks with &kernel_heap and
  // total_blocks as args.
  return heap_malloc_blocks (heap, total_blocks);
}

/**
 * @brief Frees a block of memory on the heap.
 * Wrapper function for heap_mark_blocks_free().
 *
 * @param heap Pointer to the heap object from which the memory is to be freed.
 * @param ptr The pointer to the memory block(s) to be freed.
 */
void
heap_free (struct heap *heap, void *ptr)
{
  heap_mark_blocks_free (heap, heap_address_to_block (heap, ptr));
}
