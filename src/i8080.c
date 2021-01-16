#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "i8080.h"

#ifdef DEBUG
    #define instr_printf(...) printf(__VA_ARGS__)
#else
    #define instr_printf(...)
#endif

// Register Getter/Setter Functions
static uint16_t read_word(i8080_t* i8080);
static uint16_t bc(i8080_t* i8080);
static uint16_t de(i8080_t* i8080);
static uint16_t hl(i8080_t* i8080);
static uint8_t flags(i8080_t* i8080);
static void set_bc(i8080_t* i8080, uint16_t bc);
static void set_de(i8080_t* i8080, uint16_t de);
static void set_hl(i8080_t* i8080, uint16_t hl);
static void set_flags(i8080_t* i8080, uint8_t flags);
static bool parity(uint8_t byte);
static void set_flags_szp(i8080_t* i8080, uint8_t byte);

// Instruction Function
static uint8_t instr_inr(i8080_t* i8080, uint8_t register_value);
static uint8_t instr_dcr(i8080_t* i8080, uint8_t register_value);
static void instr_daa(i8080_t* i8080);
static uint8_t instr_add(i8080_t* i8080, uint8_t register_value, bool include_carry);
static uint8_t instr_sub(i8080_t* i8080, uint8_t register_value, bool include_carry);
static uint8_t instr_ana(i8080_t* i8080, uint8_t register_value);
static uint8_t instr_xra(i8080_t* i8080, uint8_t register_value);
static uint8_t instr_ora(i8080_t* i8080, uint8_t register_value);
static void instr_rlc(i8080_t* i8080);
static void instr_rrc(i8080_t* i8080);
static void instr_ral(i8080_t* i8080);
static void instr_rar(i8080_t* i8080);
static void instr_push(i8080_t* i8080, uint16_t register_value);
static void instr_push_psw(i8080_t* i8080);
static uint16_t instr_pop(i8080_t* i8080);
static void instr_pop_psw(i8080_t* i8080);
static void instr_dad(i8080_t* i8080, uint16_t register_pair);
static void instr_xchg(i8080_t* i8080);
static void instr_xthl(i8080_t* i8080);
static void instr_shld(i8080_t* i8080);
static void instr_lhld(i8080_t* i8080);
static void instr_jmp(i8080_t* i8080, uint16_t address, bool condition);
static void instr_call(i8080_t* i8080, uint16_t address, bool condition);
static void instr_ret(i8080_t* i8080, bool condition);

i8080_t* init_i8080(uint16_t initial_pc) {
    i8080_t* i8080 = malloc(sizeof(i8080_t));
    i8080->a = 0x00;
    i8080->b = 0x00;
    i8080->c = 0x00;
    i8080->d = 0x00;
    i8080->e = 0x00;
    i8080->h = 0x00;
    i8080->l = 0x00;
    i8080->pc = initial_pc;
    i8080->sp = 0x0000;
    i8080->s = false;
    i8080->z = false;
    i8080->ac = false;
    i8080->p = false;
    i8080->cy = false;
    i8080->interrupt_enabled = false;
    return i8080;
}

void free_i8080(i8080_t* i8080) {
    if(i8080 == NULL) {
        return;
    }

    free(i8080);
}

void decode_i8080(i8080_t* i8080) {
    uint8_t opcode = i8080->read_byte(i8080->pc++);

    switch(opcode) {
        case 0x00: instr_printf("NOP"); break;

        // Carry Bit Instructions
        case 0x37: instr_printf("STC"); i8080->cy = true; break;
        case 0x3f: instr_printf("CMC"); i8080->cy = !i8080->cy; break;

        // Single Register Instructions
        case 0x3c: instr_printf("INR A"); i8080->a = instr_inr(i8080, i8080->a); break;
        case 0x04: instr_printf("INR B"); i8080->b = instr_inr(i8080, i8080->b); break;
        case 0x0c: instr_printf("INR C"); i8080->c = instr_inr(i8080, i8080->c); break;
        case 0x14: instr_printf("INR D"); i8080->d = instr_inr(i8080, i8080->d); break;
        case 0x1c: instr_printf("INR E"); i8080->e = instr_inr(i8080, i8080->e); break;
        case 0x24: instr_printf("INR H"); i8080->h = instr_inr(i8080, i8080->h); break;
        case 0x2c: instr_printf("INR L"); i8080->l = instr_inr(i8080, i8080->l); break;
        case 0x34: instr_printf("INR M"); i8080->write_byte(hl(i8080), instr_inr(i8080, i8080->read_byte(hl(i8080)))); break;

        case 0x3d: instr_printf("DCR A"); i8080->a = instr_dcr(i8080, i8080->a); break;
        case 0x05: instr_printf("DCR B"); i8080->b = instr_dcr(i8080, i8080->b); break;
        case 0x0d: instr_printf("DCR C"); i8080->c = instr_dcr(i8080, i8080->c); break;
        case 0x15: instr_printf("DCR D"); i8080->d = instr_dcr(i8080, i8080->d); break;
        case 0x1d: instr_printf("DCR E"); i8080->e = instr_dcr(i8080, i8080->e); break;
        case 0x25: instr_printf("DCR H"); i8080->h = instr_dcr(i8080, i8080->h); break;
        case 0x2d: instr_printf("DCR L"); i8080->l = instr_dcr(i8080, i8080->l); break;
        case 0x35: instr_printf("DCR M"); i8080->write_byte(hl(i8080), instr_dcr(i8080, i8080->read_byte(hl(i8080)))); break;

        case 0x2f: instr_printf("CMA"); i8080->a ^= 0xff ; break;
        case 0x27: instr_printf("DAA"); instr_daa(i8080); break;

        // Data Transfer Instructions
        case 0x7f: instr_printf("MOV A, A"); i8080->a = i8080->a; break;
        case 0x78: instr_printf("MOV A, B"); i8080->a = i8080->b; break;
        case 0x79: instr_printf("MOV A, C"); i8080->a = i8080->c; break;
        case 0x7a: instr_printf("MOV A, D"); i8080->a = i8080->d; break;
        case 0x7b: instr_printf("MOV A, E"); i8080->a = i8080->e; break;
        case 0x7c: instr_printf("MOV A, H"); i8080->a = i8080->h; break;
        case 0x7d: instr_printf("MOV A, L"); i8080->a = i8080->l; break;
        case 0x7e: instr_printf("MOV A, M"); i8080->a = i8080->read_byte(hl(i8080)); break;

        case 0x47: instr_printf("MOV B, A"); i8080->b = i8080->a; break;
        case 0x40: instr_printf("MOV B, B"); i8080->b = i8080->b; break;
        case 0x41: instr_printf("MOV B, C"); i8080->b = i8080->c; break;
        case 0x42: instr_printf("MOV B, D"); i8080->b = i8080->d; break;
        case 0x43: instr_printf("MOV B, E"); i8080->b = i8080->e; break;
        case 0x44: instr_printf("MOV B, H"); i8080->b = i8080->h; break;
        case 0x45: instr_printf("MOV B, L"); i8080->b = i8080->l; break;
        case 0x46: instr_printf("MOV B, M"); i8080->b = i8080->read_byte(hl(i8080)); break;

        case 0x4f: instr_printf("MOV C, A"); i8080->c = i8080->a; break;
        case 0x48: instr_printf("MOV C, B"); i8080->c = i8080->b; break;
        case 0x49: instr_printf("MOV C, C"); i8080->c = i8080->c; break;
        case 0x4a: instr_printf("MOV C, D"); i8080->c = i8080->d; break;
        case 0x4b: instr_printf("MOV C, E"); i8080->c = i8080->e; break;
        case 0x4c: instr_printf("MOV C, H"); i8080->c = i8080->h; break;
        case 0x4d: instr_printf("MOV C, L"); i8080->c = i8080->l; break;
        case 0x4e: instr_printf("MOV C, M"); i8080->c = i8080->read_byte(hl(i8080)); break;

        case 0x57: instr_printf("MOV D, A"); i8080->d = i8080->a; break;
        case 0x50: instr_printf("MOV D, B"); i8080->d = i8080->b; break;
        case 0x51: instr_printf("MOV D, C"); i8080->d = i8080->c; break;
        case 0x52: instr_printf("MOV D, D"); i8080->d = i8080->d; break;
        case 0x53: instr_printf("MOV D, E"); i8080->d = i8080->e; break;
        case 0x54: instr_printf("MOV D, H"); i8080->d = i8080->h; break;
        case 0x55: instr_printf("MOV D, L"); i8080->d = i8080->l; break;
        case 0x56: instr_printf("MOV D, M"); i8080->d = i8080->read_byte(hl(i8080)); break;

        case 0x5f: instr_printf("MOV E, A"); i8080->e = i8080->a; break;
        case 0x58: instr_printf("MOV E, B"); i8080->e = i8080->b; break;
        case 0x59: instr_printf("MOV E, C"); i8080->e = i8080->c; break;
        case 0x5a: instr_printf("MOV E, D"); i8080->e = i8080->d; break;
        case 0x5b: instr_printf("MOV E, E"); i8080->e = i8080->e; break;
        case 0x5c: instr_printf("MOV E, H"); i8080->e = i8080->h; break;
        case 0x5d: instr_printf("MOV E, L"); i8080->e = i8080->l; break;
        case 0x5e: instr_printf("MOV E, M"); i8080->e = i8080->read_byte(hl(i8080)); break;

        case 0x67: instr_printf("MOV H, A"); i8080->h = i8080->a; break;
        case 0x60: instr_printf("MOV H, B"); i8080->h = i8080->b; break;
        case 0x61: instr_printf("MOV H, C"); i8080->h = i8080->c; break;
        case 0x62: instr_printf("MOV H, D"); i8080->h = i8080->d; break;
        case 0x63: instr_printf("MOV H, E"); i8080->h = i8080->e; break;
        case 0x64: instr_printf("MOV H, H"); i8080->h = i8080->h; break;
        case 0x65: instr_printf("MOV H, L"); i8080->h = i8080->l; break;
        case 0x66: instr_printf("MOV H, M"); i8080->h = i8080->read_byte(hl(i8080)); break;

        case 0x6f: instr_printf("MOV L, A"); i8080->l = i8080->a; break;
        case 0x68: instr_printf("MOV L, B"); i8080->l = i8080->b; break;
        case 0x69: instr_printf("MOV L, C"); i8080->l = i8080->c; break;
        case 0x6a: instr_printf("MOV L, D"); i8080->l = i8080->d; break;
        case 0x6b: instr_printf("MOV L, E"); i8080->l = i8080->e; break;
        case 0x6c: instr_printf("MOV L, H"); i8080->l = i8080->h; break;
        case 0x6d: instr_printf("MOV L, L"); i8080->l = i8080->l; break;
        case 0x6e: instr_printf("MOV L, M"); i8080->l = i8080->read_byte(hl(i8080)); break;

        case 0x77: instr_printf("MOV M, A"); i8080->write_byte(hl(i8080), i8080->a); break;
        case 0x70: instr_printf("MOV M, B"); i8080->write_byte(hl(i8080), i8080->b); break;
        case 0x71: instr_printf("MOV M, C"); i8080->write_byte(hl(i8080), i8080->c); break;
        case 0x72: instr_printf("MOV M, D"); i8080->write_byte(hl(i8080), i8080->d); break;
        case 0x73: instr_printf("MOV M, E"); i8080->write_byte(hl(i8080), i8080->e); break;
        case 0x74: instr_printf("MOV M, H"); i8080->write_byte(hl(i8080), i8080->h); break;
        case 0x75: instr_printf("MOV M, L"); i8080->write_byte(hl(i8080), i8080->l); break;

        case 0x02: instr_printf("STAX B"); i8080->write_byte(bc(i8080), i8080->a); break;
        case 0x12: instr_printf("STAX D"); i8080->write_byte(de(i8080), i8080->a); break;

        case 0x0a: instr_printf("LDAX B"); i8080->a = i8080->read_byte(bc(i8080)); break;
        case 0x1a: instr_printf("LDAX D"); i8080->a = i8080->read_byte(de(i8080)); break;

        // Regiser or Memory to Accumulator Instructions
        case 0x87: instr_printf("ADD A"); i8080->a = instr_add(i8080, i8080->a, false); break;
        case 0x80: instr_printf("ADD B"); i8080->a = instr_add(i8080, i8080->b, false); break;
        case 0x81: instr_printf("ADD C"); i8080->a = instr_add(i8080, i8080->c, false); break;
        case 0x82: instr_printf("ADD D"); i8080->a = instr_add(i8080, i8080->d, false); break;
        case 0x83: instr_printf("ADD E"); i8080->a = instr_add(i8080, i8080->e, false); break;
        case 0x84: instr_printf("ADD H"); i8080->a = instr_add(i8080, i8080->h, false); break;
        case 0x85: instr_printf("ADD L"); i8080->a = instr_add(i8080, i8080->l, false); break;
        case 0x86: instr_printf("ADD M"); i8080->a = instr_add(i8080, i8080->read_byte(hl(i8080)), false); break;

        case 0x8f: instr_printf("ADC A"); i8080->a = instr_add(i8080, i8080->a, i8080->cy); break;
        case 0x88: instr_printf("ADC B"); i8080->a = instr_add(i8080, i8080->b, i8080->cy); break;
        case 0x89: instr_printf("ADC C"); i8080->a = instr_add(i8080, i8080->c, i8080->cy); break;
        case 0x8a: instr_printf("ADC D"); i8080->a = instr_add(i8080, i8080->d, i8080->cy); break;
        case 0x8b: instr_printf("ADC E"); i8080->a = instr_add(i8080, i8080->e, i8080->cy); break;
        case 0x8c: instr_printf("ADC H"); i8080->a = instr_add(i8080, i8080->h, i8080->cy); break;
        case 0x8d: instr_printf("ADC L"); i8080->a = instr_add(i8080, i8080->l, i8080->cy); break;
        case 0x8e: instr_printf("ADC M"); i8080->a = instr_add(i8080, i8080->read_byte(hl(i8080)), i8080->cy); break;

        case 0x97: instr_printf("SUB A"); i8080->a = instr_sub(i8080, i8080->a, false); break;
        case 0x90: instr_printf("SUB B"); i8080->a = instr_sub(i8080, i8080->b, false); break;
        case 0x91: instr_printf("SUB C"); i8080->a = instr_sub(i8080, i8080->c, false); break;
        case 0x92: instr_printf("SUB D"); i8080->a = instr_sub(i8080, i8080->d, false); break;
        case 0x93: instr_printf("SUB E"); i8080->a = instr_sub(i8080, i8080->e, false); break;
        case 0x94: instr_printf("SUB H"); i8080->a = instr_sub(i8080, i8080->h, false); break;
        case 0x95: instr_printf("SUB L"); i8080->a = instr_sub(i8080, i8080->l, false); break;
        case 0x96: instr_printf("SUB M"); i8080->a = instr_sub(i8080, i8080->read_byte(hl(i8080)), false); break;

        case 0x9f: instr_printf("SBB A"); i8080->a = instr_sub(i8080, i8080->a, i8080->cy); break;
        case 0x98: instr_printf("SBB B"); i8080->a = instr_sub(i8080, i8080->b, i8080->cy); break;
        case 0x99: instr_printf("SBB C"); i8080->a = instr_sub(i8080, i8080->c, i8080->cy); break;
        case 0x9a: instr_printf("SBB D"); i8080->a = instr_sub(i8080, i8080->d, i8080->cy); break;
        case 0x9b: instr_printf("SBB E"); i8080->a = instr_sub(i8080, i8080->e, i8080->cy); break;
        case 0x9c: instr_printf("SBB H"); i8080->a = instr_sub(i8080, i8080->h, i8080->cy); break;
        case 0x9d: instr_printf("SBB L"); i8080->a = instr_sub(i8080, i8080->l, i8080->cy); break;
        case 0x9e: instr_printf("SBB M"); i8080->a = instr_sub(i8080, i8080->read_byte(hl(i8080)), i8080->cy); break;

        case 0xa7: instr_printf("ANA A"); i8080->a = instr_ana(i8080, i8080->a); break;
        case 0xa0: instr_printf("ANA B"); i8080->a = instr_ana(i8080, i8080->b); break;
        case 0xa1: instr_printf("ANA C"); i8080->a = instr_ana(i8080, i8080->c); break;
        case 0xa2: instr_printf("ANA D"); i8080->a = instr_ana(i8080, i8080->d); break;
        case 0xa3: instr_printf("ANA E"); i8080->a = instr_ana(i8080, i8080->e); break;
        case 0xa4: instr_printf("ANA H"); i8080->a = instr_ana(i8080, i8080->h); break;
        case 0xa5: instr_printf("ANA L"); i8080->a = instr_ana(i8080, i8080->l); break;
        case 0xa6: instr_printf("ANA M"); i8080->a = instr_ana(i8080, i8080->read_byte(hl(i8080))); break;

        case 0xaf: instr_printf("XRA A"); i8080->a = instr_xra(i8080, i8080->a); break;
        case 0xa8: instr_printf("XRA B"); i8080->a = instr_xra(i8080, i8080->b); break;
        case 0xa9: instr_printf("XRA C"); i8080->a = instr_xra(i8080, i8080->c); break;
        case 0xaa: instr_printf("XRA D"); i8080->a = instr_xra(i8080, i8080->d); break;
        case 0xab: instr_printf("XRA E"); i8080->a = instr_xra(i8080, i8080->e); break;
        case 0xac: instr_printf("XRA H"); i8080->a = instr_xra(i8080, i8080->h); break;
        case 0xad: instr_printf("XRA L"); i8080->a = instr_xra(i8080, i8080->l); break;
        case 0xae: instr_printf("XRA M"); i8080->a = instr_xra(i8080, i8080->read_byte(hl(i8080))); break;

        case 0xb7: instr_printf("ORA A"); i8080->a = instr_ora(i8080, i8080->a); break;
        case 0xb0: instr_printf("ORA B"); i8080->a = instr_ora(i8080, i8080->b); break;
        case 0xb1: instr_printf("ORA C"); i8080->a = instr_ora(i8080, i8080->c); break;
        case 0xb2: instr_printf("ORA D"); i8080->a = instr_ora(i8080, i8080->d); break;
        case 0xb3: instr_printf("ORA E"); i8080->a = instr_ora(i8080, i8080->e); break;
        case 0xb4: instr_printf("ORA H"); i8080->a = instr_ora(i8080, i8080->h); break;
        case 0xb5: instr_printf("ORA L"); i8080->a = instr_ora(i8080, i8080->l); break;
        case 0xb6: instr_printf("ORA M"); i8080->a = instr_ora(i8080, i8080->read_byte(hl(i8080))); break;

        case 0xbf: instr_printf("CMP A"); instr_sub(i8080, i8080->a, i8080->cy); break;
        case 0xb8: instr_printf("CMP B"); instr_sub(i8080, i8080->b, i8080->cy); break;
        case 0xb9: instr_printf("CMP C"); instr_sub(i8080, i8080->c, i8080->cy); break;
        case 0xba: instr_printf("CMP D"); instr_sub(i8080, i8080->d, i8080->cy); break;
        case 0xbb: instr_printf("CMP E"); instr_sub(i8080, i8080->e, i8080->cy); break;
        case 0xbc: instr_printf("CMP H"); instr_sub(i8080, i8080->h, i8080->cy); break;
        case 0xbd: instr_printf("CMP L"); instr_sub(i8080, i8080->l, i8080->cy); break;
        case 0xbe: instr_printf("CMP M"); instr_sub(i8080, i8080->read_byte(hl(i8080)), i8080->cy); break;

        // Rotate Accumulator Instructions
        case 0x07: instr_printf("RLC"); instr_rlc(i8080); break;
        case 0x0f: instr_printf("RRC"); instr_rrc(i8080); break;
        case 0x17: instr_printf("RAL"); instr_ral(i8080); break;
        case 0x1f: instr_printf("RAR"); instr_rar(i8080); break;

        // Register Pair Instructions
        case 0xc5: instr_printf("PUSH B"); instr_push(i8080, bc(i8080)); break;
        case 0xd5: instr_printf("PUSH D"); instr_push(i8080, de(i8080)); break;
        case 0xe5: instr_printf("PUSH H"); instr_push(i8080, hl(i8080)); break;
        case 0xf5: instr_printf("PUSH PSW"); instr_push_psw(i8080); break;

        case 0xc1: instr_printf("POP B"); set_bc(i8080, instr_pop(i8080)); break;
        case 0xd1: instr_printf("POP D"); set_de(i8080, instr_pop(i8080)); break;
        case 0xe1: instr_printf("POP H"); set_hl(i8080, instr_pop(i8080)); break;
        case 0xf1: instr_printf("POP PSW"); instr_pop_psw(i8080); break;

        case 0x09: instr_printf("DAD B"); instr_dad(i8080, bc(i8080)); break;
        case 0x19: instr_printf("DAD D"); instr_dad(i8080, de(i8080)); break;
        case 0x29: instr_printf("DAD H"); instr_dad(i8080, hl(i8080)); break;
        case 0x39: instr_printf("DAD SP"); instr_dad(i8080, i8080->sp); break;

        case 0x03: instr_printf("INX B"); set_bc(i8080, bc(i8080) + 1); break;
        case 0x13: instr_printf("INX D"); set_de(i8080, de(i8080) + 1); break;
        case 0x23: instr_printf("INX H"); set_hl(i8080, hl(i8080) + 1); break;
        case 0x33: instr_printf("INX SP"); i8080->sp++; break;

        case 0x0b: instr_printf("DCX B"); set_bc(i8080, bc(i8080) - 1); break;
        case 0x1b: instr_printf("DCX D"); set_de(i8080, de(i8080) - 1); break;
        case 0x2b: instr_printf("DCX H"); set_hl(i8080, hl(i8080) - 1); break;
        case 0x3b: instr_printf("DCX SP"); i8080->sp--; break;

        case 0xeb: instr_printf("XCHG"); instr_xchg(i8080); break;
        case 0xe3: instr_printf("XTHL"); instr_xthl(i8080); break;
        case 0xf9: instr_printf("SPHL"); i8080->sp = hl(i8080); break;

        // Immediate Instructions
        case 0x01: instr_printf("LXI B, #0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); set_bc(i8080, read_word(i8080)); break;
        case 0x11: instr_printf("LXI D, #0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); set_de(i8080, read_word(i8080)); break;
        case 0x21: instr_printf("LXI H, #0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); set_hl(i8080, read_word(i8080)); break;
        case 0x31: instr_printf("LXI SP, #0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); i8080->sp = read_word(i8080); break;

        case 0x3e: instr_printf("MVI A, #0x%02x", i8080->read_byte(i8080->pc)); i8080->a = i8080->read_byte(i8080->pc++); break;
        case 0x06: instr_printf("MVI B, #0x%02x", i8080->read_byte(i8080->pc)); i8080->b = i8080->read_byte(i8080->pc++); break;
        case 0x0e: instr_printf("MVI C, #0x%02x", i8080->read_byte(i8080->pc)); i8080->c = i8080->read_byte(i8080->pc++); break;
        case 0x16: instr_printf("MVI D, #0x%02x", i8080->read_byte(i8080->pc)); i8080->d = i8080->read_byte(i8080->pc++); break;
        case 0x1e: instr_printf("MVI E, #0x%02x", i8080->read_byte(i8080->pc)); i8080->e = i8080->read_byte(i8080->pc++); break;
        case 0x26: instr_printf("MVI H, #0x%02x", i8080->read_byte(i8080->pc)); i8080->h = i8080->read_byte(i8080->pc++); break;
        case 0x2e: instr_printf("MVI L, #0x%02x", i8080->read_byte(i8080->pc)); i8080->l = i8080->read_byte(i8080->pc++); break;
        case 0x36: instr_printf("MVI M, #0x%02x", i8080->read_byte(i8080->pc)); i8080->write_byte(hl(i8080), i8080->read_byte(i8080->pc++)); break;

        case 0xc6: instr_printf("ADI #0x%02x", i8080->read_byte(i8080->pc)); i8080->a = instr_add(i8080, i8080->read_byte(i8080->pc++), false); break;
        case 0xce: instr_printf("ACI #0x%02x", i8080->read_byte(i8080->pc)); i8080->a = instr_add(i8080, i8080->read_byte(i8080->pc++), i8080->cy); break;
        case 0xd6: instr_printf("SUI #0x%02x", i8080->read_byte(i8080->pc)); i8080->a = instr_sub(i8080, i8080->read_byte(i8080->pc++), false); break;
        case 0xde: instr_printf("SBI #0x%02x", i8080->read_byte(i8080->pc)); i8080->a = instr_sub(i8080, i8080->read_byte(i8080->pc++), i8080->cy); break;
        case 0xe6: instr_printf("ANI #0x%02x", i8080->read_byte(i8080->pc)); i8080->a = instr_ana(i8080, i8080->read_byte(i8080->pc++)); break;
        case 0xee: instr_printf("XRI #0x%02x", i8080->read_byte(i8080->pc)); i8080->a = instr_xra(i8080, i8080->read_byte(i8080->pc++)); break;
        case 0xf6: instr_printf("ORI #0x%02x", i8080->read_byte(i8080->pc)); i8080->a = instr_ora(i8080, i8080->read_byte(i8080->pc++)); break;
        case 0xfe: instr_printf("CPI #0x%02x", i8080->read_byte(i8080->pc)); instr_sub(i8080, i8080->read_byte(i8080->pc++), false); break;

        // Direct Addressing Instructions
        case 0x32: instr_printf("STA 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); i8080->write_byte(read_word(i8080), i8080->a); break;
        case 0x3a: instr_printf("LDA 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); i8080->a = i8080->read_byte(read_word(i8080)); break;

        case 0x22: instr_printf("SHLD 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_shld(i8080); break;
        case 0x2a: instr_printf("LHLD 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_lhld(i8080); break;

        // Jump Instructions
        case 0xe9: instr_printf("PCHL"); i8080->pc = hl(i8080); break;
        case 0xc3: instr_printf("JMP 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), true); break;
        case 0xda: instr_printf("JC 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), i8080->cy); break;
        case 0xd2: instr_printf("JNC 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), !i8080->cy); break;
        case 0xca: instr_printf("JZ 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), i8080->z); break;
        case 0xc2: instr_printf("JNZ 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), !i8080->z); break;
        case 0xfa: instr_printf("JM 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), i8080->s); break;
        case 0xf2: instr_printf("JP 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), !i8080->s); break;
        case 0xea: instr_printf("JPE 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), i8080->p); break;
        case 0xe2: instr_printf("JPO 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_jmp(i8080, read_word(i8080), !i8080->p); break;

        // Call Subroutine Instructions
        case 0xcd: instr_printf("CALL 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), true); break;
        case 0xdc: instr_printf("CC 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), i8080->cy); break;
        case 0xd4: instr_printf("CNC 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), !i8080->cy); break;
        case 0xcc: instr_printf("CZ 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), i8080->z); break;
        case 0xc4: instr_printf("CNZ 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), !i8080->z); break;
        case 0xfc: instr_printf("CM 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), i8080->s); break;
        case 0xf4: instr_printf("CP 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), !i8080->s); break;
        case 0xec: instr_printf("CPE 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), i8080->p); break;
        case 0xe4: instr_printf("CPO 0x%02x%02x", i8080->read_byte(i8080->pc + 1), i8080->read_byte(i8080->pc)); instr_call(i8080, read_word(i8080), !i8080->p); break;

        // Return From Subroutine Instructions
        case 0xc9: instr_printf("RET"); instr_ret(i8080, true); break;
        case 0xd8: instr_printf("RC"); instr_ret(i8080, i8080->cy); break;
        case 0xd0: instr_printf("RNC"); instr_ret(i8080, !i8080->cy); break;
        case 0xc8: instr_printf("RZ"); instr_ret(i8080, i8080->z); break;
        case 0xc0: instr_printf("RNZ"); instr_ret(i8080, !i8080->z); break;
        case 0xf8: instr_printf("RM"); instr_ret(i8080, i8080->s); break;
        case 0xf0: instr_printf("RP"); instr_ret(i8080, !i8080->s); break;
        case 0xe8: instr_printf("RPE"); instr_ret(i8080, i8080->p); break;
        case 0xe0: instr_printf("RPO"); instr_ret(i8080, !i8080->p); break;

        // RST (Reset) Instructions
        case 0xc7: instr_printf("RST 0"); instr_call(i8080, 0x0000, true); break;
        case 0xcf: instr_printf("RST 1"); instr_call(i8080, 0x0008, true); break;
        case 0xd7: instr_printf("RST 2"); instr_call(i8080, 0x0010, true); break;
        case 0xdf: instr_printf("RST 3"); instr_call(i8080, 0x0018, true); break;
        case 0xe7: instr_printf("RST 4"); instr_call(i8080, 0x0020, true); break;
        case 0xef: instr_printf("RST 5"); instr_call(i8080, 0x0028, true); break;
        case 0xf7: instr_printf("RST 6"); instr_call(i8080, 0x0030, true); break;
        case 0xff: instr_printf("RST 7"); instr_call(i8080, 0x0038, true); break;

        // Interrupt Flip-Flop Instructions
        case 0xfb: instr_printf("EI"); i8080->interrupt_enabled = true; break;
        case 0xf3: instr_printf("DI"); i8080->interrupt_enabled = false; break;

        // Input/Output Instructions
        case 0xdb: instr_printf("IN #0x%02x", i8080->read_byte(i8080->pc)); break;
        case 0xd3: instr_printf("OUT #0x%02x", i8080->read_byte(i8080->pc)); break;

        // HLT (Halt) Instructions
        case 0x76: instr_printf("HLT"); i8080->pc--; break;

        // Other Instructions
        case 0x08: instr_printf("-"); break;
        case 0x10: instr_printf("-"); break;
        case 0x18: instr_printf("-"); break;
        case 0x20: instr_printf("-"); break;
        case 0x28: instr_printf("-"); break;
        case 0x30: instr_printf("-"); break;
        case 0x38: instr_printf("-"); break;
        case 0xcb: instr_printf("-"); break;
        case 0xd9: instr_printf("-"); instr_ret(i8080, true); break;
        case 0xdd: instr_printf("-"); break;
        case 0xed: instr_printf("-"); break;
        case 0xfd: instr_printf("-"); break;
    }
}

// Register Getter/Setter Functions
uint16_t read_word(i8080_t* i8080) {
    uint16_t word = i8080->read_byte(i8080->pc++);
    word = (i8080->read_byte(i8080->pc++) << 8) | word;
    return word;
}

uint16_t bc(i8080_t* i8080) {
    return (i8080->b << 8) | i8080->c;
}

uint16_t de(i8080_t* i8080) {
    return (i8080->d << 8) | i8080->e;
}

uint16_t hl(i8080_t* i8080) {
    return (i8080->h << 8) | i8080->l;
}

// Bit Position: 7  6  5  4  3  2  1  0
//               S  Z  0  AC 0  P  1  CY
uint8_t flags(i8080_t* i8080) {
    return (i8080->s << 7) | (i8080->z << 6) | (0x1 << 5) | (i8080->ac << 4) |
           (0x0 << 3) | (i8080->p << 2) | (0x1 << 1) | (i8080->cy);
}

void set_bc(i8080_t* i8080, uint16_t bc) {
    i8080->b = (bc & 0xff00) >> 8;
    i8080->c = bc & 0x00ff;
}

void set_de(i8080_t* i8080, uint16_t de) {
    i8080->d = (de & 0xff00) >> 8;
    i8080->e = de & 0x00ff;
}

void set_hl(i8080_t* i8080, uint16_t hl) {
    i8080->h = (hl & 0xff00) >> 8;
    i8080->l = hl & 0x00ff;
}

void set_flags(i8080_t* i8080, uint8_t flags) {
    i8080->s = (flags & 0x80) >> 7;
    i8080->z = (flags & 0x40) >> 6;
    i8080->ac = (flags & 0x10) >> 4;
    i8080->p = (flags & 0x04) >> 2;
    i8080->cy = flags & 0x01;
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

void set_flags_szp(i8080_t* i8080, uint8_t byte) {
    i8080->s = (byte & 0x80) != 0;
    i8080->z = byte == 0;
    i8080->p = parity(byte);
}

// Instruction Function
uint8_t instr_inr(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = register_value + 1;
    set_flags_szp(i8080, result);
    i8080->ac = (result & 0x0f) == 0;
    return result;
}

uint8_t instr_dcr(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = register_value - 1;
    set_flags_szp(i8080, result);
    i8080->ac = (result & 0x0f) != 0x0f;
    return result;
}

void instr_daa(i8080_t* i8080) {
    // Step 1:
    // If lower 4-bit of accumulator is greater than 0x09 or auxiliary carry is set
    // add 0x06 to the lower 4-bit number. Auxiliary carry is affected by this step.
    // Step 2:
    // If upper 4-bit of the resulting accumulator is greater than 0x09 or carry is set
    // add 0x06 to the upper 4-bit number. Carry is affected by the step.
    // 
    // The carry and auxiliary carry flags are affected by the upper and lower 4-bits
    // operations repsectivley, so is like a normal addition to the accumulator by the
    // number to add (either 0x00, 0x06, 0x60 or 0x66) and the flags will be affected
    // like any add instruction.
    
    uint8_t add_value = 0x00;
    uint8_t lower_nibble = i8080->a & 0x0f;
    if(lower_nibble > 0x09 || i8080->ac) {
        add_value += 0x06;
    }

    uint8_t upper_nibble = ((i8080->a + add_value) & 0xf0) >> 4;
    if(upper_nibble > 0x09 || i8080->cy) {
        add_value += 0x60;
    }

    instr_add(i8080, add_value, false);
}

uint8_t instr_add(i8080_t* i8080, uint8_t register_value, bool include_carry) {
    uint16_t result = i8080->a + register_value + include_carry;
    set_flags_szp(i8080, result & 0xff);

    i8080->cy = (result & 0x0100) != 0;
    
    uint8_t aux_byte = (i8080->a & 0x0f) + (register_value & 0x0f) + include_carry;
    i8080->ac = (aux_byte & 0x10) != 0;

    return result & 0xff;
}

uint8_t instr_sub(i8080_t* i8080, uint8_t register_value, bool include_carry) {
    uint16_t result = i8080->a - register_value;
    set_flags_szp(i8080, result & 0xff);

    i8080->cy = (result & 0x0100) != 0;
    
    uint8_t aux_byte = (i8080->a & 0x0f) - (register_value & 0x0f) - include_carry;
    i8080->ac = (aux_byte & 0x10) != 0;

    return result & 0xff;
}

uint8_t instr_ana(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = i8080->a & register_value;
    set_flags_szp(i8080, result & 0xff);
    i8080->cy = false;
    i8080->ac = false;
    return result;
}

uint8_t instr_xra(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = i8080->a ^ register_value;
    set_flags_szp(i8080, result & 0xff);
    i8080->cy = false;
    i8080->ac = false;
    return result;
}

uint8_t instr_ora(i8080_t* i8080, uint8_t register_value) {
    uint8_t result = i8080->a | register_value;
    set_flags_szp(i8080, result & 0xff);
    i8080->cy = false;
    i8080->ac = false;
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

void instr_push(i8080_t* i8080, uint16_t register_value) {
    i8080->write_byte(--i8080->sp, (register_value & 0xff00) >> 8);
    i8080->write_byte(--i8080->sp, register_value & 0x00ff);
}

uint16_t instr_pop(i8080_t* i8080) {
    uint16_t address = i8080->read_byte(i8080->sp++);
    address = (i8080->read_byte(i8080->sp++) << 8) | address;
    return address;
}

void instr_push_psw(i8080_t* i8080) {
    i8080->write_byte(--i8080->sp, i8080->a);
    i8080->write_byte(--i8080->sp, flags(i8080));
}

void instr_pop_psw(i8080_t* i8080) {
    set_flags(i8080, i8080->read_byte(i8080->sp++));
    i8080->a = i8080->read_byte(i8080->sp++);
}

void instr_dad(i8080_t* i8080, uint16_t register_pair) {
    unsigned int result = hl(i8080) + register_pair;
    i8080->cy = (result & 0x00010000) != 0;
    set_hl(i8080, result & 0xffff);
}

void instr_xchg(i8080_t* i8080) {
    uint16_t temp_hl = hl(i8080);
    set_hl(i8080, de(i8080));
    set_de(i8080, temp_hl);
}

void instr_xthl(i8080_t* i8080) {
    uint16_t temp_sp = instr_pop(i8080);
    instr_push(i8080, hl(i8080));
    set_hl(i8080, temp_sp);
}

void instr_shld(i8080_t* i8080) {
    uint16_t address = read_word(i8080);
    i8080->write_byte(address, i8080->l);
    i8080->write_byte(address + 1, i8080->h);
}

void instr_lhld(i8080_t* i8080) {
    uint16_t address = read_word(i8080);
    i8080->l = i8080->read_byte(address);
    i8080->h = i8080->read_byte(address + 1);
}

void instr_jmp(i8080_t* i8080, uint16_t address, bool condition) {
    if(condition) {
        i8080->pc = address;
    }
}

void instr_call(i8080_t* i8080, uint16_t address, bool condition) {
    if(condition) {
        instr_push(i8080, i8080->pc);
        instr_jmp(i8080, address, true);
    }
}

void instr_ret(i8080_t* i8080, bool condition) {
    if(condition) {
        i8080->pc = instr_pop(i8080);
    }
}
