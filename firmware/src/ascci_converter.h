#ifndef ASCII_CONVERTER_H
#define ASCII_CONVERTER_H

#include <cstdint>

/**
 * @brief Call this function on initialization to load the ASCII <=> scancode maps.
 */
void fill_scancode_to_ascii();

/**
 * @brief This function converts a scancode to an ASCII character.
 * @return '\0' if no corresponding found 
 */
char convert_to_ascii(uint32_t scancode);

#endif