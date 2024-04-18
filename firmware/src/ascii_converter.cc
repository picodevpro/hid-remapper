#include <unordered_map>
#include <string.h>
#include <cstdlib>
#include "ascci_converter.h"


/**
 * @brief Map used to get the correspondence between the scancode and the associated ASCII character.
 */
static std::unordered_map<int, char> scancode_to_ascii;
static std::unordered_map<int, char> scancode_to_ascii_maj;


void fill_scancode_to_ascii() {
    char keys[] = "abcdefghijklmnopqrstuvwxyz1234567890\n  \t -=[]\\#;'`,./\0";
    char keys_maj[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890\n  \t -=[]\\#;'`,./\0";
    char keypad[] = "/*-+\n1234567890.";

    uint32_t i = 0;
    int scancode = 0x70004; // Commence à 0x70004 pour 'a'
    uint32_t len = strlen(keys);

    for( i = 0; i < len ; i++,  ++scancode ) {
        scancode_to_ascii[scancode] = keys[i];
        scancode_to_ascii_maj[scancode] = keys_maj[i];
    }
    // keypad
    scancode = 0x70054;
    len = strlen(keypad);
    for( i = 0; i < len ; i++,  ++scancode ) {
        scancode_to_ascii[scancode] = keypad[i];
        scancode_to_ascii_maj[scancode] = keypad[i];
    }
}

uint8_t is_maj_key(uint32_t scancode)
{
    uint8_t result = 0;
    switch(scancode){
        case 0x700039: // Keyboard Caps Lock
        case 0x7000E1: // Keyboard Left Shift
        case 0x7000E5: // Keyboard Left Shift
            result = 1;
            break;
        default:
            break;
    }
    return result;
}

char convert_to_ascii(uint32_t scancode, bool maj) 
{
    auto it = scancode_to_ascii.find(scancode);
    if (it != scancode_to_ascii.end()) {
        return it->second;
    } else {
        return '\0';
    }
}