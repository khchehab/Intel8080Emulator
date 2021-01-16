#ifndef __I_8080_H__
#define __I_8080_H__

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

typedef struct i8080_t {
    uint8_t a, b, c, d, e, h, l;
    uint16_t sp, pc;
    _Bool s, z, ac, p, cy;
    _Bool interrupt_enabled;
    
    uint8_t (*read_byte)(uint16_t);
    void (*write_byte)(uint16_t, uint8_t);
} i8080_t;

i8080_t* init_i8080(uint16_t initial_pc);
void free_i8080(i8080_t* i8080);
void decode_i8080(i8080_t* i8080);

#endif // __I_8080_H__
