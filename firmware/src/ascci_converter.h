#ifndef ASCII_CONVERTER_H
#define ASCII_CONVERTER_H

#include <cstdint>

/**
 * @brief Call this function on initialization to load the ASCII <=> scancode maps.
 */
void fill_scancode_to_ascii();

/**
 * @brief Call this function to check if this scancode correponding to an upper case request like left shift
 * @return 1 if its an upper keys else 0
 */
uint8_t is_maj_key(uint32_t scancode);

/**
 * @brief This function converts a scancode to an ASCII character.
 * @param scancode selected key 
 * @param maj 1 if maj is required 
 * @return '\0' if no corresponding found 
 */
char convert_to_ascii(uint32_t scancode, bool maj);

#endif