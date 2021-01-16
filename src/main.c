#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "i8080.h"

static const char* TIME_FORMAT = "%d-%m-%Y %H:%M:%S";
static const int MEMORY_SIZE = 0x10000;
static uint8_t* memory;

static uint8_t read_byte(uint16_t address);
static void write_byte(uint16_t address, uint8_t byte);
static bool write_rom_into_memory(const char* rom_filename, int offset);
static bool run_test_rom(const char* rom_filename, int offset);

int main(int argc, char* argv[]) {
    run_test_rom("tests/TST8080.COM", 0x0100);
    return 0;
}

uint8_t read_byte(uint16_t address) {
    return memory[address];
}

void write_byte(uint16_t address, uint8_t byte) {
    memory[address] = byte;
}

bool write_rom_into_memory(const char* rom_filename, int offset) {
    // assume memory has been allocated
    FILE* fp = fopen(rom_filename, "rb");
    if(fp == NULL) {
        printf("Error could not open the file '%s' for reading.\n", rom_filename);
        return false;
    }

    fseek(fp, 0L, SEEK_END); // go to the end of the file
    int file_size = ftell(fp); // get the file size in bytes
    fseek(fp, 0L, SEEK_SET); // return the pointer to the beginning of the file

    fread(memory + offset, 1, file_size, fp);
    fclose(fp);

    return true;
}

bool run_test_rom(const char* rom_filename, int offset) {
    printf("=====================================\n");

    time_t start_time, end_time;
    struct tm* local_time;
    char time_representation[200];

    start_time = time(NULL);
    local_time = localtime(&start_time);

    if(local_time == NULL) {
        printf("Error getting local time for start time\n");
        return false;
    }

    if(strftime(time_representation, sizeof(time_representation), TIME_FORMAT, local_time) == 0) {
        printf("Error printing out the start time representation\n");
        return false;
    }
    printf("Start Time: %s\n", time_representation);
    printf("=====================================\n");

    memory = calloc(MEMORY_SIZE, sizeof(uint8_t));

    if(write_rom_into_memory(rom_filename, offset)) {
        i8080_t* i8080 = init_i8080(offset);
        i8080->read_byte = read_byte;
        i8080->write_byte = write_byte;

        i8080->write_byte(0x0005, 0xc9);

        uint16_t current_pc, string_address;
        while(true) {
            current_pc = i8080->pc;

            // HLT instruction
            if(i8080->read_byte(i8080->pc) == 0x76) {
                printf("HLT at %04x\n", i8080->pc);
            }

            if(i8080->pc == 0x0005) {
                if(i8080->c == 0x09) {
                    string_address = (i8080->d << 8) | (i8080->e);
                    do {
                        printf("%c", i8080->read_byte(string_address));
                        string_address++;
                    } while(i8080->read_byte(string_address) != 0x24); // print characters until '$' (ascii 0x24) character is reached
                }

                if(i8080->c == 0x02) {
                    printf("%c", i8080->e);
                }

            }

            decode_i8080(i8080);

            if(i8080->pc == 0x0000) {
                printf("\nJumped to 0x0000 from 0x%04x\n", current_pc);
                break;
            }
        }

        free_i8080(i8080);
    } else {
        printf("Failed to write rom into memory\n");
    }

    free(memory);

    end_time = time(NULL);
    local_time = localtime(&end_time);

    if(local_time == NULL) {
        printf("Error getting local time for end time\n");
        return false;
    }

    if(strftime(time_representation, sizeof(time_representation), TIME_FORMAT, local_time) == 0) {
        printf("Error printing out the end time representation\n");
        return false;
    }

    printf("End Time: %s\n", time_representation);
    printf("The test '%s' took %.2lf seconds\n", rom_filename, difftime(end_time, start_time));

    return true;
}
