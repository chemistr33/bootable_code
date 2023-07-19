#ifndef CONFIG_H
#define CONFIG_H

/**
 * @brief Code Segment Selector
 * 
 * The offset of the code_seg entry in the GDT is 0x08.
 */
#define KERNEL_CODE_SELECTOR 0X08

/**
 * @brief Data Segment Selector
 * 
 * The offset of the data_seg entry in the GDT is 0x10.
 */
#define KERNEL_DATA_SELECTOR 0X10

/**
 * @brief Macro Constant Defining Total Interrupts.
 * 
 * The IDT is an array of 512 descriptors, each 8 bytes long. Although in
 * reality only 256 are actually available for use by programmers. The rest
 * are reserved by the CPU for one reason or another.
 */
#define LAMEOS_TOTAL_INTERRUPTS 512

/**
 * @brief Size of the kernel heap in bytes. (100 MB)
 */
#define LAMEOS_HEAP_SIZE_BYTES 104857600

/**
 * @brief Size of each block in the kernel heap in bytes, (4 KB).
 * @example User requests 1 byte of memory. The kernel will allocate 4 KB. 
 */
#define LAMEOS_HEAP_BLOCK_SIZE 4096

/**
 * @brief The starting address of the kernel heap, (16 MB).
 * The kernel heap begins here and ends at 16 MB + 100 MB = 116 MB, Which is
 * 0x01000000 + 0x6400000 = 0x6F00000. 
 */
#define LAMEOS_HEAP_ADDRESS       0x01000000

/**
 * @brief The address of the kernel heap table (32 KB). The size of the
 * heap table itself is 32 KB, which is 0x8000 bytes.
 */
#define LAMEOS_HEAP_TABLE_ADDRESS 0x00007E00

#endif