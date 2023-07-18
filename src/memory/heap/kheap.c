/**
 * @file kheap.c
 * @brief Kernel heap management implementation.
 *
 * This file contains the implementations for the kernel heap management
 * functions declared in 'kheap.h'. These functions include memory allocation,
 * memory deallocation, and heap initialization functions for the kernel heap.
 * It uses the heap management interfaces provided in 'heap.h'.
 */
#include "kheap.h"
#include "../../config.h"
#include "../../kernel.h"
#include "heap.h"

/**
 * @brief Global heap object used by the kernel.
 *
 * This is the heap object that the kernel uses to allocate and deallocate
 * memory.
 */
struct heap kernel_heap;

/**
 * @brief Global heap table used by the kernel.
 *
 * This is the heap table object that keeps track of the state of each block in
 * the kernel heap.
 */
struct heap_table kernel_heap_table;

/**
 * @brief Initializes the kernel heap.
 *
 * Initializes the kernel heap and the heap table with pre-defined memory size
 * and table addresses. If the heap creation fails, it logs a message
 * indicating the failure.
 *
 * The kernel heap size and table address are defined by constants
 * LAMEOS_HEAP_SIZE_BYTES, LAMEOS_HEAP_BLOCK_SIZE, and
 * LAMEOS_HEAP_TABLE_ADDRESS. The heap creation is done using heap_create()
 * function, which checks the heap alignment, heap block counts and initializes
 * the heap table.
 *
 * @see heap_create()
 */
void
kheap_init ()
{
  int total_table_entries = LAMEOS_HEAP_SIZE_BYTES / LAMEOS_HEAP_BLOCK_SIZE;
  kernel_heap_table.entries
      = (HEAP_BLOCK_TABLE_ENTRY *)(LAMEOS_HEAP_TABLE_ADDRESS);
  kernel_heap_table.total = total_table_entries;

  void *end = (void *)(LAMEOS_HEAP_ADDRESS + LAMEOS_HEAP_SIZE_BYTES);

  int res = heap_create (&kernel_heap, (void *)(LAMEOS_HEAP_ADDRESS), end,
                         &kernel_heap_table);
  if (res < 0)
    {
      print ("Failed to create heap\n");
    }
}

/**
 * @brief Allocates memory from the kernel heap.
 *
 * This function wraps the heap_malloc function, providing an interface for
 * kernel-level memory allocation. The requested size is passed to the heap
 * manager, which will return a pointer to a block of memory of at least the
 * requested size.
 *
 * @param size The amount of memory, in bytes, to allocate from the heap.
 * @return void* A pointer to the allocated memory on the heap. If the heap
 * cannot fulfill the request, this will be a NULL pointer.
 *
 * @see heap_malloc()
 */
void *
kmalloc (size_t size)
{
  return heap_malloc (&kernel_heap, size);
}

/**
 * @brief Frees memory on the kernel heap.
 *
 * This function wraps the heap_free function, providing an interface for
 * kernel-level memory deallocation. It will free the block of memory that the
 * provided pointer points to, making it available again for future
 * allocations.
 *
 * @param ptr A pointer to the memory block on the heap to be freed.
 *
 * @see heap_free()
 */
void
kfree (void *ptr)
{
  heap_free (&kernel_heap, ptr);
}