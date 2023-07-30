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
#define LAMEOS_TOTAL_INTERRUPTS 256

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
 * 0x01000000 + 0x6400000 = 0x7400000.
 */
#define LAMEOS_HEAP_ADDRESS 0x01000000

/**
 * @brief The address of the kernel heap table (32 KB). The size of the
 * heap table itself is 32 KB, which is 0x8000 bytes.
 */
#define LAMEOS_HEAP_TABLE_ADDRESS 0x00007E00

#define LAMEOS_SECTOR_SIZE 512

#define LAMEOS_MAX_FILESYSTEMS 12

#define LAMEOS_MAX_FILE_DESCRIPTORS 512

#define LAMEOS_MAX_PATH 108

#define LAMEOS_TOTAL_GDT_SEGMENTS 6

#define LAMEOS_PROGRAM_VIRTUAL_ADDRESS 0x400000

#define LAMEOS_PROGRAM_VIRTUAL_USER_PROGRAM_STACK_SIZE (1024 * 16)

#define LAMEOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START 0x3FF000

#define LAMEOS_PROGRAM_VIRTUAL_STACK_ADDRESS_END                              \
  (LAMEOS_PROGRAM_VIRTUAL_STACK_ADDRESS_START                                 \
   - LAMEOS_PROGRAM_VIRTUAL_USER_PROGRAM_STACK_SIZE)

#define USER_DATA_SEGMENT 0x23
#define USER_CODE_SEGMENT 0x1B

#endif