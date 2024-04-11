#include <unordered_map>
#include <string.h>
#include <cstdlib>
#include "ascci_converter.h"


/**
 * @brief Map used to get the correspondence between the scancode and the associated ASCII character.
 */
static std::unordered_map<int, char> scancode_to_ascii;



void fill_scancode_to_ascii() {
    char keys[] = "abcdefghijklmnopqrstuvwxyz1234567890\n \t -+[]\\#;'`,./\0";
    char keypad[] = "/*-+\n1234567890.";

    uint32_t i = 0;
    int scancode = 0x70004; // Commence à 0x70004 pour 'a'
    uint32_t len1 = strlen(keys);
    for( i = 0; i < len1 ; i++,  ++scancode ) {
        scancode_to_ascii[scancode] = keys[i];
    }
    // keypad
    scancode = 0x70054;
    len1 = strlen(keypad);
    for( i = 0; i < len1 ; i++,  ++scancode ) {
        scancode_to_ascii[scancode] = keypad[i];
    }

}

char convert_to_ascii(uint32_t scancode) {
    auto it = scancode_to_ascii.find(scancode);
    if (it != scancode_to_ascii.end()) {
        return it->second;
    } else {
        return '\0';
    }
}