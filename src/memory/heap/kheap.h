/**
 * @file kheap.h
 * @brief Kernel heap management interfaces.
 *
 * This file declares the functions used for managing the kernel heap. This
 * includes initialization of the heap, as well as memory allocation and
 * deallocation. The `kmalloc` function is used to allocate memory, and `kfree`
 * is used to free previously allocated memory. The `kheap_init` function is
 * used to initialize the heap.
 */
#ifndef KHEAP_H
#define KHEAP_H
#include <stddef.h>
#include <stdint.h>

void *kmalloc (size_t size);
void kfree (void *ptr);
void kheap_init ();

#endif