#ifndef IO_H
#define IO_H

/**
 * @brief C wrapper of BIOS `insb` instruction
 * Reads a byte in from a PIO port.
 * @see /src/io/io.asm
 * @param port The PIO port to read from, range 0x0000 - 0xFFFF (0-65535).
 * @return unsigned char, the byte read in from the port.
 * @note This function is implemented in assembly. A char is 1 byte.
 */
unsigned char insb(unsigned short port);

/**
 * @brief C wrapper of BIOS `insw` instruction
 * Reads a word in from a PIO port.
 * @param port The PIO port to read from, range 0x0000 - 0xFFFF (0-65535).
 * @return unsigned short, the word read in from the port.
 * @note This function is implemented in assembly. A short is 2 bytes.
 */
unsigned short insw(unsigned short port);

/**
 * @brief C wrapper of BIOS `outb` instruction
 * Writes a byte out to a PIO port.
 * @param port The PIO port to write to, range 0x0000 - 0xFFFF (0-65535).
 * @param val The byte to write out to the port.
 * @note This function is implemented in assembly. A char is 1 byte.
 */
void outb(unsigned short port, unsigned char val);

/**
 * @brief C wrapper of BIOS `outw` instruction
 * Writes a word out to a PIO port. 
 * @param port The PIO port to write to, range 0x0000 - 0xFFFF (0-65535).
 * @param val The word to write out to the port.
 * @note This function is implemented in assembly. A short is 2 bytes.
 */
void outw(unsigned short port, unsigned short val);

#endif