#include "idt.h"
#include "config.h"
#include "io/io.h"
#include "kernel.h"
#include "memory/memory.h"
#include "task/task.h"

/**
 * @brief Array of 512 IDT Descriptors.
 * The kernel maintains an array of 512 IDT descriptors. Each descriptor
 * corresponds to a specific interrupt or exception. The array is initialized
 * by idt_init.
 * @note
 */
struct idt_desc idt_descriptors[LAMEOS_TOTAL_INTERRUPTS];

/**
 * @brief A struct representing the IDT register (IDTR).
 * The IDTR is a control register used by the processor to locate and access
 * the IDT. The IDTR is initialized by idt_init.
 */
struct idtr_desc idtr_descriptor;

static ISR80H_COMMAND isr80h_commands[LAMEOS_MAX_ISR80H_COMMANDS];


/**
 * @brief Wrapper function for assembly routine idt_load.
 * The wrapper fct is called from within idt_init. It loads the IDTR by
 * calling the assembly function idt_load. By loading the kernel IDTR struct,
 * the processor knows where the kernel IDT struct-array is located in memory.
 * @note The assembly routine is exposed to the linker by `global idt_load` in
 * the idt.asm file.
 * @see idt_init in src/idt/idt.c
 * @see idt_load in src/idt/idt.asm
 * @param ptr a void pointer to the IDTR descriptor
 */
extern void idt_load (struct idtr_desc *ptr);
extern void int21h ();
extern void no_interrupt ();
extern void isr80h_wrapper();
void
int21h_handler ()
{
  print ("Key pressed.\n");
  outb (0x20, 0x20);
}

void
no_interrupt_handler ()
{
  outb (0x20, 0x20);
}

/**
 * @brief Interrupt Zero Definition.
 * This interrupt routine is called by the CPU when a divide by zero exception
 * occurs. It is mapped to interrupt 0 in the CPU's IDT when idt_init is
 * called. The routine clears the screen and prints an error message.
 * @see idt_load in src/idt/idt.asm
 */
void
idt_zero ()
{
  print ("ERROR: divide by zero exception occurred.\n");
}

/**
 * @brief Defines an IDT descriptor.
 * Defines a descriptor by setting the offset, selector, zero, type_attr, and
 * offset_2 fields of the descriptor. The offset is the address of the
 * programmable interrupt routine. The selector is the kernel code selector.
 * The zero field is unused and set to zero. The type_attr field is set to
 * 0xEE, which is the type and attributes for a 32-bit interrupt gate. The
 * offset_2 field is the upper 16 bits of the offset.
 * @see idt_init in src/idt/idt.c
 * @param interrupt_no The CPU interrupt number to map fct address to.
 * @param address The address of the programmable interrupt routine.
 */
void
idt_set (int interrupt_no, void *address)
{
  struct idt_desc *desc = &idt_descriptors[interrupt_no];
  desc->offset_1 = (uint32_t)address & 0x0000ffff;
  desc->selector = KERNEL_CODE_SELECTOR;
  desc->zero = 0x00;
  desc->type_attr = 0xEE;
  desc->offset_2 = (uint32_t)address >> 16;
}

/**
 * @brief Initialize Kernel Interrupt Descriptor Table (IDT).
 * Initializes kernel IDT array by zeroing every descpritor in the array,
 * Sets the IDTR descriptor limit and base, Intended to set each IDT
 * descriptor, but currently only sets the interrupt descriptor 0, Concludes
 * Loads the IDTR by calling wrapper function idt_load, for the asm function of
 * the same name. The asm routine idt_load loads the IDTR with the kernel IDTR
 * struct.
 * @note There is a 1:1 mapping between the IDT and the CPU's interrupt
 * numbers.
 * @see memset in src/memory/memory.c
 * @see idt_set in src/idt/idt.c
 * @see idt_load in src/idt/idt.asm
 */
void
idt_init ()
{
  memset (idt_descriptors, 0, sizeof (idt_descriptors));

  idtr_descriptor.limit = sizeof (idt_descriptors) - 1;
  idtr_descriptor.base = (uint32_t)idt_descriptors;

  // Configure any kernel interrupts here
  //--------------------------------------

  // set all interrupts to no_interrupt_handler by default
  for (int i = 0; i < LAMEOS_TOTAL_INTERRUPTS; i++)
    {
      idt_set (i, no_interrupt);
    }

  // set the interrupt 0 handler, divide by zero
  idt_set (0, idt_zero);

  // set the interrupt 0x21 handler, keyboard
  idt_set (0x21, int21h);

  idt_set(0x80, isr80h_wrapper);
  //--------------------------------------

  // load the IDT
  idt_load (&idtr_descriptor);
}


void isr80h_register_command(int command_id, ISR80H_COMMAND command)
{
  if (command_id < 0 || command_id >= LAMEOS_MAX_ISR80H_COMMANDS)
  {
    panic("The command is out of bounds.\n");
  }

  if (isr80h_commands[command_id])
  {
    panic("Attempting to overwrite and existing command.\n");
  }

  isr80h_commands[command_id] = command;
}

void *
isr80h_handle_command (int command, struct interrupt_frame *frame)
{
  void *result = 0;


  if (command < 0 || command >= LAMEOS_MAX_ISR80H_COMMANDS)
  {
    // Invalid command
    return 0;
  }

  ISR80H_COMMAND command_func = isr80h_commands[command];
  if(!command_func)
  {
    return 0;
  }

  result = command_func(frame);
  return result;

}

void *
isr80h_handler (int command, struct interrupt_frame *frame)
{
  void *res = 0;
  kernel_page ();
  task_current_save_state (frame);
  res = isr80h_handle_command (command, frame);
  task_page ();
  return res;
}
