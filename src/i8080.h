#ifndef __I_8080_H__
#define __I_8080_H__

#include <stdbool.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

typedef struct i8080_t {
    uint8_t a, b, c, d, e, h, l;
    uint16_t sp, pc;
    bool s, z, ac, p, cy;
    // uint8_t flags;
    
    uint8_t (*read_byte)(uint16_t);
    void (*write_byte)(uint16_t, uint8_t);
} i8080_t;

i8080_t* init_i8080();
void free_i8080(i8080_t* i8080);
void decode(i8080_t* i8080);

#endif // __I_8080_H__
