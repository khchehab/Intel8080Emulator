#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "i8080.h"

static const uint8_t OPCODE_LENGTHS[] = {
/*      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
/* 0 */ 1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
/* 1 */ 1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
/* 2 */ 1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1,
/* 3 */ 1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1,
/* 4 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 6 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 8 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 9 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* a */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* b */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* c */ 1, 1, 3, 3, 3, 1, 2, 1, 1, 1, 3, 3, 3, 3, 2, 1,
/* d */ 1, 1, 3, 2, 3, 1, 2, 1, 1, 1, 3, 2, 3, 3, 2, 1,
/* e */ 1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1,
/* f */ 1, 1, 3, 1, 3, 1, 2, 1, 1, 1, 3, 1, 3, 3, 2, 1
};

// private helper functions
static uint8_t instr_inr(i8080_t* i8080, uint8_t register_value);
static uint8_t instr_dcr(i8080_t* i8080, uint8_t register_value);
static uint8_t instr_add_adc(i8080_t* i8080, uint8_t register_value, bool include_carry);
static uint8_t instr_sub_sbb(i8080_t* i8080, uint8_t register_value, bool include_carry);
static uint8_t instr_ana(i8080_t* i8080, uint8_t register_value);
static uint8_t instr_xra(i8080_t* i8080, uint8_t register_value);
static uint8_t instr_ora(i8080_t* i8080, uint8_t register_value);
static void instr_rlc(i8080_t* i8080);
static void instr_rrc(i8080_t* i8080);
static void instr_ral(i8080_t* i8080);
static void instr_rar(i8080_t* i8080);
static void instr_dad(i8080_t* i8080, uint16_t register_value);
static uint16_t instr_inx(i8080_t* i8080, uint16_t register_value);
static uint16_t instr_dcx(i8080_t* i8080, uint16_t register_value);
static void instr_xchg(i8080_t* i8080);
static void instr_xthl(i8080_t* i8080);
static void instr_push_bdh(i8080_t* i8080, uint16_t register_value);
static void instr_push_psw(i8080_t* i8080);
static uint16_t instr_pop_bdh(i8080_t* i8080);
static void instr_pop_psw(i8080_t* i8080);
static uint16_t instr_lxi(i8080_t* i8080);
static void instr_sta(i8080_t* i8080);
static uint16_t instr_lda(i8080_t* i8080);
static void instr_shld(i8080_t* i8080);
static void instr_lhld(i8080_t* i8080);
static void instr_jmp(i8080_t* i8080);
static void instr_call(i8080_t* i8080);
static void instr_ret(i8080_t* i8080);

static uint16_t i8080_bc(i8080_t* i8080);
static uint16_t i8080_de(i8080_t* i8080);
static uint16_t i8080_hl(i8080_t* i8080);
static uint8_t i8080_psw(i8080_t* i8080);
static uint16_t read_word(i8080_t* i8080);
static void i8080_set_bc(i8080_t* i8080, uint16_t bc);
static void i8080_set_de(i8080_t* i8080, uint16_t de);
static void i8080_set_hl(i8080_t* i8080, uint16_t hl);
static void i8080_set_psw(i8080_t* i8080, uint8_t psw);
static void i8080_szp(i8080_t* i8080, uint8_t byte);
static bool parity(uint8_t byte);

i8080_t* init_i8080() {
    i8080_t* i8080 = malloc(sizeof(i8080_t));
    i8080->a = 0x00;
    i8080->b = 0x00;
    i8080->c = 0x00;
    i8080->d = 0x00;
    i8080->e = 0x00;
    i8080->h = 0x00;
    i8080->l = 0x00;
    i8080->sp = 0x0000;
    i8080->pc = 0x0000;
    i8080->s = false;
    i8080->z = false;
    i8080->ac = false;
    i8080->p = false;
    i8080->cy = false;
    return i8080;
}

void free_i8080(i8080_t* i8080) {
    if(i8080 != NULL) {
        free(i8080);
    }
}

void decode(i8080_t* i8080) {
    uint8_t opcode = i8080->read_byte(i8080->pc);
    uint8_t opcode_size = OPCODE_LENGTHS[opcode];

    // print the current program counter and the opcode
    printf("0x%04x\t0x%02x\t", i8080->pc, opcode);

    // execute the current opcode
    switch(opcode) {
        case 0x00: printf("NOP"); break;

        // Carry Bit Instructions
        case 0x37: printf("STC"); i8080->cy = true; break;
        case 0x3f: printf("CMC"); i8080->cy = !i8080->cy; break;

        // Single Register Instructions
        case 0x3c: printf("INR A"); i8080->a = instr_inr(i8080, i8080->a); break;
        case 0x04: printf("INR B"); i8080->b = instr_inr(i8080, i8080->b); break;
        case 0x0c: printf("INR C"); i8080->c = instr_inr(i8080, i8080->c); break;
        case 0x14: printf("INR D"); i8080->d = instr_inr(i8080, i8080->d); break; 
        case 0x1c: printf("INR E"); i8080->e = instr_inr(i8080, i8080->e); break;
        case 0x24: printf("INR H"); i8080->h = instr_inr(i8080, i8080->h); break;
        case 0x2c: printf("INR L"); i8080->l = instr_inr(i8080, i8080->l); break;
        case 0x34: printf("INR M"); i8080->write_byte(i8080_hl(i8080), instr_inr(i8080, i8080->read_byte(i8080_hl(i8080)))); break;

        case 0x3d: printf("DCR A"); i8080->a = instr_dcr(i8080, i8080->a); break;
        case 0x05: printf("DCR B"); i8080->b = instr_dcr(i8080, i8080->b); break;
        case 0x0d: printf("DCR C"); i8080->c = instr_dcr(i8080, i8080->c); break;
        case 0x15: printf("DCR D"); i8080->d = instr_dcr(i8080, i8080->d); break;
        case 0x1d: printf("DCR E"); i8080->e = instr_dcr(i8080, i8080->e); break;
        case 0x25: printf("DCR H"); i8080->h = instr_dcr(i8080, i8080->h); break;
        case 0x2d: printf("DCR L"); i8080->l = instr_dcr(i8080, i8080->l); break;
        case 0x35: printf("DCR M"); i8080->write_byte(i8080_hl(i8080), instr_dcr(i8080, i8080->read_byte(i8080_hl(i8080)))); break;

        case 0x2f: printf("CMA"); i8080->a ^= 0xff ; break;
        case 0x27: printf("DAA"); break;

        // Data Transfer Instructions
        case 0x7f: printf("MOV A, A"); i8080->a = i8080->a; break;
        case 0x78: printf("MOV A, B"); i8080->a = i8080->b; break;
        case 0x79: printf("MOV A, C"); i8080->a = i8080->c; break;
        case 0x7a: printf("MOV A, D"); i8080->a = i8080->d; break;
        case 0x7b: printf("MOV A, E"); i8080->a = i8080->e; break;
        case 0x7c: printf("MOV A, H"); i8080->a = i8080->h; break;
        case 0x7d: printf("MOV A, L"); i8080->a = i8080->l; break;
        case 0x7e: printf("MOV A, M"); i8080->a = i8080->read_byte(i8080_hl(i8080)); break;

        case 0x47: printf("MOV B, A"); i8080->b = i8080->a; break;
        case 0x40: printf("MOV B, B"); i8080->b = i8080->b; break;
        case 0x41: printf("MOV B, C"); i8080->b = i8080->c; break;
        case 0x42: printf("MOV B, D"); i8080->b = i8080->d; break;
        case 0x43: printf("MOV B, E"); i8080->b = i8080->e; break;
        case 0x44: printf("MOV B, H"); i8080->b = i8080->h; break;
        case 0x45: printf("MOV B, L"); i8080->b = i8080->l; break;
        case 0x46: printf("MOV B, M"); i8080->b = i8080->read_byte(i8080_hl(i8080)); break;

        case 0x4f: printf("MOV C, A"); i8080->c = i8080->a; break;
        case 0x48: printf("MOV C, B"); i8080->c = i8080->b; break;
        case 0x49: printf("MOV C, C"); i8080->c = i8080->c; break;
        case 0x4a: printf("MOV C, D"); i8080->c = i8080->d; break;
        case 0x4b: printf("MOV C, E"); i8080->c = i8080->e; break;
        case 0x4c: printf("MOV C, H"); i8080->c = i8080->h; break;
        case 0x4d: printf("MOV C, L"); i8080->c = i8080->l; break;
        case 0x4e: printf("MOV C, M"); i8080->c = i8080->read_byte(i8080_hl(i8080)); break;

        case 0x57: printf("MOV D, A"); i8080->d = i8080->a; break;
        case 0x50: printf("MOV D, B"); i8080->d = i8080->b; break;
        case 0x51: printf("MOV D, C"); i8080->d = i8080->c; break;
        case 0x52: printf("MOV D, D"); i8080->d = i8080->d; break;
        case 0x53: printf("MOV D, E"); i8080->d = i8080->e; break;
        case 0x54: printf("MOV D, H"); i8080->d = i8080->h; break;
        case 0x55: printf("MOV D, L"); i8080->d = i8080->l; break;
        case 0x56: printf("MOV D, M"); i8080->d = i8080->read_byte(i8080_hl(i8080)); break;

        case 0x5f: printf("MOV E, A"); i8080->e = i8080->a; break;
        case 0x58: printf("MOV E, B"); i8080->e = i8080->b; break;
        case 0x59: printf("MOV E, C"); i8080->e = i8080->c; break;
        case 0x5a: printf("MOV E, D"); i8080->e = i8080->d; break;
        case 0x5b: printf("MOV E, E"); i8080->e = i8080->e; break;
        case 0x5c: printf("MOV E, H"); i8080->e = i8080->h; break;
        case 0x5d: printf("MOV E, L"); i8080->e = i8080->l; break;
        case 0x5e: printf("MOV E, M"); i8080->e = i8080->read_byte(i8080_hl(i8080)); break;

        case 0x67: printf("MOV H, A"); i8080->h = i8080->a; break;
        case 0x60: printf("MOV H, B"); i8080->h = i8080->b; break;
        case 0x61: printf("MOV H, C"); i8080->h = i8080->c; break;
        case 0x62: printf("MOV H, D"); i8080->h = i8080->d; break;
        case 0x63: printf("MOV H, E"); i8080->h = i8080->e; break;
        case 0x64: printf("MOV H, H"); i8080->h = i8080->h; break;
        case 0x65: printf("MOV H, L"); i8080->h = i8080->l; break;
        case 0x66: printf("MOV H, M"); i8080->h = i8080->read_byte(i8080_hl(i8080)); break;

        case 0x6f: printf("MOV L, A"); i8080->l = i8080->a; break;
        case 0x68: printf("MOV L, B"); i8080->l = i8080->b; break;
        case 0x69: printf("MOV L, C"); i8080->l = i8080->c; break;
        case 0x6a: printf("MOV L, D"); i8080->l = i8080->d; break;
        case 0x6b: printf("MOV L, E"); i8080->l = i8080->e; break;
        case 0x6c: printf("MOV L, H"); i8080->l = i8080->h; break;
        case 0x6d: printf("MOV L, L"); i8080->l = i8080->l; break;
        case 0x6e: printf("MOV L, M"); i8080->l = i8080->read_byte(i8080_hl(i8080)); break;

        case 0x77: printf("MOV M, A"); i8080->write_byte(i8080_hl(i8080), i8080->a); break;
        case 0x70: printf("MOV M, B"); i8080->write_byte(i8080_hl(i8080), i8080->b); break;
        case 0x71: printf("MOV M, C"); i8080->write_byte(i8080_hl(i8080), i8080->c); break;
        case 0x72: printf("MOV M, D"); i8080->write_byte(i8080_hl(i8080), i8080->d); break;
        case 0x73: printf("MOV M, E"); i8080->write_byte(i8080_hl(i8080), i8080->e); break;
        case 0x74: printf("MOV M, H"); i8080->write_byte(i8080_hl(i8080), i8080->h); break;
        case 0x75: printf("MOV M, L"); i8080->write_byte(i8080_hl(i8080), i8080->l); break;

        case 0x02: printf("STAX B"); i8080->write_byte(i8080_bc(i8080), i8080->a); break;
        case 0x12: printf("STAX D"); i8080->write_byte(i8080_de(i8080), i8080->a); break;

        case 0x0a: printf("LDAX B"); i8080->a = i8080->read_byte(i8080_bc(i8080)); break;
        case 0x1a: printf("LDAX D"); i8080->a = i8080->read_byte(i8080_de(i8080)); break;

        // Regiser or Memory to Accumulator Instructions
        case 0x87: printf("ADD A"); i8080->a = instr_add_adc(i8080, i8080->a, false); break;
        case 0x80: printf("ADD B"); i8080->a = instr_add_adc(i8080, i8080->b, false); break;
        case 0x81: printf("ADD C"); i8080->a = instr_add_adc(i8080, i8080->c, false); break;
        case 0x82: printf("ADD D"); i8080->a = instr_add_adc(i8080, i8080->d, false); break;
        case 0x83: printf("ADD E"); i8080->a = instr_add_adc(i8080, i8080->e, false); break;
        case 0x84: printf("ADD H"); i8080->a = instr_add_adc(i8080, i8080->h, false); break;
        case 0x85: printf("ADD L"); i8080->a = instr_add_adc(i8080, i8080->l, false); break;
        case 0x86: printf("ADD M"); i8080->a = instr_add_adc(i8080, i8080->read_byte(i8080_hl(i8080)), false); break;

        case 0x8f: printf("ADC A"); i8080->a = instr_add_adc(i8080, i8080->a, i8080->cy); break;
        case 0x88: printf("ADC B"); i8080->a = instr_add_adc(i8080, i8080->b, i8080->cy); break;
        case 0x89: printf("ADC C"); i8080->a = instr_add_adc(i8080, i8080->c, i8080->cy); break;
        case 0x8a: printf("ADC D"); i8080->a = instr_add_adc(i8080, i8080->d, i8080->cy); break;
        case 0x8b: printf("ADC E"); i8080->a = instr_add_adc(i8080, i8080->e, i8080->cy); break;
        case 0x8c: printf("ADC H"); i8080->a = instr_add_adc(i8080, i8080->h, i8080->cy); break;
        case 0x8d: printf("ADC L"); i8080->a = instr_add_adc(i8080, i8080->l, i8080->cy); break;
        case 0x8e: printf("ADC M"); i8080->a = instr_add_adc(i8080, i8080->read_byte(i8080_hl(i8080)), i8080->cy); break;

        case 0x97: printf("SUB A"); i8080->a = instr_sub_sbb(i8080, i8080->a, false); break;
        case 0x90: printf("SUB B"); i8080->a = instr_sub_sbb(i8080, i8080->b, false); break;
        case 0x91: printf("SUB C"); i8080->a = instr_sub_sbb(i8080, i8080->c, false); break;
        case 0x92: printf("SUB D"); i8080->a = instr_sub_sbb(i8080, i8080->d, false); break;
        case 0x93: printf("SUB E"); i8080->a = instr_sub_sbb(i8080, i8080->e, false); break;
        case 0x94: printf("SUB H"); i8080->a = instr_sub_sbb(i8080, i8080->h, false); break;
        case 0x95: printf("SUB L"); i8080->a = instr_sub_sbb(i8080, i8080->l, false); break;
        case 0x96: printf("SUB M"); i8080->a = instr_sub_sbb(i8080, i8080->read_byte(i8080_hl(i8080)), false); break;

        case 0x9f: printf("SBB A"); i8080->a = instr_sub_sbb(i8080, i8080->a, i8080->cy); break;
        case 0x98: printf("SBB B"); i8080->a = instr_sub_sbb(i8080, i8080->b, i8080->cy); break;
        case 0x99: printf("SBB C"); i8080->a = instr_sub_sbb(i8080, i8080->c, i8080->cy); break;
        case 0x9a: printf("SBB D"); i8080->a = instr_sub_sbb(i8080, i8080->d, i8080->cy); break;
        case 0x9b: printf("SBB E"); i8080->a = instr_sub_sbb(i8080, i8080->e, i8080->cy); break;
        case 0x9c: printf("SBB H"); i8080->a = instr_sub_sbb(i8080, i8080->h, i8080->cy); break;
        case 0x9d: printf("SBB L"); i8080->a = instr_sub_sbb(i8080, i8080->l, i8080->cy); break;
        case 0x9e: printf("SBB M"); i8080->a = instr_sub_sbb(i8080, i8080->read_byte(i8080_hl(i8080)), i8080->cy); break;

        case 0xa7: printf("ANA A"); i8080->a = instr_ana(i8080, i8080->a); break;
        case 0xa0: printf("ANA B"); i8080->a = instr_ana(i8080, i8080->b); break;
        case 0xa1: printf("ANA C"); i8080->a = instr_ana(i8080, i8080->c); break;
        case 0xa2: printf("ANA D"); i8080->a = instr_ana(i8080, i8080->d); break;
        case 0xa3: printf("ANA E"); i8080->a = instr_ana(i8080, i8080->e); break;
        case 0xa4: printf("ANA H"); i8080->a = instr_ana(i8080, i8080->h); break;
        case 0xa5: printf("ANA L"); i8080->a = instr_ana(i8080, i8080->l); break;
        case 0xa6: printf("ANA M"); i8080->a = instr_ana(i8080, i8080->read_byte(i8080_hl(i8080))); break;

        case 0xaf: printf("XRA A"); i8080->a = instr_xra(i8080, i8080->a); break;
        case 0xa8: printf("XRA B"); i8080->a = instr_xra(i8080, i8080->b); break;
        case 0xa9: printf("XRA C"); i8080->a = instr_xra(i8080, i8080->c); break;
        case 0xaa: printf("XRA D"); i8080->a = instr_xra(i8080, i8080->d); break;
        case 0xab: printf("XRA E"); i8080->a = instr_xra(i8080, i8080->e); break;
        case 0xac: printf("XRA H"); i8080->a = instr_xra(i8080, i8080->h); break;
        case 0xad: printf("XRA L"); i8080->a = instr_xra(i8080, i8080->l); break;
        case 0xae: printf("XRA M"); i8080->a = instr_xra(i8080, i8080->read_byte(i8080_hl(i8080))); break;

        case 0xb7: printf("ORA A"); i8080->a = instr_ora(i8080, i8080->a); break;
        case 0xb0: printf("ORA B"); i8080->a = instr_ora(i8080, i8080->b); break;
        case 0xb1: printf("ORA C"); i8080->a = instr_ora(i8080, i8080->c); break;
        case 0xb2: printf("ORA D"); i8080->a = instr_ora(i8080, i8080->d); break;
        case 0xb3: printf("ORA E"); i8080->a = instr_ora(i8080, i8080->e); break;
        case 0xb4: printf("ORA H"); i8080->a = instr_ora(i8080, i8080->h); break;
        case 0xb5: printf("ORA L"); i8080->a = instr_ora(i8080, i8080->l); break;
        case 0xb6: printf("ORA M"); i8080->a = instr_ora(i8080, i8080->read_byte(i8080_hl(i8080))); break;

        // CMP basically does the same subtract as SUB instruction and sets the flags with the same rules without changing any registers
        case 0xbf: printf("CMP A"); instr_sub_sbb(i8080, i8080->a, false); break;
        case 0xb8: printf("CMP B"); instr_sub_sbb(i8080, i8080->b, false); break;
        case 0xb9: printf("CMP C"); instr_sub_sbb(i8080, i8080->c, false); break;
        case 0xba: printf("CMP D"); instr_sub_sbb(i8080, i8080->d, false); break;
        case 0xbb: printf("CMP E"); instr_sub_sbb(i8080, i8080->e, false); break;
        case 0xbc: printf("CMP H"); instr_sub_sbb(i8080, i8080->h, false); break;
        case 0xbd: printf("CMP L"); instr_sub_sbb(i8080, i8080->l, false); break;
        case 0xbe: printf("CMP M"); instr_sub_sbb(i8080, i8080->read_byte(i8080_hl(i8080)), false); break;

        // Rotate Accumulator Instructions
        case 0x07: printf("RLC"); instr_rlc(i8080); break;
        case 0x0f: printf("RRC"); instr_rrc(i8080); break;
        case 0x17: printf("RAL"); instr_ral(i8080); break;
        case 0x1f: printf("RAR"); instr_rar(i8080); break;

        // Register Pair Instructions
        case 0xc5: printf("PUSH B"); instr_push_bdh(i8080, i8080_bc(i8080)); break;
        case 0xd5: printf("PUSH D"); instr_push_bdh(i8080, i8080_de(i8080)); break;
        case 0xe5: printf("PUSH H"); instr_push_bdh(i8080, i8080_hl(i8080)); break;
        case 0xf5: printf("PUSH PSW"); instr_push_psw(i8080); break;

        case 0xc1: printf("POP B"); i8080_set_bc(i8080, instr_pop_bdh(i8080)); break;
        case 0xd1: printf("POP D"); i8080_set_de(i8080, instr_pop_bdh(i8080)); break;
        case 0xe1: printf("POP H"); i8080_set_hl(i8080, instr_pop_bdh(i8080)); break;
        case 0xf1: printf("POP PSW"); instr_pop_psw(i8080); break;

        case 0x09: printf("DAD B"); instr_dad(i8080, i8080_bc(i8080)); break;
        case 0x19: printf("DAD D"); instr_dad(i8080, i8080_de(i8080)); break;
        case 0x29: printf("DAD H"); instr_dad(i8080, i8080_hl(i8080)); break;
        case 0x39: printf("DAD SP"); instr_dad(i8080, i8080->sp); break;

        case 0x03: printf("INX B"); i8080_set_bc(i8080, instr_inx(i8080, i8080_bc(i8080))); break;
        case 0x13: printf("INX D"); i8080_set_de(i8080, instr_inx(i8080, i8080_de(i8080))); break;
        case 0x23: printf("INX H"); i8080_set_hl(i8080, instr_inx(i8080, i8080_hl(i8080))); break;
        case 0x33: printf("INX SP"); i8080->sp = instr_inx(i8080, i8080->sp); break;

        case 0x0b: printf("DCX B"); i8080_set_bc(i8080, instr_dcx(i8080, i8080_bc(i8080))); break;
        case 0x1b: printf("DCX D"); i8080_set_de(i8080, instr_dcx(i8080, i8080_de(i8080))); break;
        case 0x2b: printf("DCX H"); i8080_set_hl(i8080, instr_dcx(i8080, i8080_hl(i8080))); break;
        case 0x3b: printf("DCX SP"); i8080->sp = instr_dcx(i8080, i8080->sp); break;

        case 0xeb: printf("XCHG"); instr_xchg(i8080); break;
        case 0xe3: printf("XTHL"); instr_xthl(i8080); break;
        case 0xf9: printf("SPHL"); i8080->sp = i8080_hl(i8080); break;

        // Immediate Instructions
        case 0x01: printf("LXI B, #0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); i8080_set_bc(i8080, instr_lxi(i8080)); break;
        case 0x11: printf("LXI D, #0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); i8080_set_de(i8080, instr_lxi(i8080)); break;
        case 0x21: printf("LXI H, #0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); i8080_set_hl(i8080, instr_lxi(i8080)); break;
        case 0x31: printf("LXI SP, #0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); i8080->sp = instr_lxi(i8080); break;

        case 0x3e: printf("MVI A, #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->a = i8080->read_byte(i8080->pc + 1); break;
        case 0x06: printf("MVI B, #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->b = i8080->read_byte(i8080->pc + 1); break;
        case 0x0e: printf("MVI C, #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->c = i8080->read_byte(i8080->pc + 1); break;
        case 0x16: printf("MVI D, #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->d = i8080->read_byte(i8080->pc + 1); break;
        case 0x1e: printf("MVI E, #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->e = i8080->read_byte(i8080->pc + 1); break;
        case 0x26: printf("MVI H, #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->h = i8080->read_byte(i8080->pc + 1); break;
        case 0x2e: printf("MVI L, #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->l = i8080->read_byte(i8080->pc + 1); break;
        case 0x36: printf("MVI M, #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->write_byte(i8080_hl(i8080), i8080->read_byte(i8080->pc + 1)); break;

        case 0xc6: printf("ADI #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->a = instr_add_adc(i8080, i8080->read_byte(i8080->pc + 1), false); break;
        case 0xce: printf("ACI #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->a = instr_add_adc(i8080, i8080->read_byte(i8080->pc + 1), i8080->cy); break;
        case 0xd6: printf("SUI #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->a = instr_sub_sbb(i8080, i8080->read_byte(i8080->pc + 1), false); break;
        case 0xde: printf("SBI #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->a = instr_sub_sbb(i8080, i8080->read_byte(i8080->pc + 1), i8080->cy); break;
        case 0xe6: printf("ANI #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->a = instr_ana(i8080, i8080->read_byte(i8080->pc + 1)); break;
        case 0xee: printf("XRI #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->a = instr_xra(i8080, i8080->read_byte(i8080->pc + 1)); break;
        case 0xf6: printf("ORI #0x%02x", i8080->read_byte(i8080->pc + 1)); i8080->a = instr_ora(i8080, i8080->read_byte(i8080->pc + 1)); break;
        case 0xfe: printf("CPI #0x%02x", i8080->read_byte(i8080->pc + 1)); instr_sub_sbb(i8080, i8080->read_byte(i8080->pc + 1), false); break;

        // Direct Addressing Instructions
        case 0x32: printf("STA 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); instr_sta(i8080); break;
        case 0x3a: printf("LDA 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); i8080->a = instr_lda(i8080); break;

        case 0x22: printf("SHLD 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); instr_shld(i8080); break;
        case 0x2a: printf("LHLD 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); instr_lhld(i8080); break;

        // Jump Instructions
        case 0xe9: printf("PCHL"); i8080->pc = i8080_hl(i8080); break;
        case 0xc3: printf("JMP 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); instr_jmp(i8080); break;
        case 0xda: printf("JC 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(i8080->cy) { instr_jmp(i8080); } break;
        case 0xd2: printf("JNC 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(!i8080->cy) { instr_jmp(i8080); } break;
        case 0xca: printf("JZ 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(i8080->z) { instr_jmp(i8080); } break;
        case 0xc2: printf("JNZ 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(!i8080->z) { instr_jmp(i8080); } break;
        case 0xfa: printf("JM 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(i8080->s) { instr_jmp(i8080); } break;
        case 0xf2: printf("JP 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(!i8080->s) { instr_jmp(i8080); } break;
        case 0xea: printf("JPE 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(i8080->p) { instr_jmp(i8080); } break;
        case 0xe2: printf("JPO 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(!i8080->p) { instr_jmp(i8080); } break;

        // Call Subroutine Instructions
        case 0xcd: printf("CALL 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); instr_call(i8080); break;
        case 0xdc: printf("CC 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(i8080->cy) { instr_call(i8080); } break;
        case 0xd4: printf("CNC 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(!i8080->cy) { instr_call(i8080); } break;
        case 0xcc: printf("CZ 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(i8080->z) { instr_call(i8080); } break;
        case 0xc4: printf("CNZ 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(!i8080->z) { instr_call(i8080); } break;
        case 0xfc: printf("CM 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(i8080->s) { instr_call(i8080); } break;
        case 0xf4: printf("CP 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(!i8080->s) { instr_call(i8080); } break;
        case 0xec: printf("CPE 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(i8080->p) { instr_call(i8080); } break;
        case 0xe4: printf("CPO 0x%02x%02x", i8080->read_byte(i8080->pc + 2), i8080->read_byte(i8080->pc + 1)); if(!i8080->p) { instr_call(i8080); } break;

        // Return From Subroutine Instructions
        case 0xc9: printf("RET"); instr_ret(i8080); break;
        case 0xd8: printf("RC"); if(i8080->cy) { instr_ret(i8080); } break;
        case 0xd0: printf("RNC"); if(!i8080->cy) { instr_ret(i8080); } break;
        case 0xc8: printf("RZ"); if(i8080->z) { instr_ret(i8080); } break;
        case 0xc0: printf("RNZ"); if(!i8080->z) { instr_ret(i8080); } break;
        case 0xf8: printf("RM"); if(i8080->s) { instr_ret(i8080); } break;
        case 0xf0: printf("RP"); if(!i8080->s) { instr_ret(i8080); } break;
        case 0xe8: printf("RPE"); if(i8080->p) { instr_ret(i8080); } break;
        case 0xe0: printf("RPO"); if(!i8080->p) { instr_ret(i8080); } break;

        // RST (Reset) Instructions
        case 0xc7: printf("RST 0"); break;
        case 0xcf: printf("RST 1"); break;
        case 0xd7: printf("RST 2"); break;
        case 0xdf: printf("RST 3"); break;
        case 0xe7: printf("RST 4"); break;
        case 0xef: printf("RST 5"); break;
        case 0xf7: printf("RST 6"); break;
        case 0xff: printf("RST 7"); break;

        // Interrupt Flip-Flop Instructions
        case 0xfb: printf("EI"); break;
        case 0xf3: printf("DI"); break;

        // Input/Output Instructions
        case 0xdb: printf("IN #0x%02x", i8080->read_byte(i8080->pc + 1)); break;
        case 0xd3: printf("OUT #0x%02x", i8080->read_byte(i8080->pc + 1)); break;

        // HLT (Halt) Instructions
        case 0x76: printf("HLT"); break;

        // Other Instructions
        case 0x08: printf("-"); break;
        case 0x10: printf("-"); break;
        case 0x18: printf("-"); break;
        case 0x20: printf("-"); break;
        case 0x28: printf("-"); break;
        case 0x30: printf("-"); break;
        case 0x38: printf("-"); break;
        case 0xcb: printf("-"); break;
        case 0xd9: printf("-"); break;
        case 0xdd: printf("-"); break;
        case 0xed: printf("-"); break;
        case 0xfd: printf("-"); break;
    }

    printf("\n");
    i8080->pc += opcode_size;
}

uint8_t instr_inr(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = register_value + 1;
    i8080_szp(i8080, result);
    i8080->ac = (result & 0x0f) == 0; // set if there is a carry from the low nibble to the high nibble, unset otherwise
    return result;
}

uint8_t instr_dcr(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = register_value - 1;
    i8080_szp(i8080, result);
    i8080->ac = (result & 0x0f) != 0x0f; // set if there is a borrow from the high nibble to the low nibble, unset otherwise
    return result;
}

uint8_t instr_add_adc(i8080_t* i8080, uint8_t register_value, bool include_carry) {
    uint16_t result = i8080->a + register_value + include_carry;
    i8080_szp(i8080, result & 0xff);
    i8080->ac = (((i8080->a & 0x0f) + (register_value & 0x0f) + include_carry) & 0x10) != 0;
    i8080->cy = (result & 0x0100) != 0;
    return result & 0xff;
}

uint8_t instr_sub_sbb(i8080_t* i8080, uint8_t register_value, bool include_carry) {
    uint16_t result = i8080->a - register_value - include_carry;
    i8080_szp(i8080, result & 0xff);
    i8080->ac = (((i8080->a & 0x0f) - (register_value & 0x0f) - include_carry) & 0x10) != 0;
    i8080->cy = (result & 0x0100) != 0;
    return result & 0xff;
}

uint8_t instr_ana(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = i8080->a & register_value;
    i8080_szp(i8080, result);
    i8080->ac = (i8080->a & 0x08) | (register_value & 0x08);
    i8080->cy = false;
    return result;
}

uint8_t instr_xra(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = i8080->a ^ register_value;
    i8080_szp(i8080, result);
    i8080->ac = false;
    i8080->cy = false;
    return result;
}

uint8_t instr_ora(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = i8080->a | register_value;
    i8080_szp(i8080, result);
    i8080->ac = false;
    i8080->cy = false;
    return result;
}

void instr_rlc(i8080_t* i8080) {
    i8080->cy = (i8080->a & 0x80) >> 7;
    i8080->a = (i8080->a << 1) | i8080->cy;
}

void instr_rrc(i8080_t* i8080) {
    i8080->cy = i8080->a & 0x01;
    i8080->a = (i8080->a >> 1) | (i8080->cy << 7);
}

void instr_ral(i8080_t* i8080) {
    bool new_cy = (i8080->a & 0x80) >> 7;
    i8080->a = (i8080->a << 1) | i8080->cy;
    i8080->cy = new_cy;
}

void instr_rar(i8080_t* i8080) {
    bool new_cy = i8080->a & 0x01;
    i8080->a = (i8080->a >> 1) | (i8080->cy << 7);
    i8080->cy = new_cy;
}

void instr_dad(i8080_t* i8080, uint16_t register_value) {
    uint16_t hl_register = i8080_hl(i8080);
    uint32_t result = hl_register + register_value;
    i8080->cy = (result & 0x00010000) != 0;
    i8080_set_hl(i8080, result & 0xffff);
}

uint16_t instr_inx(i8080_t* i8080, uint16_t register_value) {
    return register_value + 1;
}

uint16_t instr_dcx(i8080_t* i8080, uint16_t register_value) {
    return register_value - 1;
}

void instr_xchg(i8080_t* i8080) {
    uint8_t tmp_h = i8080->h, tmp_l = i8080->l;
    i8080->h = i8080->d;
    i8080->l = i8080->e;
    i8080->d = tmp_h;
    i8080->e = tmp_l;
}

void instr_xthl(i8080_t* i8080) {
    uint8_t tmp_h = i8080->h, tmp_l = i8080->l;
    i8080->h = i8080->read_byte(i8080->sp + 1);
    i8080->l = i8080->read_byte(i8080->sp);
    i8080->write_byte(i8080->sp + 1, tmp_h);
    i8080->write_byte(i8080->sp, tmp_l);
}

void instr_push_bdh(i8080_t* i8080, uint16_t register_value) {
    i8080->write_byte(i8080->sp - 1, (register_value & 0xff00) >> 8);
    i8080->write_byte(i8080->sp - 2, register_value & 0x00ff);
    i8080->sp -= 2;
}

void instr_push_psw(i8080_t* i8080) {
    i8080->write_byte(i8080->sp - 1, i8080->a);
    i8080->write_byte(i8080->sp - 2, i8080_psw(i8080));
    i8080->sp -= 2;
}

uint16_t instr_pop_bdh(i8080_t* i8080) {
    uint16_t result = (i8080->read_byte(i8080->sp + 1) << 8) | i8080->read_byte(i8080->sp);
    i8080->sp += 2;
    return result;
}

void instr_pop_psw(i8080_t* i8080) {
    i8080_set_psw(i8080, i8080->read_byte(i8080->sp));
    i8080->a = i8080->read_byte(i8080->sp + 1);
    i8080->sp += 2;
}

uint16_t instr_lxi(i8080_t* i8080) {
    return (i8080->read_byte(i8080->pc + 2) << 8) | i8080->read_byte(i8080->pc + 1);
}

void instr_sta(i8080_t* i8080) {
    uint16_t address = (i8080->read_byte(i8080->pc + 2) << 8) | i8080->read_byte(i8080->pc + 1);
    i8080->write_byte(address, i8080->a);
}

uint16_t instr_lda(i8080_t* i8080) {
    return (i8080->read_byte(i8080->pc + 2) << 8) | i8080->read_byte(i8080->pc + 1);
}

void instr_shld(i8080_t* i8080) {
    uint16_t address = (i8080->read_byte(i8080->pc + 2) << 8) | i8080->read_byte(i8080->pc + 1);
    i8080->write_byte(address, i8080->l);
    i8080->write_byte(address + 1, i8080->h);
}

void instr_lhld(i8080_t* i8080) {
    uint16_t address = (i8080->read_byte(i8080->pc + 2) << 8) | i8080->read_byte(i8080->pc + 1);
    i8080->l = i8080->read_byte(address);
    i8080->h = i8080->read_byte(address + 1);
}

void instr_jmp(i8080_t* i8080) {
    i8080->pc = read_word(i8080);
}

void instr_call(i8080_t* i8080) {
    i8080->write_byte(i8080->sp - 1, (i8080->pc & 0xff00) >> 8);
    i8080->write_byte(i8080->sp - 2, i8080->pc & 0x00ff);
    i8080->sp -= 2;
    i8080->pc = read_word(i8080);
}

void instr_ret(i8080_t* i8080) {
    i8080->pc = (i8080->read_byte(i8080->sp + 1) << 8) | i8080->read_byte(i8080->sp);
    i8080->sp += 2;
}

uint16_t i8080_bc(i8080_t* i8080) {
    return (i8080->b << 8) | (i8080->c);
}

uint16_t i8080_de(i8080_t* i8080) {
    return (i8080->d << 8) | (i8080->e);
}

uint16_t i8080_hl(i8080_t* i8080) {
    return (i8080->h << 8) | (i8080->l);
}

/*
Flag register (PSW):
    Bit Position: 7  6  5  4  3  2  1  0
    Flag:         S  Z  0  AC 0  P  1  C
    
    S:  Sign flag.
    Z:  Zero flag.
    AC: Auxiliary Carry flag.
    P:  Parity flag.
    C:  Carry flag.
    Bits 5, 3 and 1 are set to 0, 0 and 1 repsectively.
*/

uint8_t i8080_psw(i8080_t* i8080) {
    return (i8080->s << 7) | (i8080->z << 6) | (0x0 << 5) | (i8080->ac << 4) |
           (0x0 << 3) | (i8080->p << 2) | (0x1 >> 1) | (i8080->cy);
}

uint16_t read_word(i8080_t* i8080) {
    return (i8080->read_byte(i8080->pc + 2) << 8) | i8080->read_byte(i8080->pc + 1);
}

void i8080_set_bc(i8080_t* i8080, uint16_t bc) {
    i8080->b = (bc & 0xff00) >> 8;
    i8080->c = (bc & 0x00ff);
}

void i8080_set_de(i8080_t* i8080, uint16_t de) {
    i8080->d = (de & 0xff00) >> 8;
    i8080->e = (de & 0x00ff);
}

void i8080_set_hl(i8080_t* i8080, uint16_t hl) {
    i8080->h = (hl & 0xff00) >> 8;
    i8080->l = (hl & 0x00ff);
}

void i8080_set_psw(i8080_t* i8080, uint8_t psw) {
    i8080->s = psw & 0x80;
    i8080->z = psw & 0x40;
    i8080->ac = psw & 0x10;
    i8080->p = psw & 0x04;
    i8080->cy = psw & 0x01;
}

void i8080_szp(i8080_t* i8080, uint8_t byte) {
    i8080->s = (byte & 0x80) != 0;
    i8080->z = byte == 0;
    i8080->p = parity(byte);
}

bool parity(uint8_t byte) {
    // if the number of 1s is even, parity is set
    // if the number of 1s is odd, parity is not set
    uint8_t number_of_ones = 0;
    for(int i = 0; i < 8; ++i)
        if((byte & (0x80 >> i)) != 0)
            number_of_ones++;
    return number_of_ones % 2 == 0;
}
