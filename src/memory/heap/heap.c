/**
 * @file heap.c
 * @brief Heap management implementation.
 *
 * This file implements the interface defined in heap.h. It provides
 * functionality for heap management, including memory allocation, memory
 * deallocation, heap initialization, block address calculation, and so on. The
 * heap management strategies are used throughout the operating system's
 * kernel, including memory management and task scheduling.
 */
#include "heap.h"
#include "kernel.h"
#include "memory/memory.h"
#include "status.h"
#include <stdbool.h>

/**
 * @brief Checks if the heap table has a valid block count.
 * Called by heap_create(). It compares the number of blocks physically present
 * in the heap with the block count as indicated by the heap table structure.
 * Is a 'sanity check' to ensure that the heap table is valid at the point of
 * heap creation.
 *
 * The function works by first calculating the total size of the heap. This is
 * accomplished by performing pointer arithmetic between the end and start
 * pointers of the heap. It then translates this size into the number of blocks
 * by dividing the total heap size by the size of a single heap block
 * (LAMEOS_HEAP_BLOCK_SIZE). The result of this division yields the total
 * number of blocks in the heap.
 *
 * Next, the function compares the computed total number of blocks with the
 * total number of blocks as stated in the heap table. If the two totals do not
 * match, the function sets the result to -EINVARG, signalling an invalid
 * argument error. Otherwise, the function returns 0, indicating a valid heap
 * table.
 *
 * @param ptr Pointer to the start of the heap. It provides the base address
 * for the heap memory.
 * @param end Pointer to the end of the heap. It serves as the boundary of the
 * heap memory.
 * @param table Pointer to the heap table. The heap table keeps track of the
 * total number of blocks in the heap.
 * @return int Returns 0 if the heap table is valid, -EINVARG otherwise.
 *              -EINVARG is a flag indicating that an invalid argument was
 * encountered.
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
 * @brief Validates if a given pointer is correctly aligned to the heap block
 * size.
 *
 * Invoked by heap_create(), this function checks whether the supplied pointer
 * adheres to the alignment requirements of the heap block size. In a correctly
 * functioning heap, each block of memory must start at an address that is a
 * multiple of the heap block size (LAMEOS_HEAP_BLOCK_SIZE). To determine
 * alignment, the function calculates the modulus of the pointer's value and
 * the heap block size. In memory arithmetic, a pointer that is correctly
 * aligned to a particular block size will have a modulus of zero when its
 * value is divided by the block size. Therefore, if the modulus is zero, the
 * function deems the pointer correctly aligned and returns true. Conversely,
 * if the modulus is non-zero, it denotes misalignment and the function returns
 * false.
 *
 * @param ptr The pointer whose alignment is to be verified. It could point to
 * any arbitrary location within the heap.
 * @return true if the pointer is aligned correctly with respect to the heap
 * block size, ensuring that it points to the start of a block.
 * @return false if the pointer is not aligned, indicating it may point to the
 * middle of a block or some other misaligned location.
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
 * The function begins by verifying the alignment of the start (ptr) and end
 * pointers of the heap. If either pointer is not correctly aligned to the heap
 * block size, the function aborts the heap creation process and returns
 * -EINVARG to signal the alignment error.
 *
 * If both pointers are correctly aligned, the function proceeds to initialize
 * the heap object. It first wipes the heap object's memory using memset,
 * setting all bytes to zero. This ensures a clean, predictable state for the
 * new heap object. Then, it sets the start address of the heap (saddr) and
 * associates the heap object with its heap table.
 *
 * After initializing the heap object, the function validates the heap table by
 * calling heap_check_table(). If this function reports an error (by returning
 * a value less than 0), the function halts the creation process and returns
 * the error code.
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
 * @brief Adjusts a provided value to align with the next upper heap block
 * boundary.
 *
 * Invoked by heap_malloc(), this function ensures that the provided value
 * (val), which represents an end pointer within the heap, is aligned with a
 * heap block boundary. This is critical for maintaining consistency within the
 * heap structure and enabling efficient memory allocation.
 *
 * The alignment process involves checking if the provided value is already a
 * multiple of the heap block size (LAMEOS_HEAP_BLOCK_SIZE). If it is, no
 * adjustment is necessary, and the function simply returns the original value.
 *
 * However, if the value is not a multiple of the block size (i.e., it falls
 * within a block), the function needs to adjust it. It first subtracts the
 * remainder of the value divided by the block size from the value itself. This
 * effectively 'rounds down' the value to the start of the current heap block.
 * Then, it adds the size of a full heap block to this result. The final value
 * is thus rounded up to the start of the next heap block, ensuring alignment
 * with the block boundary.
 *
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
  if ((val % LAMEOS_HEAP_BLOCK_SIZE) == 0)
    {
      return val;
    }

  val = (val - (val % LAMEOS_HEAP_BLOCK_SIZE));
  val += LAMEOS_HEAP_BLOCK_SIZE;
  return val;
}

/**
 * @brief Retrieves the status of a heap block table entry.
 *
 * This utility function extracts the type (status) of a heap block table
 * entry. It performs a bitwise AND operation with the hexadecimal value 0x0F
 * on the provided entry, effectively isolating the lower 4 bits. These bits
 * represent the status of the block.
 *
 * The usage of the lower 4 bits for block status enables efficient storage and
 * retrieval of this information. Possible status values include 0x00 for a
 * free block and 0x01 for a taken (allocated) block.
 *
 * It's important to note that this function interprets the status of a single
 * heap block, not an array or sequence of blocks. It aids in the process of
 * finding, allocating, and freeing blocks within the heap.
 *
 * @param entry The heap block table entry to examine. This value corresponds
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
 *
 * This function is invoked when trying to allocate a chunk of memory from the
 * heap. It scans the heap's block table for a contiguous sequence of free
 * blocks that can accommodate the requested memory size. The total size of the
 * memory request is represented in terms of the number of blocks
 * (total_blocks).
 *
 * The search process starts from the beginning of the heap block table,
 * iterating through each block entry. It maintains a count of consecutive free
 * blocks (bc) and the start index of the first block in this free sequence
 * (bs).
 *
 * For each block, it uses the heap_get_entry_type() function to check if the
 * block is free. If a block is not free, it resets the free block count and
 * start index to start the search anew from the next block. If a block is free
 * and it's the first in a new sequence, the function records its index as the
 * start index.
 *
 * The function keeps incrementing the free block count until it reaches the
 * total required block count or until it encounters a taken block. If it
 * successfully finds a sufficient sequence of free blocks, it returns the
 * start index of this sequence. Otherwise, it returns an -ENOMEM error to
 * indicate insufficient memory in the heap.
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
 * @brief Converts a heap block index into its corresponding memory address.
 *
 * This function assists in the conversion of a relative block index within
 * the heap into an absolute memory address. The function achieves this by
 * taking the start address of the heap and adding the product of the block
 * index and the predefined size of each heap block (LAMEOS_HEAP_BLOCK_SIZE).
 *
 * This form of address calculation is central to the functioning of a heap
 * memory manager, enabling the translation from an abstract block index to a
 * physical memory address that can be utilized for storing and retrieving
 * data. This function is typically invoked during the memory allocation
 * process, where specific blocks within the heap are allocated to meet a
 * requested memory size.
 *
 * It's important to note that this function doesn't check if the block index
 * is within the valid range of the heap or whether the block at the given
 * index is free or allocated.
 *
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
  return heap->saddr + (block * LAMEOS_HEAP_BLOCK_SIZE);
}

/**
 * @brief Marks a range of blocks in the heap as taken.
 *
 * This function is used when allocating memory from the heap. It receives the
 * index of the first block and the total number of blocks to be marked as
 * taken. It then proceeds to mark these blocks in the heap's block table as
 * taken.
 *
 * The marking process involves setting the status of each block entry in the
 * heap's block table. The first block is marked with the
 * HEAP_BLOCK_TABLE_ENTRY_TAKEN and HEAP_BLOCK_IS_FIRST flags. If there are
 * multiple blocks, the first block also receives the HEAP_BLOCK_HAS_NEXT flag
 * to indicate that the allocated sequence of blocks continues in the
 * subsequent block.
 *
 * For the following blocks, the function marks them with the
 * HEAP_BLOCK_TABLE_ENTRY_TAKEN flag. If a block isn't the last in the
 * sequence, it also receives the HEAP_BLOCK_HAS_NEXT flag.
 *
 * @param heap Pointer to the heap object. This heap contains the block table
 * to be updated.
 * @param start_block The index of the first block to be marked as taken.
 * @param total_blocks The total number of contiguous blocks to be marked as
 * taken.
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
 * @brief Allocates a specified number of blocks in the heap.
 *
 * This function is tasked with finding and allocating a contiguous sequence of
 * free blocks in the heap. The number of blocks required is specified as an
 * input parameter. It follows a three-step process:
 *
 * 1. Locate the start of a sufficient sequence of free blocks: It does so by
 * calling the helper function heap_get_start_block(). This function returns
 * the index of the first block in a sufficient sequence of free blocks. If no
 * such sequence exists, the function returns an error code, and
 * heap_malloc_blocks() immediately returns, indicating a failed allocation.
 *
 * 2. Compute the memory address corresponding to the first block: This step
 * involves calling the helper function heap_block_to_address() with the heap
 * object and the first block index obtained in the previous step. This
 * function calculates the memory address corresponding to a given block index
 * by offsetting the heap's start address with the product of the block index
 * and the block size.
 *
 * 3. Mark the found blocks as taken: In the final step, the function calls
 * heap_mark_blocks_taken() to mark the blocks as taken in the heap's block
 * table. This function updates the entries in the block table corresponding to
 * the allocated blocks, setting their status as taken and updating the linking
 * flags accordingly.
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
 * @brief Marks a sequence of blocks in the heap as free.
 *
 * This function is responsible for marking a sequence of blocks in the heap as
 * free. It works with a start block index and continues marking subsequent
 * blocks as free until it hits a block that does not have the
 * 'HEAP_BLOCK_HAS_NEXT' flag set. This indicates the end of a previously
 * allocated sequence of blocks.
 *
 * The function iterates over the heap block table entries starting from the
 * given 'start_block' index. For each block, it retrieves the corresponding
 * table entry and sets it to 'HEAP_BLOCK_TABLE_ENTRY_FREE' indicating that the
 * block is now free. It also checks if the current block was part of a
 * multi-block allocation by examining the 'HEAP_BLOCK_HAS_NEXT' flag. If this
 * flag is not set, it means the end of the sequence has been reached, and the
 * function stops marking blocks as free.
 *
 * @param heap Pointer to the heap object in which the blocks are to be freed.
 * @param start_block The index of the first block in the sequence to be freed.
 */
int
heap_address_to_block (struct heap *heap, void *address)
{
  return ((int)(address - heap->saddr)) / LAMEOS_HEAP_BLOCK_SIZE;
}

/**
 * @brief Allocates a block of memory from the heap.
 *
 * The function begins by aligning the requested size to the heap block size.
 * This is done using the helper function 'heap_align_value_to_upper()'. The
 * alignment ensures that the allocated block of memory will start at an
 * address that is a multiple of 'LAMEOS_HEAP_BLOCK_SIZE', thereby respecting
 * the architecture's memory alignment restrictions. This aligned size is then
 * divided by the block size to determine the total number of blocks needed to
 * satisfy the request.
 *
 * After the total number of blocks is calculated, 'heap_malloc_blocks()' is
 * called to allocate these blocks from the heap. If successful,
 * 'heap_malloc_blocks()' returns a pointer to the start of the allocated
 * memory.
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
  size_t aligned_size = heap_align_value_to_upper (size);
  uint32_t total_blocks = aligned_size / LAMEOS_HEAP_BLOCK_SIZE;
  return heap_malloc_blocks (heap, total_blocks);
}

/**
 * @brief Deallocates a block of memory from the heap.
 *
 * The function frees up the previously allocated block of memory by marking it
 * as free in the heap's block table. The address of the block to be freed is
 * passed to the function as 'ptr'.
 *
 * The process begins by converting the memory address 'ptr' to a block index
 * within the heap using the helper function 'heap_address_to_block()'. The
 * resulting block index represents the start of the block(s) that were
 * previously allocated.
 *
 * After obtaining the start block index, 'heap_mark_blocks_free()' is called
 * to mark the associated block(s) in the heap's block table as free. This
 * effectively deallocates the block of memory and makes it available for
 * future allocation requests.
 *
 * @param heap Pointer to the heap object from which the memory is to be
 * deallocated.
 * @param ptr Pointer to the start of the block of memory to be deallocated.
 * @see heap_mark_blocks_free().
 */
void
heap_free (struct heap *heap, void *ptr)
{
  heap_mark_blocks_free (heap, heap_address_to_block (heap, ptr));
}
