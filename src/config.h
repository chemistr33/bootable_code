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

#endif