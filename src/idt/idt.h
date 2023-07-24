#ifndef IDT_H
#define IDT_H
#include <stdint.h>

/**
 * @brief Interrupt Descriptor Table (IDT) Descriptor.
 *
 * This structure represents a single entry in the Interrupt Descriptor Table
 * (IDT). The IDT is used by the processor to handle interrupts and exceptions.
 * Each IDT descriptor corresponds to a specific interrupt or exception and
 * provides the necessary information for the processor to handle them
 * correctly.
 * @note An IDT descriptor is 8 bytes long.
 */
struct idt_desc
{
  uint16_t offset_1; // offset bits 0 - 15
  uint16_t selector; // Selector that's in the GDT
  uint8_t zero;      // Does nothing, usused set to zero
  uint8_t type_attr; // Descriptor type and attributes
  uint16_t offset_2; // Offset bits 16 - 31
} __attribute__ ((packed));

/**
 * @brief IDT Register (IDTR) Descriptor.
 *
 * This structure represents the IDT Register (IDTR) descriptor, which provides
 * the base address and limit of the Interrupt Descriptor Table (IDT). The IDTR
 * is a control register used by the processor to locate and access the IDT.
 */
struct idtr_desc
{
  uint16_t limit; // Size of descriptor table - 1
  uint32_t base;  // base address of the table start point in memory
} __attribute__ ((packed));

/**
 * @brief Initialize Kernel Interrupt Descriptor Table (IDT).
 * Initializes the Interrupt Descriptor Table (IDT) by: Zeroing out the user-IDT
 * array, Setting the IDT Register (IDTR) descriptor limit and base, Setting
 * the IDT descriptors for each programmed interrupt, and Loading the IDTR by
 * calling the assembly function idt_load.
 * @see idt_init in src/idt/idt.c
 */
void idt_init ();
void enable_interrupts ();
void disable_interrupts ();

#endif