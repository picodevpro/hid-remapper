#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <bsp/board_api.h>
#include <tusb.h>

#include <hardware/flash.h>
#include <hardware/gpio.h>
#include <pico/bootrom.h>
#include <pico/mutex.h>
#include <pico/platform.h>
#include <pico/stdio.h>
#include <pico/unique_id.h>

#include "activity_led.h"
#include "config.h"
#include "crc.h"
#include "descriptor_parser.h"
#include "globals.h"
#include "i2c.h"
#include "mcp4651.h"
#include "our_descriptor.h"
#include "platform.h"
#include "remapper.h"
#include "tick.h"
#include "serial.h"
#include "ascci_converter.h"

#define CONFIG_OFFSET_IN_FLASH (PICO_FLASH_SIZE_BYTES - PERSISTED_CONFIG_SIZE)
#define FLASH_CONFIG_IN_MEMORY (((uint8_t*) XIP_BASE) + CONFIG_OFFSET_IN_FLASH)

uint64_t next_print = 0;

mutex_t mutexes[(uint8_t) MutexId::N];

uint32_t gpio_valid_pins_mask = 0;
uint32_t gpio_in_mask = 0;
uint32_t gpio_out_mask = 0;
uint32_t prev_gpio_state = 0;
uint64_t last_gpio_change[32] = { 0 };
bool set_gpio_dir_pending = false;

bool maj_on = false;

void print_stats_maybe() {
    uint64_t now = time_us_64();
    if (now > next_print) {
        print_stats();
        while (next_print < now) {
            next_print += 1000000;
        }
    }
}

void __no_inline_not_in_flash_func(sof_handler)(uint32_t frame_count) {
    sof_callback();
}

/**
 * @brief Call this function to get the current pressed key and put the ASCII on the serial bus 
 */
void send_ascii_result(){
    int size = 16;
    key_value_t keys[size] = {0};
    int pressed = (int) get_pressed_key(keys, size);
    if(pressed != 0){
        for(int i =0; i<pressed; i++){
            uint32_t scancode = keys[i].keycode;
            if(is_maj_key(scancode)){
                printf("Value: %ld\n", keys[i].value);
                maj_on = keys[i].value;
            } else {
                char ascii = convert_to_ascii( scancode, maj_on );
                if(ascii != '\0'){
                    serial_write_data(ascii);
                }
            }
        }
    }
}

bool do_send_report(uint8_t interface, const uint8_t* report_with_id, uint8_t len) {
    
    tud_hid_n_report(interface, report_with_id[0], report_with_id + 1, len - 1);
    
    send_ascii_result();

    return true; 
}

void gpio_pins_init() {
    gpio_valid_pins_mask = get_gpio_valid_pins_mask();
    gpio_init_mask(gpio_valid_pins_mask);
}

void set_gpio_inout_masks(uint32_t in_mask, uint32_t out_mask) {
    // if some pin appears as both input and output, input wins
    gpio_out_mask = (out_mask & ~in_mask) & gpio_valid_pins_mask;
    // we treat all pins except the output ones as input so that the monitor works
    gpio_in_mask = gpio_valid_pins_mask & ~gpio_out_mask;
    set_gpio_dir_pending = true;
}

void set_gpio_dir() {
    gpio_set_dir_masked(gpio_in_mask, 0);
    // output pin direction will be set in write_gpio()
    for (uint8_t i = 0; i <= 29; i++) {
        uint32_t bit = 1 << i;
        if (gpio_valid_pins_mask & bit) {
            gpio_set_pulls(i, gpio_in_mask & bit, false);
        }
    }
}

bool read_gpio(uint64_t now) {
    uint32_t gpio_state = gpio_get_all() & gpio_in_mask;
    uint32_t changed = prev_gpio_state ^ gpio_state;
    if (changed != 0) {
        for (uint8_t i = 0; i <= 29; i++) {
            uint32_t bit = 1 << i;
            if (changed & bit) {
                if (last_gpio_change[i] + gpio_debounce_time <= now) {
                    uint32_t usage = GPIO_USAGE_PAGE | i;
                    int32_t state = !(gpio_state & bit);  // active low
                    set_input_state(usage, state);
                    if (monitor_enabled) {
                        monitor_usage(usage, state, 0);
                    }
                    last_gpio_change[i] = now;
                } else {
                    // ignore this change
                    gpio_state ^= bit;
                    changed ^= bit;
                }
            }
        }
        prev_gpio_state = gpio_state;
    }
    return changed != 0;
}

void write_gpio() {
    if (suspended) {
        return;
    }

    uint32_t value = gpio_out_state[0] | (gpio_out_state[1] << 8) | (gpio_out_state[2] << 16) | (gpio_out_state[3] << 24);
    switch (gpio_output_mode) {
        case 0:
            gpio_put_masked(gpio_out_mask, value);
            gpio_set_dir_masked(gpio_out_mask, gpio_out_mask);
            break;
        case 1:
            gpio_put_masked(gpio_out_mask, 0);
            gpio_set_dir_masked(gpio_out_mask, value);
            break;
    }
    memset(gpio_out_state, 0, sizeof(gpio_out_state));
}

void do_persist_config(uint8_t* buffer) {
#if !PICO_COPY_TO_RAM
    uint32_t ints = save_and_disable_interrupts();
#endif
    flash_range_erase(CONFIG_OFFSET_IN_FLASH, PERSISTED_CONFIG_SIZE);
    flash_range_program(CONFIG_OFFSET_IN_FLASH, buffer, PERSISTED_CONFIG_SIZE);
#if !PICO_COPY_TO_RAM
    restore_interrupts(ints);
#endif
}

void reset_to_bootloader() {
    reset_usb_boot(0, 0);
}

void pair_new_device() {
}

void clear_bonds() {
}

void my_mutexes_init() {
    for (int i = 0; i < (int8_t) MutexId::N; i++) {
        mutex_init(&mutexes[i]);
    }
}

void my_mutex_enter(MutexId id) {
    mutex_enter_blocking(&mutexes[(uint8_t) id]);
}

void my_mutex_exit(MutexId id) {
    mutex_exit(&mutexes[(uint8_t) id]);
}

uint64_t get_time() {
    return time_us_64();
}

uint64_t get_unique_id() {
    pico_unique_board_id_t unique_id;
    pico_get_unique_board_id(&unique_id);
    uint64_t ret = 0;
    for (int i = 0; i < 8; i++) {
        ret |= (uint64_t) unique_id.id[7 - i] << (8 * i);
    }
    return ret;
}

int main() {
    my_mutexes_init();
    gpio_pins_init();
#ifdef I2C_ENABLED
    our_i2c_init();
#endif
    tick_init();
    load_config(FLASH_CONFIG_IN_MEMORY);
    our_descriptor = &our_descriptors[our_descriptor_number];
    parse_our_descriptor();
    set_mapping_from_config();
    board_init();
    extra_init();
    tusb_init();
    stdio_init_all();
    fill_scancode_to_ascii();
    // Initialize UART
    serial_init();
    serial_write_data('\n');
    printf("UART REMAPPER READY\n");

    tud_sof_isr_set(sof_handler);

    next_print = time_us_64() + 1000000;

    while (true) {
        bool tick;
        bool new_report;
        read_report(&new_report, &tick);
        if (new_report) {
            activity_led_on();
        }
        if (their_descriptor_updated) {
            update_their_descriptor_derivates();
            their_descriptor_updated = false;
        }
        if (tick) {
            bool gpio_state_changed = read_gpio(time_us_64());
            if (gpio_state_changed) {
                activity_led_on();
            }
            process_mapping(true);
            write_gpio();
#ifdef MCP4651_ENABLED
            mcp4651_write();
#endif
        }
        tud_task();
        if (config_updated) {
            set_mapping_from_config();
            config_updated = false;
        }
        if (set_gpio_dir_pending && !suspended) {
            set_gpio_dir();
            set_gpio_dir_pending = false;
        }
        if (tud_hid_n_ready(0)) {
            send_report(do_send_report);
        }
        if (monitor_enabled && tud_hid_n_ready(1)) {
            send_monitor_report(do_send_report);
        }
        if (our_descriptor->main_loop_task != nullptr) {
            our_descriptor->main_loop_task();
        }
        send_out_report();
        if (need_to_persist_config) {
            persist_config();
            need_to_persist_config = false;
        }

        activity_led_off_maybe();
    }

    return 0;
}
