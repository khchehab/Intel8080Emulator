#include <stdio.h>
#include <stdlib.h>

#include "i8080.h"

uint8_t* rom_bytes;

uint8_t read_byte(uint16_t address);
void write_byte(uint16_t address, uint8_t byte);

void print_binary(uint8_t byte);
_Bool parity(uint8_t byte);

int main(int argc, char* argv[]) {
    // if(argc < 2) {
    //     printf("Rom filename should be entered\n");
    //     return 0;
    // }

    char* filename = "/Users/khaled/Downloads/invaders/invaders.h"; // argv[1];
    printf("filename = %s\n", filename);

    FILE* file = fopen(filename, "rb");
    if(file == NULL) {
        printf("Error could not open the file %s!\n", filename);
        return 0;
    }

    // get the file size
    fseek(file, 0L, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    printf("file size is %d\n", file_size);

    rom_bytes = malloc(file_size);

    fread(rom_bytes, file_size, 1, file);
    fclose(file);

    i8080_t* i8080 = init_i8080();
    i8080->read_byte = read_byte;
    i8080->write_byte = write_byte;

    printf("PC\tOpcode\tInstruction\n");
    while(i8080->pc < file_size) {
        decode(i8080);
    }

    free_i8080(i8080);

    uint8_t a = 0b11011011;
    printf("a: ");
    print_binary(a);
    printf("a has 6 1s in its binary representation which means parity is not set. parity flag: %s\n", (parity(a) ? "true" : "false"));

    return 0;
}

void print_binary(uint8_t byte) {
    printf("0b");
    for(int i = 0; i < 8; ++i)
        printf("%d", (byte & (0x80 >> i)) >> (7 - i));
    printf("\n");
}

_Bool parity(uint8_t byte) {
    // if even number of 1s, return true
    // if odd number of 1s, return false
    uint8_t number_of_ones = 0;
    for(int i = 0; i < 8; ++i)
        if((byte & (0x80 >> i)) != 0)
            number_of_ones++;
    return number_of_ones % 2 == 0;
}

uint8_t read_byte(uint16_t address) {
    return rom_bytes[address];
}

void write_byte(uint16_t address, uint8_t byte) {
    rom_bytes[address] = byte;
}