
#include "../irda_app_signal.h"
#include "irda.h"
#include "irda/helpers/irda_parser.h"
#include "irda_worker.h"
#include "m-string.h"
#include <flipper_file.h>
#include <memory>
#include <string>
#include <furi_hal_irda.h>

#define TAG "IrdaParser"

bool irda_parser_save_signal(FlipperFile* ff, const IrdaAppSignal& signal, const std::string& name) {
    furi_assert(ff);
    furi_assert(!name.empty());

    bool result = false;

    do {
        if(!flipper_file_write_comment_cstr(ff, "")) break;
        if(!flipper_file_write_string_cstr(ff, "name", name.c_str())) break;
        if(signal.is_raw()) {
            furi_assert(signal.get_raw_signal().timings_cnt <= MAX_TIMINGS_AMOUNT);
            auto raw_signal = signal.get_raw_signal();
            if(!flipper_file_write_string_cstr(ff, "type", "raw")) break;
            if(!flipper_file_write_uint32(ff, "frequency", &raw_signal.frequency, 1)) break;
            if(!flipper_file_write_float(ff, "duty_cycle", &raw_signal.duty_cycle, 1)) break;
            if(!flipper_file_write_uint32(ff, "data", raw_signal.timings, raw_signal.timings_cnt))
                break;
        } else {
            auto parsed_signal = signal.get_message();
            const char* protocol_name = irda_get_protocol_name(parsed_signal.protocol);
            if(!flipper_file_write_string_cstr(ff, "type", "parsed")) break;
            if(!flipper_file_write_string_cstr(ff, "protocol", protocol_name)) break;
            if(!flipper_file_write_hex(ff, "address", (uint8_t*)&parsed_signal.address, 4)) break;
            if(!flipper_file_write_hex(ff, "command", (uint8_t*)&parsed_signal.command, 4)) break;
        }
        result = true;
    } while(0);

    return result;
}

bool irda_parser_read_signal(FlipperFile* ff, IrdaAppSignal& signal, std::string& name) {
    furi_assert(ff);

    bool result = false;
    string_t read_string;
    string_init(read_string);

    do {
        if(!flipper_file_read_string(ff, "name", read_string)) break;
        name = string_get_cstr(read_string);
        if(!flipper_file_read_string(ff, "type", read_string)) break;
        if(!string_cmp_str(read_string, "raw")) {
            uint32_t* timings = nullptr;
            uint32_t timings_cnt = 0;
            uint32_t frequency = 0;
            float duty_cycle = 0;

            if(!flipper_file_read_uint32(ff, "frequency", &frequency, 1)) break;
            if(!flipper_file_read_float(ff, "duty_cycle", &duty_cycle, 1)) break;
            if(!flipper_file_get_value_count(ff, "data", &timings_cnt)) break;
            if(timings_cnt > MAX_TIMINGS_AMOUNT) break;
            timings = (uint32_t*)furi_alloc(sizeof(uint32_t) * timings_cnt);
            if(flipper_file_read_uint32(ff, "data", timings, timings_cnt)) {
                signal.set_raw_signal(timings, timings_cnt, frequency, duty_cycle);
                result = true;
            }
            free(timings);
        } else if(!string_cmp_str(read_string, "parsed")) {
            IrdaMessage parsed_signal;
            if(!flipper_file_read_string(ff, "protocol", read_string)) break;
            parsed_signal.protocol = irda_get_protocol_by_name(string_get_cstr(read_string));
            if(!flipper_file_read_hex(ff, "address", (uint8_t*)&parsed_signal.address, 4)) break;
            if(!flipper_file_read_hex(ff, "command", (uint8_t*)&parsed_signal.command, 4)) break;
            if(!irda_parser_is_parsed_signal_valid(&parsed_signal)) break;
            signal.set_message(&parsed_signal);
            result = true;
        } else {
            FURI_LOG_E(TAG, "Unknown type of signal (allowed - raw/parsed) ");
        }
    } while(0);

    string_clear(read_string);
    return result;
}

bool irda_parser_is_parsed_signal_valid(const IrdaMessage* signal) {
    furi_assert(signal);
    bool result = true;

    if(!irda_is_protocol_valid(signal->protocol)) {
        FURI_LOG_E(TAG, "Unknown protocol");
        result = false;
    }

    uint32_t address_length = irda_get_protocol_address_length(signal->protocol);
    uint32_t address_mask = (1LU << address_length) - 1;
    if(signal->address != (signal->address & address_mask)) {
        FURI_LOG_E(
            TAG,
            "Address is out of range (mask 0x%08lX): 0x%lX\r\n",
            address_mask,
            signal->address);
        result = false;
    }

    uint32_t command_length = irda_get_protocol_command_length(signal->protocol);
    uint32_t command_mask = (1LU << command_length) - 1;
    if(signal->command != (signal->command & command_mask)) {
        FURI_LOG_E(
            TAG,
            "Command is out of range (mask 0x%08lX): 0x%lX\r\n",
            command_mask,
            signal->command);
        result = false;
    }

    return result;
}

bool irda_parser_is_raw_signal_valid(uint32_t frequency, float duty_cycle, uint32_t timings_cnt) {
    bool result = true;

    if((frequency > IRDA_MAX_FREQUENCY) || (frequency < IRDA_MIN_FREQUENCY)) {
        FURI_LOG_E(
            TAG,
            "Frequency is out of range (%lX - %lX): %lX",
            IRDA_MIN_FREQUENCY,
            IRDA_MAX_FREQUENCY,
            frequency);
        result = false;
    } else if((duty_cycle <= 0) || (duty_cycle > 1)) {
        FURI_LOG_E(TAG, "Duty cycle is out of range (0 - 1): %f", duty_cycle);
        result = false;
    } else if((timings_cnt <= 0) || (timings_cnt > MAX_TIMINGS_AMOUNT)) {
        FURI_LOG_E(
            TAG, "Timings amount is out of range (0 - %lX): %lX", MAX_TIMINGS_AMOUNT, timings_cnt);
        result = false;
    }

    return result;
}
