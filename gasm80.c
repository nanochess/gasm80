/*
 ** Generic Assembler Z80 (gasm80)
 **
 ** by Oscar Toledo G.
 ** https://nanochess.org/
 **
 ** Creation date: Mar/03/2024. Based on tinyasm. Fully functional.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

/*#define DEBUG*/

char *input_filename;
int line_number;

char *output_filename;
FILE *output;
int output_address;

char *listing_filename;
FILE *listing;

enum {CPU_Z80, CPU_6502} processor;
int assembler_step;
int default_start_address;
int start_address;
int address;
int first_time;

int instruction_offset;

int instruction_register;
int instruction_register_2;

int instruction_value;
int instruction_value2;

#define MAX_SIZE        4096

char line[MAX_SIZE];
char part[MAX_SIZE];
char name[MAX_SIZE];
char expr_name[MAX_SIZE];
char undefined_name[MAX_SIZE];
char global_label[MAX_SIZE];
char included_file[MAX_SIZE];
char *prev_p;
char *p;

char *g;
char generated[8];

int errors;
int warnings;
int bytes;
int change;
int change_number;

struct label {
    struct label *left;
    struct label *right;
    int value;
    char name[1];
};

struct label *label_list;
struct label *last_label;
int undefined;

/*
 ** Instructions using less bytes should appear first.
 */
char *cpu_6502_instruction_set[] = {
    "BRX",      "",             "x00",
    "BPL",      "%a8",          "x10 %a8",
    "JSR",      "%i16",         "x20 %i16",
    "BMI",      "%a8",          "x30 %a8",
    "RTI",      "",             "x40",
    "BVC",      "%a8",          "x50 %a8",
    "RTS",      "",             "x60",
    "BVS",      "%a8",          "x70 %a8",
    "BCC",      "%a8",          "x90 %a8",
    "LDY",      "#%i8",         "xa0 %i8",
    "LDX",      "#%i8",         "xa2 %i8",
    "BCS",      "%a8",          "xb0 %a8",
    "CPY",      "#%i8",         "xc0 %i8",
    "BNE",      "%a8",          "xd0 %a8",
    "CPX",      "#%i8",         "xe0 %i8",
    "BEQ",      "%a8",          "xf0 %a8",
    "ORA",      "(%i8,X)",      "x01 %i8",
    "ORA",      "(%i8),Y",      "x11 %i8",
    "AND",      "(%i8,X)",      "x21 %i8",
    "AND",      "(%i8),Y",      "x31 %i8",
    "EOR",      "(%i8,X)",      "x41 %i8",
    "EOR",      "(%i8),Y",      "x51 %i8",
    "ADC",      "(%i8,X)",      "x61 %i8",
    "ADC",      "(%i8),Y",      "x71 %i8",
    "STA",      "(%i8,X)",      "x81 %i8",
    "STA",      "(%i8),Y",      "x91 %i8",
    "LDA",      "(%i8,X)",      "xa1 %i8",
    "LDA",      "(%i8),Y",      "xb1 %i8",
    "CMP",      "(%i8,X)",      "xc1 %i8",
    "CMP",      "(%i8),Y",      "xd1 %i8",
    "SBC",      "(%i8,X)",      "xe1 %i8",
    "SBC",      "(%i8),Y",      "xf1 %i8",
    "BIT",      "%i8",          "x24 %i8",
    "STY",      "%i8",          "x84 %i8",
    "STY",      "%i8,X",        "x94 %i8",
    "LDY",      "%i8",          "xa4 %i8",
    "LDY",      "%i8,X",        "xb4 %i8",
    "CPY",      "%i8",          "xc4 %i8",
    "CPX",      "%i8",          "xe4 %i8",
    "ORA",      "%i8",          "x05 %i8",
    "ORA",      "%i8,X",        "x15 %i8",
    "AND",      "%i8",          "x25 %i8",
    "AND",      "%i8,X",        "x35 %i8",
    "EOR",      "%i8",          "x45 %i8",
    "EOR",      "%i8,X",        "x55 %i8",
    "ADC",      "%i8",          "x65 %i8",
    "ADC",      "%i8,X",        "x75 %i8",
    "STA",      "%i8",          "x85 %i8",
    "STA",      "%i8,X",        "x95 %i8",
    "LDA",      "%i8",          "xa5 %i8",
    "LDA",      "%i8,X",        "xb5 %i8",
    "CMP",      "%i8",          "xc5 %i8",
    "CMP",      "%i8,X",        "xd5 %i8",
    "SBC",      "%i8",          "xe5 %i8",
    "SBC",      "%i8,X",        "xf5 %i8",
    "ASL",      "%i8",          "x06 %i8",
    "ASL",      "%i8,X",        "x16 %i8",
    "ROL",      "%i8",          "x26 %i8",
    "ROL",      "%i8,X",        "x36 %i8",
    "LSR",      "%i8",          "x46 %i8",
    "LSR",      "%i8,X",        "x56 %i8",
    "ROR",      "%i8",          "x66 %i8",
    "ROR",      "%i8,X",        "x76 %i8",
    "STX",      "%i8",          "x86 %i8",
    "STX",      "%i8,X",        "x96 %i8",
    "LDX",      "%i8",          "xa6 %i8",
    "LDX",      "%i8,X",        "xb6 %i8",
    "DEC",      "%i8",          "xc6 %i8",
    "DEC",      "%i8,X",        "xd6 %i8",
    "INC",      "%i8",          "xe6 %i8",
    "INC",      "%i8,X",        "xf6 %i8",
    "PHP",      "",             "x08",
    "CLC",      "",             "x18",
    "PLP",      "",             "x28",
    "SEC",      "",             "x38",
    "PHA",      "",             "x48",
    "CLI",      "",             "x58",
    "PLA",      "",             "x68",
    "SEI",      "",             "x78",
    "DEY",      "",             "x88",
    "TYA",      "",             "x98",
    "TAY",      "",             "xa8",
    "CLV",      "",             "xb8",
    "INY",      "",             "xc8",
    "CLD",      "",             "xd8",
    "INX",      "",             "xe8",
    "SED",      "",             "xf8",
    "ORA",      "#%i8",         "x09 %i8",
    "ORA",      "%i16,Y",       "x19 %i16",
    "AND",      "#%i8",         "x29 %i8",
    "AND",      "%i16,Y",       "x39 %i16",
    "EOR",      "#%i8",         "x49 %i8",
    "EOR",      "%i16,Y",       "x59 %i16",
    "ADC",      "#%i8",         "x69 %i8",
    "ADC",      "%i16,Y",       "x79 %i16",
    "STA",      "%i16,Y",       "x99 %i16",
    "LDA",      "#%i8",         "xa9 %i8",
    "LDA",      "%i16,Y",       "xb9 %i16",
    "CMP",      "#%i8",         "xc9 %i8",
    "CMP",      "%i16,Y",       "xd9 %i16",
    "SBC",      "#%i8",         "xe9 %i8",
    "SBC",      "%i16,Y",       "xf9 %i16",
    "ASL",      "A",            "x0a",
    "ROL",      "A",            "x2a",
    "LSR",      "A",            "x4a",
    "ROR",      "A",            "x6a",
    "TXA",      "",             "x8a",
    "TXS",      "",             "x9a",
    "TAX",      "",             "xaa",
    "TSX",      "",             "xba",
    "DEX",      "",             "xca",
    "NOP",      "",             "xea",
    "BIT",      "%i16",         "x2c %i16",
    "JMP",      "(%i16)",       "x6c %i16",
    "JMP",      "%i16",         "x4c %i16",
    "STY",      "%i16",         "x8c %i16",
    "LDY",      "%i16",         "xac %i16",
    "LDY",      "%i16,X",       "xbc %i16",
    "CPY",      "%i16",         "xcc %i16",
    "CPX",      "%i16",         "xec %i16",
    "ORA",      "%i16",         "x0d %i16",
    "ORA",      "%i16,X",       "x1d %i16",
    "AND",      "%i16",         "x2d %i16",
    "AND",      "%i16,X",       "x3d %i16",
    "EOR",      "%i16",         "x4d %i16",
    "EOR",      "%i16,X",       "x5d %i16",
    "ADC",      "%i16",         "x6d %i16",
    "ADC",      "%i16,X",       "x7d %i16",
    "STA",      "%i16",         "x8d %i16",
    "STA",      "%i16,X",       "x9d %i16",
    "LDA",      "%i16",         "xad %i16",
    "LDA",      "%i16,X",       "xbd %i16",
    "CMP",      "%i16",         "xcd %i16",
    "CMP",      "%i16,X",       "xdd %i16",
    "SBC",      "%i16",         "xed %i16",
    "SBC",      "%i16,X",       "xfd %i16",
    "ASL",      "%i16",         "x0e %i16",
    "ASL",      "%i16,X",       "x1e %i16",
    "ROL",      "%i16",         "x2e %i16",
    "ROL",      "%i16,X",       "x3e %i16",
    "LSR",      "%i16",         "x4e %i16",
    "LSR",      "%i16,X",       "x5e %i16",
    "ROR",      "%i16",         "x6e %i16",
    "ROR",      "%i16,X",       "x7e %i16",
    "STX",      "%i16",         "x8e %i16",
    "LDX",      "%i16",         "xae %i16",
    "LDX",      "%i16,Y",       "xbe %i16",
    "DEC",      "%i16",         "xce %i16",
    "DEC",      "%i16,X",       "xde %i16",
    "INC",      "%i16",         "xee %i16",
    "INC",      "%i16,X",       "xfe %i16",
    NULL,       NULL,           NULL,
};

char *cpu_z80_instruction_set[] = {
    "NOP",      "",             "x00",
    "LD",       "%t8,(IX%d8)",  "xdd b01%t8110 %d8",
    "LD",       "%t8,(IY%d8)",  "xfd b01%t8110 %d8",
    "LD",       "HL,(%i16)",    "x2a %i16",
    "LD",       "A,(%i16)",     "x3a %i16",
    "LD",       "(IX%d8),%t8",  "xdd b01110%t8 %d8",
    "LD",       "(IY%d8),%t8",  "xfd b01110%t8 %d8",
    "LD",       "(IX%d8),%i8",  "xdd b00110110 %d8 %i8",
    "LD",       "(IY%d8),%i8",  "xfd b00110110 %d8 %i8",
    "LD",       "(BC),A",       "x02",
    "INC",      "%r16",         "b00%r160011",
    "INC",      "%r8",          "b00%r8100",
    "DEC",      "%r8",          "b00%r8101",
    "DEC",      "%r16",         "b00%r161011",
    "RLCA",     "",             "x07",
    "EX",       "AF,AF'",       "x08",
    "ADD",      "HL,%r16",      "b00%r161001",
    "LD",       "A,(BC)",       "x0a",
    "RRCA",     "",             "x0f",
    "DJNZ",     "%a8",          "x10 %a8",
    "LD",       "(DE),A",       "x12",
    "RLA",      "",             "x17",
    "RRA",      "",             "x1f",
    "JR",       "%a8",          "x18 %a8",
    "LD",       "A,(DE)",       "x1a",
    "JR",       "NZ,%a8",       "x20 %a8",
    "LD",       "(%i16),HL",    "x22 %i16",
    "DAA",      "",             "x27",
    "JR",       "Z,%a8",        "x28 %a8",
    "CPL",      "",             "x2f",
    "JR",       "NC,%a8",       "x30 %a8",
    "LD",       "(%i16),A",     "x32 %i16",
    "SCF",      "",             "x37",
    "JR",       "C,%a8",        "x38 %a8",
    "CCF",      "",             "x3f",
    "HALT",     "",             "x76",
    "RET",      "%f",           "b11%f000",
    "POP",      "%s16",         "b11%s160001",
    "JP",       "%f,%i16",      "b11%f010 %i16",
    "CALL",     "%f,%i16",      "b11%f100 %i16",
    "PUSH",     "%s16",         "b11%s160101",
    "RST",      "%b8",          "b11%b8111",
    "RET",      "",             "xc9",
    "RLC",      "%r8",          "xcb b00000%r8",
    "RRC",      "%r8",          "xcb b00001%r8",
    "RL",       "%r8",          "xcb b00010%r8",
    "RR",       "%r8",          "xcb b00011%r8",
    "SLA",      "%r8",          "xcb b00100%r8",
    "SRA",      "%r8",          "xcb b00101%r8",
    "SRL",      "%r8",          "xcb b00111%r8",
    "BIT",      "%c,%r8",       "xcb b01%c%r8",
    "RES",      "%c,%r8",       "xcb b10%c%r8",
    "SET",      "%c,%r8",       "xcb b11%c%r8",
    "CALL",     "%i16",         "xcd %i16",
    "OUT",      "(%i8),A",      "xd3 %i8",
    "EXX",      "",             "xd9",
    "IN",       "A,(%i8)",      "xdb %i8",
    "ADD",      "IX,BC",        "xdd x09",
    "ADD",      "IX,DE",        "xdd x19",
    "LD",       "IX,(%i16)",    "xdd x2a %i16",
    "LD",       "IX,%i16",      "xdd x21 %i16",
    "LD",       "(%i16),IX",    "xdd x22 %i16",
    "INC",      "IX",           "xdd x23",
    "ADD",      "IX,IX",        "xdd x29",
    "DEC",      "IX",           "xdd x2b",
    "ADD",      "IX,SP",        "xdd x39",
    "ADD",      "A,(IX%d8)",    "xdd x86 %d8",
    "ADC",      "A,(IX%d8)",    "xdd x8e %d8",
    "SUB",      "(IX%d8)",      "xdd x96 %d8",
    "SBC",      "A,(IX%d8)",    "xdd x9e %d8",
    "AND",      "(IX%d8)",      "xdd xa6 %d8",
    "XOR",      "(IX%d8)",      "xdd xae %d8",
    "OR",       "(IX%d8)",      "xdd xb6 %d8",
    "CP",       "(IX%d8)",      "xdd xbe %d8",
    "RLC",      "(IX%d8)",      "xdd xcb %d8 x06",
    "RRC",      "(IX%d8)",      "xdd xcb %d8 x0e",
    "RL",       "(IX%d8)",      "xdd xcb %d8 x16",
    "RR",       "(IX%d8)",      "xdd xcb %d8 x1e",
    "SLA",      "(IX%d8)",      "xdd xcb %d8 x26",
    "SRA",      "(IX%d8)",      "xdd xcb %d8 x2e",
    "SRL",      "(IX%d8)",      "xdd xcb %d8 x3e",
    "BIT",      "%c,(IX%d8)",   "xdd xcb %d8 b01%c110",
    "RES",      "%c,(IX%d8)",   "xdd xcb %d8 b10%c110",
    "SET",      "%c,(IX%d8)",   "xdd xcb %d8 b11%c110",
    "POP",      "IX",           "xdd xe1",
    "EX",       "(SP),IX",      "xdd xe3",
    "PUSH",     "IX",           "xdd xe5",
    "JP",       "(IX)",         "xdd xe9",
    "LD",       "SP,IX",        "xdd xf9",
    "EX",       "(SP),HL",      "xe3",
    "JP",       "(HL)",         "xe9",
    "EX",       "DE,HL",        "xeb",
    "IN",       "%s8,(C)",      "xed b01%s8000",
    "OUT",      "(C),%s8",      "xed b01%s8001",
    "SBC",      "HL,%r16",      "xed b01%r160010",
    "LD",       "(%i16),%r16",  "xed b01%r160011 %i16",
    "NEG",      "",             "xed x44",
    "RETN",     "",             "xed x45",
    "IM",       "0",            "xed x46",
    "LD",       "I,A",          "xed x47",
    "ADC",      "HL,%r16",      "xed b01%r161010",
    "LD",       "%r16,(%i16)",  "xed b01%r161011 %i16",
    "RETI",     "",             "xed x4d",
    "LD",       "R,A",          "xed x4f",
    "IM",       "1",            "xed x56",
    "LD",       "A,I",          "xed x57",
    "IM",       "2",            "xed x5e",
    "LD",       "A,R",          "xed x5f",
    "RRD",      "",             "xed x67",
    "RLD",      "",             "xed x6f",
    "LDI",      "",             "xed xa0",
    "CPI",      "",             "xed xa1",
    "INI",      "",             "xed xa2",
    "OUTI",     "",             "xed xa3",
    "LDD",      "",             "xed xa8",
    "CPD",      "",             "xed xa9",
    "IND",      "",             "xed xaa",
    "OUTD",     "",             "xed xab",
    "LDIR",     "",             "xed xb0",
    "CPIR",     "",             "xed xb1",
    "INIR",     "",             "xed xb2",
    "OTIR",     "",             "xed xb3",
    "LDDR",     "",             "xed xb8",
    "CPDR",     "",             "xed xb9",
    "INDR",     "",             "xed xba",
    "OTDR",     "",             "xed xbb",
    "DI",       "",             "xf3",
    "LD",       "SP,HL",        "xf9",
    "EI",       "",             "xfb",
    "ADD",      "IY,BC",        "xfd x09",
    "ADD",      "IY,DE",        "xfd x19",
    "LD",       "IY,(%i16)",    "xfd x2a %i16",
    "LD",       "IY,%i16",      "xfd x21 %i16",
    "LD",       "(%i16),IY",    "xfd x22 %i16",
    "INC",      "IY",           "xfd x23",
    "ADD",      "IY,IY",        "xfd x29",
    "DEC",      "IY",           "xfd x2b",
    "ADD",      "IY,SP",        "xfd x39",
    "ADD",      "A,(IY%d8)",    "xfd x86 %d8",
    "ADC",      "A,(IY%d8)",    "xfd x8e %d8",
    "SUB",      "(IY%d8)",      "xfd x96 %d8",
    "SBC",      "A,(IY%d8)",    "xfd x9e %d8",
    "AND",      "(IY%d8)",      "xfd xa6 %d8",
    "XOR",      "(IY%d8)",      "xfd xae %d8",
    "OR",       "(IY%d8)",      "xfd xb6 %d8",
    "CP",       "(IY%d8)",      "xfd xbe %d8",
    "RLC",      "(IY%d8)",      "xfd xcb %d8 x06",
    "RRC",      "(IY%d8)",      "xfd xcb %d8 x0e",
    "RL",       "(IY%d8)",      "xfd xcb %d8 x16",
    "RR",       "(IY%d8)",      "xfd xcb %d8 x1e",
    "SLA",      "(IY%d8)",      "xfd xcb %d8 x26",
    "SRA",      "(IY%d8)",      "xfd xcb %d8 x2e",
    "SRL",      "(IY%d8)",      "xfd xcb %d8 x3e",
    "BIT",      "%c,(IY%d8)",   "xfd xcb %d8 b01%c110",
    "RES",      "%c,(IY%d8)",   "xfd xcb %d8 b10%c110",
    "SET",      "%c,(IY%d8)",   "xfd xcb %d8 b11%c110",
    "POP",      "IY",           "xfd xe1",
    "EX",       "(SP),IY",      "xfd xe3",
    "PUSH",     "IY",           "xfd xe5",
    "JP",       "(IY)",         "xfd xe9",
    "LD",       "SP,IY",        "xfd xf9",
    "LD",       "%r8,%i8",      "b00%r8110 %i8",
    "LD",       "%r8,%t8",      "b01%r8%t8",
    "LD",       "%t8,%r8",      "b01%t8%r8",
    "ADD",      "A,%r8",        "b10000%r8",
    "ADC",      "A,%r8",        "b10001%r8",
    "SUB",      "%r8",          "b10010%r8",
    "SBC",      "A,%r8",        "b10011%r8",
    "AND",      "%r8",          "b10100%r8",
    "XOR",      "%r8",          "b10101%r8",
    "OR",       "%r8",          "b10110%r8",
    "CP",       "%r8",          "b10111%r8",
    "ADD",      "A,%i8",        "xc6 %i8",
    "ADC",      "A,%i8",        "xce %i8",
    "SUB",      "%i8",          "xd6 %i8",
    "SBC",      "A,%i8",        "xde %i8",
    "AND",      "%i8",          "xe6 %i8",
    "XOR",      "%i8",          "xee %i8",
    "OR",       "%i8",          "xf6 %i8",
    "CP",       "%i8",          "xfe %i8",
    "JP",       "%i16",         "xc3 %i16",
    "LD",       "%r16,%i16",    "b00%r160001 %i16",
    NULL,       NULL,           NULL,
};

char *regs[] = {
    "B",
    "C",
    "D",
    "E",
    "H",
    "L",
    "(HL)",
    "A",
    
    "BC",
    "DE",
    "HL",
    "SP",
    
    "BC",
    "DE",
    "HL",
    "AF",
    
    "NZ",
    "Z",
    "NC",
    "C",
    "PO",
    "PE",
    "P",
    "M",
};

void message();
char *match_register(char *, int, int *);
char *match_expression(char *, int *);
char *match_expression_level1(char *, int *);
char *match_expression_level2(char *, int *);
char *match_expression_level3(char *, int *);
char *match_expression_level4(char *, int *);
char *match_expression_level5(char *, int *);
char *match_expression_level6(char *, int *);

/*
 ** Define a new label
 */
struct label *define_label(char *name, int value)
{
    struct label *label;
    struct label *explore;
    int c;
    
    /* Allocate label */
    label = malloc(sizeof(struct label) + strlen(name));
    if (label == NULL) {
        fprintf(stderr, "Out of memory for label\n");
        exit(1);
        return NULL;
    }
    
    /* Fill label */
    label->left = NULL;
    label->right = NULL;
    label->value = value;
    strcpy(label->name, name);
    
    /* Populate binary tree */
    if (label_list == NULL) {
        label_list = label;
    } else {
        explore = label_list;
        while (1) {
            c = strcmp(label->name, explore->name);
            if (c < 0) {
                if (explore->left == NULL) {
                    explore->left = label;
                    break;
                }
                explore = explore->left;
            } else if (c > 0) {
                if (explore->right == NULL) {
                    explore->right = label;
                    break;
                }
                explore = explore->right;
            }
        }
    }
    return label;
}

/*
 ** Find a label
 */
struct label *find_label(char *name)
{
    struct label *explore;
    int c;
    
    /* Follows a binary tree */
    explore = label_list;
    while (explore != NULL) {
        c = strcmp(name, explore->name);
        if (c == 0)
            return explore;
        if (c < 0)
            explore = explore->left;
        else
            explore = explore->right;
    }
    return NULL;
}

/*
 ** Sort recursively labels (already done by binary tree)
 */
void sort_labels(struct label *node)
{
    if (node->left != NULL)
        sort_labels(node->left);
    fprintf(listing, "%-20s %04x\n", node->name, node->value);
    if (node->right != NULL)
        sort_labels(node->right);
}

/*
 ** Avoid spaces in input
 */
char *avoid_spaces(char *p)
{
    while (isspace(*p))
        p++;
    return p;
}

/*
 ** Check for a label character
 */
int islabel(int c)
{
    return isalpha(c) || isdigit(c) || c == '_' || c == '.' || c == '#';
}

/*
 ** Match register
 */
char *match_register(char *p, int type, int *value)
{
    char reg[5];
    int c;
    int base;
    int length;
    
    p = avoid_spaces(p);
    if (type == 0) {    /* 8-bit */
        if (p[0] == '(' && p[1] == 'H' && p[2] == 'L' && p[3] == ')') {
            memcpy(reg, p, 4);
            reg[4] = '\0';
            p += 4;
        } else if (!isalpha(p[0]) || islabel(p[1])) {
            return NULL;
        } else {
            reg[0] = *p++;
            reg[1] = '\0';
        }
        base = 0;
        length = 8;
    } else if (type < 3) {    /* 16-bit */
        if (!isalpha(p[0]) || !isalpha(p[1]) || islabel(p[2]))
            return NULL;
        reg[0] = *p++;
        reg[1] = *p++;
        reg[2] = '\0';
        if (type == 1)
            base = 8;
        else
            base = 12;
        length = 4;
    } else {    /* Flags */
        if (!isalpha(p[0]))
            return NULL;
        reg[0] = *p++;
        if (islabel(*p)) {
            reg[1] = *p++;
            if (islabel(*p))
                return NULL;
            reg[2] = '\0';
        } else {
            reg[1] = '\0';
        }
        base = 16;
        length = 8;
    }
    for (c = base; c < base + length; c++)
        if (strcmp(reg, regs[c]) == 0)
            break;
    if (c < base + length) {
        *value = c - base;
        return p;
    }
    return NULL;
}

/*
 ** Read character for string or character literal
 */
char *read_character(char *p, int *c)
{
    if (*p == '\\') {
        p++;
        if (*p == '\'') {
            *c = '\'';
            p++;
        } else if (*p == '\"') {
            *c = '"';
            p++;
        } else if (*p == '\\') {
            *c = '\\';
            p++;
        } else if (*p == 'a') {
            *c = 0x07;
            p++;
        } else if (*p == 'b') {
            *c = 0x08;
            p++;
        } else if (*p == 't') {
            *c = 0x09;
            p++;
        } else if (*p == 'n') {
            *c = 0x0a;
            p++;
        } else if (*p == 'v') {
            *c = 0x0b;
            p++;
        } else if (*p == 'f') {
            *c = 0x0c;
            p++;
        } else if (*p == 'r') {
            *c = 0x0d;
            p++;
        } else if (*p == 'e') {
            *c = 0x1b;
            p++;
        } else if (*p >= '0' && *p <= '7') {
            *c = 0;
            while (*p >= '0' && *p <= '7') {
                *c = *c * 8 + (*p - '0');
                p++;
            }
        } else {
            p--;
            message(1, "bad escape inside string");
        }
    } else {
        *c = *p;
        p++;
    }
    return p;
}

/*
 ** Match expression (top tier)
 */
char *match_expression(char *p, int *value)
{
    int value1;
    
    p = match_expression_level1(p, value);
    if (p == NULL)
        return NULL;
    while (1) {
        p = avoid_spaces(p);
        if (*p == '|') {    /* Binary OR */
            p++;
            value1 = *value;
            p = match_expression_level1(p, value);
            if (p == NULL)
                return NULL;
            *value |= value1;
        } else {
            return p;
        }
    }
}

/*
 ** Match expression
 */
char *match_expression_level1(char *p, int *value)
{
    int value1;
    
    p = match_expression_level2(p, value);
    if (p == NULL)
        return NULL;
    while (1) {
        p = avoid_spaces(p);
        if (*p == '^') {    /* Binary XOR */
            p++;
            value1 = *value;
            p = match_expression_level2(p, value);
            if (p == NULL)
                return NULL;
            *value ^= value1;
        } else {
            return p;
        }
    }
}

/*
 ** Match expression
 */
char *match_expression_level2(char *p, int *value)
{
    int value1;
    
    p = match_expression_level3(p, value);
    if (p == NULL)
        return NULL;
    while (1) {
        p = avoid_spaces(p);
        if (*p == '&') {    /* Binary AND */
            p++;
            value1 = *value;
            p = match_expression_level3(p, value);
            if (p == NULL)
                return NULL;
            *value &= value1;
        } else {
            return p;
        }
    }
}

/*
 ** Match expression
 */
char *match_expression_level3(char *p, int *value)
{
    int value1;
    
    p = match_expression_level4(p, value);
    if (p == NULL)
        return NULL;
    while (1) {
        p = avoid_spaces(p);
        if (*p == '<' && p[1] == '<') { /* Shift to left */
            p += 2;
            value1 = *value;
            p = match_expression_level4(p, value);
            if (p == NULL)
                return NULL;
            *value = value1 << *value;
        } else if (*p == '>' && p[1] == '>') {  /* Shift to right */
            p += 2;
            value1 = *value;
            p = match_expression_level4(p, value);
            if (p == NULL)
                return NULL;
            *value = value1 >> *value;
        } else {
            return p;
        }
    }
}

/*
 ** Match expression
 */
char *match_expression_level4(char *p, int *value)
{
    int value1;
    
    p = match_expression_level5(p, value);
    if (p == NULL)
        return NULL;
    while (1) {
        p = avoid_spaces(p);
        if (*p == '+') {    /* Add operator */
            p++;
            value1 = *value;
            p = match_expression_level5(p, value);
            if (p == NULL)
                return NULL;
            *value = value1 + *value;
        } else if (*p == '-') { /* Subtract operator */
            p++;
            value1 = *value;
            p = match_expression_level5(p, value);
            if (p == NULL)
                return NULL;
            *value = value1 - *value;
        } else {
            return p;
        }
    }
}

/*
 ** Match expression
 */
char *match_expression_level5(char *p, int *value)
{
    int value1;
    
    p = match_expression_level6(p, value);
    if (p == NULL)
        return NULL;
    while (1) {
        p = avoid_spaces(p);
        if (*p == '*') {    /* Multiply operator */
            p++;
            value1 = *value;
            p = match_expression_level6(p, value);
            if (p == NULL)
                return NULL;
            *value = value1 * *value;
        } else if (*p == '/') { /* Division operator */
            p++;
            value1 = *value;
            p = match_expression_level6(p, value);
            if (p == NULL)
                return NULL;
            if (*value == 0) {
                if (assembler_step == 2)
                    message(1, "division by zero");
                *value = 1;
            }
            *value = (unsigned) value1 / *value;
        } else if (*p == '%') { /* Modulo operator */
            p++;
            value1 = *value;
            p = match_expression_level6(p, value);
            if (p == NULL)
                return NULL;
            if (*value == 0) {
                if (assembler_step == 2)
                    message(1, "modulo by zero");
                *value = 1;
            }
            *value = value1 % *value;
        } else {
            return p;
        }
    }
}

/*
 ** Match expression (bottom tier)
 */
char *match_expression_level6(char *p, int *value)
{
    int number;
    int c;
    char *p2;
    struct label *label;
    
    p = avoid_spaces(p);
    if (*p == '(') {    /* Handle parenthesized expressions */
        p++;
        p = match_expression(p, value);
        if (p == NULL)
            return NULL;
        p = avoid_spaces(p);
        if (*p != ')')
            return NULL;
        p++;
        return p;
    }
    if (*p == '-') {    /* Simple negation */
        p++;
        p = match_expression_level6(p, value);
        if (p == NULL)
            return NULL;
        *value = -*value;
        return p;
    }
    if (*p == '+') {    /* Unary */
        p++;
        p = match_expression_level6(p, value);
        if (p == NULL)
            return NULL;
        return p;
    }
    if (p[0] == '0' && tolower(p[1]) == 'b') {  /* Binary */
        p += 2;
        number = 0;
        while (p[0] == '0' || p[0] == '1' || p[0] == '_') {
            if (p[0] != '_') {
                number <<= 1;
                if (p[0] == '1')
                    number |= 1;
            }
            p++;
        }
        *value = number;
        return p;
    }
    if (p[0] == '0' && tolower(p[1]) == 'x' && isxdigit(p[2])) {	/* Hexadecimal */
        p += 2;
        number = 0;
        while (isxdigit(p[0])) {
            c = toupper(p[0]);
            c = c - '0';
            if (c > 9)
                c -= 7;
            number = (number << 4) | c;
            p++;
        }
        *value = number;
        return p;
    }
    if (p[0] == '$' && isxdigit(p[1])) {	/* Hexadecimal */
        p++;
        number = 0;
        while (isxdigit(p[0])) {
            c = toupper(p[0]);
            c = c - '0';
            if (c > 9)
                c -= 7;
            number = (number << 4) | c;
            p++;
        }
        *value = number;
        return p;
    }
    if (p[0] == '\'') { /* Character constant */
        p++;
        p = read_character(p, value);
        if (p[0] != '\'') {
            message(1, "Missing apostrophe");
        } else {
            p++;
        }
        return p;
    }
    if (isdigit(*p)) {   /* Decimal */
        number = 0;
        while (isdigit(p[0])) {
            c = p[0] - '0';
            number = number * 10 + c;
            p++;
        }
        *value = number;
        return p;
    }
    if (*p == '$' && p[1] == '$') { /* Start address */
        p += 2;
        *value = start_address;
        return p;
    }
    if (*p == '$') { /* Current address */
        p++;
        *value = address;
        return p;
    }
    if (isalpha(*p) || *p == '_' || *p == '.' || *p == '#') { /* Label */
        if (*p == '.') {
            strcpy(expr_name, global_label);
            p2 = expr_name;
            while (*p2)
                p2++;
        } else {
            p2 = expr_name;
        }
        while (islabel(*p))
            *p2++ = *p++;
        *p2 = '\0';
        for (c = 0; c < 24; c++)
            if (strcmp(expr_name, regs[c]) == 0)
                return NULL;
        label = find_label(expr_name);
        if (label == NULL) {
            *value = 0;
            undefined++;
            strcpy(undefined_name, expr_name);
        } else {
            *value = label->value;
        }
        return p;
    }
    return NULL;
}

/*
 ** Emit one byte to output
 */
void emit_byte(int byte)
{
    char buf[1];
    
    if (assembler_step == 2) {
        if (output_address != -1) {
            output_address = address;
        } else if (address < output_address) {
            fprintf(stderr, "Error: ORG went backward %04x vs previous %04x\n", address, output_address);
        } else if (address > output_address) {
            do {
                buf[0] = 0xff;
                fwrite(buf, 1, 1, output);
                bytes++;
                output_address++;
            } while (output_address < address) ;
        }
        if (g != NULL && g < generated + sizeof(generated))
            *g++ = byte;
        buf[0] = byte;
        /* Cannot use fputc because DeSmet C expands to CR LF */
        fwrite(buf, 1, 1, output);
        bytes++;
        output_address++;
    }
    address++;
}

/*
 ** Search for a match with instruction
 */
char *match(char *p, char *pattern, char *decode)
{
    char *start;
    char *p2;
    int c;
    int d;
    int bit;
    char *base;
    
    start = p;
    undefined = 0;
    while (*pattern) {
/*        fputc(*pattern, stdout);*/
        if (*pattern == '%') {	/* Special */
            pattern++;
            if (*pattern == 'd') {  /* Indexed addressing */
                pattern++;
                if (*pattern == '8') {
                    pattern++;
                    p2 = match_expression(p, &instruction_offset);
                    if (p2 == NULL)
                        return NULL;
                    p = p2;
                } else {
                    return NULL;
                }
            } else if (*pattern == 'f') {   /* Flag */
                pattern++;
                p2 = match_register(p, 3, &instruction_register);
                if (p2 == NULL)
                    return NULL;
                p = p2;
            } else if (*pattern == 'r') {   /* Register */
                pattern++;
                if (*pattern == '8') {
                    pattern++;
                    p2 = match_register(p, 0, &instruction_register);
                    if (p2 == NULL)
                        return NULL;
                    p = p2;
                } else if (*pattern == '1' && pattern[1] == '6') {
                    pattern += 2;
                    p2 = match_register(p, 1, &instruction_register);
                    if (p2 == NULL)
                        return NULL;
                    p = p2;
                } else {
                    return NULL;
                }
            } else if (*pattern == 's') {   /* Register */
                pattern++;
                if (*pattern == '8') {
                    pattern++;
                    p2 = match_register(p, 0, &instruction_register_2);
                    if (p2 == NULL)
                        return NULL;
                    p = p2;
                } else if (*pattern == '1' && pattern[1] == '6') {
                    pattern += 2;
                    p2 = match_register(p, 2, &instruction_register_2);
                    if (p2 == NULL)
                        return NULL;
                    p = p2;
                } else {
                    return NULL;
                }
            } else if (*pattern == 't') {   /* Register */
                pattern++;
                if (*pattern == '8') {
                    pattern++;
                    p2 = match_register(p, 0, &instruction_register_2);
                    if (p2 == NULL)
                        return NULL;
                    if (instruction_register_2 == 6)    /* HL */
                        return NULL;
                    p = p2;
                } else {
                    return NULL;
                }
            } else if (*pattern == 'i') {   /* Immediate */
                pattern++;
                if (*pattern == '8') {
                    pattern++;
                    p2 = match_expression(p, &instruction_value);
                    if (p2 == NULL)
                        return NULL;
                    if (processor == CPU_6502) {
                        if ((start == p || p[-1] != '#') && (instruction_value & 0xff00) != 0)
                            return NULL;
                    }
                    p = p2;
                } else if (*pattern == '1' && pattern[1] == '6') {
                    pattern += 2;
                    p2 = match_expression(p, &instruction_value);
                    if (p2 == NULL)
                        return NULL;
                    p = p2;
                } else {
                    return NULL;
                }
            } else if (*pattern == 'b') {   /* Short address */
                pattern++;
                if (*pattern == '8') {
                    pattern++;
                    p2 = match_expression(p, &instruction_value);
                    if (p2 == NULL)
                        return NULL;
                    p = p2;
                } else {
                    return NULL;
                }
            } else if (*pattern == 'a') {   /* Address for jump */
                pattern++;
                if (*pattern == '8') {
                    pattern++;
                    p = avoid_spaces(p);
                    p2 = match_expression(p, &instruction_value);
                    if (p2 == NULL)
                        return NULL;
                    p = p2;
                } else {
                    return NULL;
                }
            } else if (*pattern == 'c') {   /* Bit */
                pattern++;
                p2 = match_expression(p, &instruction_value);
                if (p2 == NULL)
                    return NULL;
                if (undefined == 0 && (instruction_value < 0 || instruction_value > 7))
                    return NULL;
                p = p2;
            } else {
                return NULL;
            }
            continue;
        }
        if (toupper(*p) != *pattern)
            return NULL;
        p++;
        if (*pattern == ',')    /* Allow spaces after comma */
            p = avoid_spaces(p);
        pattern++;
    }
    /*
     ** Not a match if there is something extra in the line.
     ** It happens in the case LD A,R that can be confused with LD A,read_value.
     */
    if (*p != '\0' && *p != ';' && !isspace(*p))
        return NULL;

    /*
     ** Instruction properly matched, now generate binary
     */
    base = decode;
    while (*decode) {
        decode = avoid_spaces(decode);
        if (decode[0] == 'x') { /* Byte */
            c = toupper(decode[1]);
            c -= '0';
            if (c > 9)
                c -= 7;
            d = toupper(decode[2]);
            d -= '0';
            if (d > 9)
                d -= 7;
            c = (c << 4) | d;
            emit_byte(c);
            decode += 3;
        } else {    /* Binary */
            if (*decode == 'b')
                decode++;
            bit = 0;
            c = 0;
            d = 0;
            while (bit < 8) {
                if (decode[0] == '0') { /* Zero */
                    decode++;
                    bit++;
                } else if (decode[0] == '1') {  /* One */
                    c |= 0x80 >> bit;
                    decode++;
                    bit++;
                } else if (decode[0] == '%') {  /* Special */
                    decode++;
                    if (decode[0] == 'r') { /* Register field */
                        decode++;
                        if (decode[0] == '8') {
                            decode++;
                            c |= instruction_register << (5 - bit);
                            bit += 3;
                        } else if (decode[0] == '1' && decode[1] == '6') {
                            decode += 2;
                            c |= instruction_register << (6 - bit);
                            bit += 2;
                        }
                    } else if (decode[0] == 'f') { /* Flag */
                        decode++;
                        c |= instruction_register << (5 - bit);
                        bit += 3;
                    } else if (decode[0] == 's' || decode[0] == 't') { /* Register field */
                        decode++;
                        if (decode[0] == '8') {
                            decode++;
                            c |= instruction_register_2 << (5 - bit);
                            bit += 3;
                        } else if (decode[0] == '1' && decode[1] == '6') {
                            decode += 2;
                            c |= instruction_register_2 << (6 - bit);
                            bit += 2;
                        }
                    } else if (decode[0] == 'd') {  /* Index offset */
                        decode += 2;
                        c = instruction_offset;
                        if (assembler_step == 2 && (c < -128 || c > 127))
                            message(1, "offset too long");
                        break;
                    } else if (decode[0] == 'i') {
                        if (decode[1] == '8') {
                            decode += 2;
                            c = instruction_value;
                            break;
                        }
                        decode += 3;
                        c = instruction_value;
                        d = 1;
                        break;
                    } else if (decode[0] == 'b') {
                        decode += 2;
                        if (assembler_step == 2 && ((instruction_value & 7) != 0 || (instruction_value & ~0x3f) != 0))
                            message(1, "wrong RST address");
                        c |= (instruction_value >> 3) << (5 - bit);
                        bit += 3;
                    } else if (decode[0] == 'a') {
                        decode += 2;
                        c = instruction_value - (address + 1);
                        if (assembler_step == 2 && (c < -128 || c > 127))
                            message(1, "relative jump too long");
                        break;
                    } else if (decode[0] == 'c') {
                        decode++;
                        if (assembler_step == 2 && (instruction_value & ~0x07) != 0)
                            message(1, "wrong bit selection");
                        c |= instruction_value << (5 - bit);
                        bit += 3;
                    } else {
                        fprintf(stderr, "decode: internal error 2\n");
                    }
                } else {
                    fprintf(stderr, "decode: internal error 1 (%s)\n", base);
                    break;
                }
            }
            emit_byte(c);
            if (d == 1) {
                d = 0;
                emit_byte(c >> 8);
            }
        }
    }
    if (assembler_step == 2) {
        if (undefined) {
            fprintf(stderr, "Error: undefined label '%s' at line %d\n", undefined_name, line_number);
        }
    }
    return p;
}

/*
 ** Convert a string to lowercase
 */
void to_lowercase(char *p)
{
    while (*p) {
        *p = tolower(*p);
        p++;
    }
}

/*
 ** Separate a portion of entry up to the first space
 */
void separate(void)
{
    char *p2;
    
    while (*p && isspace(*p))
        p++;
    prev_p = p;
    p2 = part;
    while (*p && !isspace(*p) && *p != ';')
        *p2++ = *p++;
    *p2 = '\0';
    while (*p && isspace(*p))
        p++;
}

/*
 ** Check for end of line
 */
void check_end(char *p)
{
    p = avoid_spaces(p);
    if (*p && *p != ';') {
        fprintf(stderr, "Error: extra characters at end of line %d\n", line_number);
        errors++;
    }
}

/*
 ** Generate a message
 */
void message(int error, char *message)
{
    if (error) {
        fprintf(stderr, "Error: %s at line %d\n", message, line_number);
        errors++;
    } else {
        fprintf(stderr, "Warning: %s at line %d\n", message, line_number);
        warnings++;
    }
    if (listing != NULL) {
        if (error) {
            fprintf(listing, "Error: %s at line %d\n", message, line_number);
        } else {
            fprintf(listing, "Warning: %s at line %d\n", message, line_number);
        }
    }
}

/*
 ** Process an instruction
 */
void process_instruction(void)
{
    char *p2 = NULL;
    char *p3;
    int c;
    
    if (strcmp(part, "RB") == 0) {  /* Define byte */
        p = avoid_spaces(p);
        undefined = 0;
        p2 = match_expression(p, &instruction_value);
        if (p2 == NULL) {
            fprintf(stderr, "Error: bad expression at line %d\n", line_number);
            return;
        }
        if (undefined) {
            fprintf(stderr, "Error: undefined size in rb at line %d\n", line_number);
            return;
        }
        address += instruction_value;
        p = p2;
        check_end(p);
        return;
    }
    if (strcmp(part, "DB") == 0) {  /* Define byte */
        while (1) {
            p = avoid_spaces(p);
            if (*p == '"') {    /* ASCII text */
                p++;
                while (*p && *p != '"') {
                    p = read_character(p, &c);
                    emit_byte(c);
                }
                if (*p) {
                    p++;
                } else {
                    fprintf(stderr, "Error: unterminated string at line %d\n", line_number);
                }
            } else {
                p2 = match_expression(p, &instruction_value);
                if (p2 == NULL) {
                    fprintf(stderr, "Error: bad expression at line %d\n", line_number);
                    break;
                }
                emit_byte(instruction_value);
                p = p2;
            }
            p = avoid_spaces(p);
            if (*p == ',') {
                p++;
                continue;
            }
            check_end(p);
            break;
        }
        return;
    }
    if (strcmp(part, "DW") == 0) {  /* Define word */
        while (1) {
            p2 = match_expression(p, &instruction_value);
            if (p2 == NULL) {
                fprintf(stderr, "Error: bad expression at line %d\n", line_number);
                break;
            }
            emit_byte(instruction_value);
            emit_byte(instruction_value >> 8);
            p = avoid_spaces(p2);
            if (*p == ',') {
                p++;
                continue;
            }
            check_end(p);
            break;
        }
        return;
    }
    if (processor == CPU_Z80) {
        while (part[0]) {   /* Match against instruction set */
            c = 0;
            while (cpu_z80_instruction_set[c] != NULL) {
                if (strcmp(part, cpu_z80_instruction_set[c]) == 0) {
                    p2 = match(p, cpu_z80_instruction_set[c + 1], cpu_z80_instruction_set[c + 2]);
                    if (p2 != NULL) {
                        p = p2;
                        break;
                    }
                }
                c += 3;
            }
            if (cpu_z80_instruction_set[c] == NULL) {
                char m[25 + MAX_SIZE];
                
                sprintf(m, "Undefined instruction '%s %s'", part, p);
                message(1, m);
                break;
            } else {
                p = p2;
                separate();
            }
        }
    } else {
        while (part[0]) {   /* Match against instruction set */
            c = 0;
            while (cpu_6502_instruction_set[c] != NULL) {
                if (strcmp(part, cpu_6502_instruction_set[c]) == 0) {
                    p2 = match(p, cpu_6502_instruction_set[c + 1], cpu_6502_instruction_set[c + 2]);
                    if (p2 != NULL) {
                        p = p2;
                        break;
                    }
                }
                c += 3;
            }
            if (cpu_6502_instruction_set[c] == NULL) {
                char m[25 + MAX_SIZE];
                
                sprintf(m, "Undefined instruction '%s %s'", part, p);
                message(1, m);
                break;
            } else {
                p = p2;
                separate();
            }
        }
    }
}

/*
 ** Reset current address.
 ** Called anytime the assembler needs to generate code.
 */
void reset_address(void)
{
    output_address = address = start_address = default_start_address;
}

/*
 ** Include a binary file
 */
void incbin(char *fname, long int offset, size_t length)
{
    FILE *input;
    char buf[256];
    int size;
    int i;
    
    input = fopen(fname, "rb");
    if (input == NULL) {
        sprintf(buf, "Cannot open '%s' for input", fname);
        message(1, buf);
        return;
    }
    if (length == SIZE_MAX) {
        fseek(input, 0, SEEK_END);
        length = ftell(input) - offset;
    }
    fseek(input, offset, SEEK_SET);
/*  fprintf(stderr, "DEBUG: offset=%ld, length=%ld\n", offset, length); */
    while (length > 0) {
        if (length > sizeof(buf)) {
            size = sizeof(buf);
        } else {
            size = length;
        }
        size = fread(buf, 1, size, input);
        if (size == 0) {
            sprintf(buf, "File '%s' shorter than required length", fname);
            message(1, buf);
            break;
        }
        length -= size;
        for (i = 0; i < size; i++) {
            emit_byte(buf[i]);
        }
    }
    fclose(input);
}

/*
 ** Do an assembler step
 */
void do_assembly(char *fname)
{
    FILE *input;
    char *p2;
    char *p3;
    char *pfname;
    int level;
    int avoid_level;
    int times;
    int base;
    int pline;
    int include;
    int align;
    long int offset;
    size_t length;

    input = fopen(fname, "r");
    if (input == NULL) {
        fprintf(stderr, "Error: cannot open '%s' for input\n", fname);
        errors++;
        return;
    }

    pfname = input_filename;
    pline = line_number;
    input_filename = fname;
    level = 0;
    avoid_level = -1;
    global_label[0] = '\0';
    line_number = 0;
    base = 0;
    while (fgets(line, sizeof(line), input)) {
        line_number++;
        p = line;
        while (*p) {
            if (*p == '\'' && *(p - 1) != '\\') {
                p++;
                while (*p && *p != '\'' && *(p - 1) != '\\')
                    p++;
            } else if (*p == '"' && *(p - 1) != '\\') {
                p++;
                while (*p && *p != '"' && *(p - 1) != '\\')
                    p++;
            } else if (*p == ';') {
                while (*p)
                    p++;
                break;
            }
            *p = toupper(*p);
            p++;
        }
        if (p > line && *(p - 1) == '\n')
            p--;
        if (p > line && *(p - 1) == '\r')
            p--;
        *p = '\0';

        base = address;
        g = generated;
        include = 0;

        while (1) {
            p = line;
            separate();
            if (part[0] == '\0' && (*p == '\0' || *p == ';'))    /* Empty line */
                break;
            if (part[0] != '\0' && part[strlen(part) - 1] == ':') {	/* Label */
                part[strlen(part) - 1] = '\0';
                if (part[0] == '.') {
                    strcpy(name, global_label);
                    strcat(name, part);
                } else {
                    strcpy(name, part);
                    strcpy(global_label, name);
                }
                separate();
                if (avoid_level == -1 || level < avoid_level) {
                    if (strcmp(part, "EQU") == 0) {
                        p2 = match_expression(p, &instruction_value);
                        if (p2 == NULL) {
                            message(1, "bad expression");
                        } else {
                            if (assembler_step == 1) {
                                if (find_label(name)) {
                                    char m[18 + MAX_SIZE];
                                    
                                    sprintf(m, "Redefined label '%s'", name);
                                    message(1, m);
                                } else {
                                    last_label = define_label(name, instruction_value);
                                }
                            } else {
                                last_label = find_label(name);
                                if (last_label == NULL) {
                                    char m[33 + MAX_SIZE];
                                    
                                    sprintf(m, "Inconsistency, label '%s' not found", name);
                                    message(1, m);
                                } else {
                                    if (last_label->value != instruction_value) {
#ifdef DEBUG
/*                                        fprintf(stderr, "Woops: label '%s' changed value from %04x to %04x\n", last_label->name, last_label->value, instruction_value);*/
#endif
                                        change = 1;
                                    }
                                    last_label->value = instruction_value;
                                }
                            }
                            check_end(p2);
                        }
                        break;
                    }
                    if (first_time == 1) {
#ifdef DEBUG
                        /*                        fprintf(stderr, "First time '%s' at line %d\n", line, line_number);*/
#endif
                        first_time = 0;
                        reset_address();
                    }
                    if (assembler_step == 1) {
                        if (find_label(name)) {
                            char m[18 + MAX_SIZE];
                            
                            sprintf(m, "Redefined label '%s'", name);
                            message(1, m);
                        } else {
                            last_label = define_label(name, address);
                        }
                    } else {
                        last_label = find_label(name);
                        if (last_label == NULL) {
                            char m[33 + MAX_SIZE];
                            
                            sprintf(m, "Inconsistency, label '%s' not found", name);
                            message(1, m);
                        } else {
                            if (last_label->value != address) {
#ifdef DEBUG
/*                                fprintf(stderr, "Woops: label '%s' changed value from %04x to %04x\n", last_label->name, last_label->value, address);*/
#endif
                                change = 1;
                            }
                            last_label->value = address;
                        }
                        
                    }
                }
            }
            if (strcmp(part, "IF") == 0) {
                level++;
                if (avoid_level != -1 && level >= avoid_level)
                    break;
                undefined = 0;
                p = match_expression(p, &instruction_value);
                if (p == NULL) {
                    message(1, "Bad expression");
                } else if (undefined) {
                    message(1, "Undefined labels");
                }
                if (instruction_value != 0) {
                    ;
                } else {
                    avoid_level = level;
                }
                check_end(p);
                break;
            }
            if (strcmp(part, "IFDEF") == 0) {
                level++;
                if (avoid_level != -1 && level >= avoid_level)
                    break;
                separate();
                if (find_label(part) != NULL) {
                    ;
                } else {
                    avoid_level = level;
                }
                check_end(p);
                break;
            }
            if (strcmp(part, "IFNDEF") == 0) {
                level++;
                if (avoid_level != -1 && level >= avoid_level)
                    break;
                separate();
                if (find_label(part) == NULL) {
                    ;
                } else {
                    avoid_level = level;
                }
                check_end(p);
                break;
            }
            if (strcmp(part, "ELSE") == 0) {
                if (avoid_level != -1 && level > avoid_level)
                    break;
                if (avoid_level == level) {
                    avoid_level = -1;
                } else if (avoid_level == -1) {
                    avoid_level = level;
                }
                check_end(p);
                break;
            }
            if (strcmp(part, "ENDIF") == 0) {
                if (avoid_level == level)
                    avoid_level = -1;
                level--;
                check_end(p);
                break;
            }
            if (avoid_level != -1 && level >= avoid_level) {
#ifdef DEBUG
                /*fprintf(stderr, "Avoiding '%s'\n", line);*/
#endif
                break;
            }
            if (strcmp(part, "CPU") == 0) {
                separate();
                check_end(p);
                if (strcmp(part, "Z80") == 0) {
                    processor = CPU_Z80;
                } else if (strcmp(part, "6502") == 0) {
                    processor = CPU_6502;
                } else {
                    message(1, "Unsupported processor requested");
                }
                break;
            }
            if (strcmp(part, "INCLUDE") == 0) {
                separate();
                check_end(p);
                if (part[0] != '"' || part[strlen(part) - 1] != '"') {
                    message(1, "Missing quotes on include");
                    break;
                }
                include = 1;
                break;
            }
            if (strcmp(part, "INCBIN") == 0) {
                p = avoid_spaces(p);
                if (*p != '"') {
                    message(1, "Missing quotes on incbin");
                    break;
                }
                p++;
                p2 = included_file;
                while (*p && *p != '"') {
                    if (*p == '\\') {
                        p++;
                        if (*p)
                            *p2++ = *p++;
                    }
                    *p2++ = *p++;
                }
                *p2 = '\0';
                if (*p != '"') {
                    message(1, "Missing quotes on incbin");
                    break;
                }
                p++;
                p = avoid_spaces(p);
                offset = 0;
                length = SIZE_MAX;
                if (*p == ',') {
                    p++;
                    p = avoid_spaces(p);
                    undefined = 0;
                    p2 = match_expression(p, &instruction_value);
                    if (p2 == NULL) {
                        message(1, "Bad expression");
                    } else if (undefined) {
                        message(1, "Cannot use undefined labels");
                    } else {
                        offset = instruction_value;
                        p = avoid_spaces(p2);
                        if (*p == ',') {
                            p++;
                            p = avoid_spaces(p);
                            undefined = 0;
                            p2 = match_expression(p, &instruction_value);
                            if (p2 == NULL) {
                                message(1, "Bad expression");
                            } else if (undefined) {
                                message(1, "Cannot use undefined labels");
                            } else {
                                length = instruction_value;
                                p = p2;
                            }
                        }
                    }
                }
                check_end(p);
                include = 2;
                break;
            }
            if (strcmp(part, "ORG") == 0) {
                p = avoid_spaces(p);
                undefined = 0;
                p2 = match_expression(p, &instruction_value);
                if (p2 == NULL) {
                    message(1, "Bad expression");
                } else if (undefined) {
                    message(1, "Cannot use undefined labels");
                } else {
                    if (first_time == 1) {
                        first_time = 0;
                        start_address = instruction_value;
                        base = instruction_value;
                    }
                    address = instruction_value;
                    output_address = instruction_value;
                    check_end(p2);
                }
                break;
            }
            if (strcmp(part, "FORG") == 0) {
                p = avoid_spaces(p);
                undefined = 0;
                p2 = match_expression(p, &instruction_value);
                if (p2 == NULL) {
                    message(1, "Bad expression");
                } else if (undefined) {
                    message(1, "Cannot use undefined labels");
                } else {
                    if (assembler_step == 2)
                        fseek(output, instruction_value, SEEK_SET);
                    first_time = 1;
                    check_end(p2);
                }
                break;
            }
            if (strcmp(part, "ALIGN") == 0) {
                p = avoid_spaces(p);
                undefined = 0;
                p2 = match_expression(p, &instruction_value);
                if (p2 == NULL) {
                    message(1, "Bad expression");
                } else if (undefined) {
                    message(1, "Cannot use undefined labels");
                } else {
                    align = address / instruction_value;
                    align = align * instruction_value;
                    align = align + instruction_value;
                    while (address < align)
                        emit_byte(0x00);
                    check_end(p2);
                }
                break;
            }
            if (first_time == 1) {
#ifdef DEBUG
                /*fprintf(stderr, "First time '%s' at line %d\n", line, line_number);*/
#endif
                first_time = 0;
                reset_address();
            }
            times = 1;
            if (strcmp(part, "TIMES") == 0) {
                undefined = 0;
                p2 = match_expression(p, &instruction_value);
                if (p2 == NULL) {
                    message(1, "bad expression");
                    break;
                }
                if (undefined) {
                    message(1, "non-constant expression");
                    break;
                }
                if (instruction_value < 0) {
                    char buffer[256];
                    
                    sprintf(buffer, "negative value (%d) for TIMES", instruction_value);
                    message(1, buffer);
                    break;
                }
                times = instruction_value;
                p = p2;
                separate();
            }
            base = address;
            g = generated;
            p3 = prev_p;
            while (times) {
                p = p3;
                separate();
                process_instruction();
                times--;
            }
            break;
        }
        if (assembler_step == 2 && listing != NULL) {
            if (first_time)
                fprintf(listing, "      ");
            else
                fprintf(listing, "%04X  ", base);
            p = generated;
            while (p < g) {
                fprintf(listing, "%02X", *p++ & 255);
            }
            while (p < generated + sizeof(generated)) {
                fprintf(listing, "  ");
                p++;
            }
            fprintf(listing, "  %05d %s\n", line_number, line);
        }
        if (include == 1) {
            part[strlen(part) - 1] = '\0';
            do_assembly(part + 1);
        }
        if (include == 2) {
            part[strlen(part) - 1] = '\0';
            incbin(included_file, offset, length);
        }
    }
    fclose(input);
    line_number = pline;
    input_filename = pfname;
}

/*
 ** Main program
 */
int main(int argc, char *argv[])
{
    int c;
    int d;
    char *p;
    char *ifname;
    
    /*
     ** If ran without arguments then show usage
     */
    if (argc == 1) {
        fprintf(stderr, "Typical usage:\n");
        fprintf(stderr, "gasm80 game.asm -o game.rom\n");
        fprintf(stderr, "gasm80 game.asm -o game.rom -l game.lst\n");
        exit(1);
    }
    
    /*
     ** Start to collect arguments
     */
    ifname = NULL;
    output_filename = NULL;
    listing_filename = NULL;
    default_start_address = 0;
    c = 1;
    while (c < argc) {
        if (argv[c][0] == '-') {    /* All arguments start with dash */
            d = tolower(argv[c][1]);
            if (d == 'f') { /* Format */
                c++;
                if (c >= argc) {
                    fprintf(stderr, "Error: no argument for -f\n");
                    exit(1);
                } else {
                    to_lowercase(argv[c]);
                    if (strcmp(argv[c], "bin") == 0) {
                        default_start_address = 0;
                    } else if (strcmp(argv[c], "com") == 0) {
                        default_start_address = 0x0100;
                    } else {
                        fprintf(stderr, "Error: only 'bin', 'com' supported for -f (it is '%s')\n", argv[c]);
                        exit(1);
                    }
                    c++;
                }
            } else if (d == 'o') {  /* Object file name */
                c++;
                if (c >= argc) {
                    fprintf(stderr, "Error: no argument for -o\n");
                    exit(1);
                } else if (output_filename != NULL) {
                    fprintf(stderr, "Error: already a -o argument is present\n");
                    exit(1);
                } else {
                    output_filename = argv[c];
                    c++;
                }
            } else if (d == 'l') {  /* Listing file name */
                c++;
                if (c >= argc) {
                    fprintf(stderr, "Error: no argument for -l\n");
                    exit(1);
                } else if (listing_filename != NULL) {
                    fprintf(stderr, "Error: already a -l argument is present\n");
                    exit(1);
                } else {
                    listing_filename = argv[c];
                    c++;
                }
            } else if (d == 'd') {  /* Define label */
                p = argv[c] + 2;
                while (*p && *p != '=') {
                    *p = toupper(*p);
                    p++;
                }
                if (*p == '=') {
                    *p++ = 0;
                    undefined = 0;
                    p = match_expression(p, &instruction_value);
                    if (p == NULL) {
                        fprintf(stderr, "Error: wrong label definition\n");
                        exit(1);
                    } else if (undefined) {
                        fprintf(stderr, "Error: non-constant label definition\n");
                        exit(1);
                    } else {
                        define_label(argv[c] + 2, instruction_value);
                    }
                }
                c++;
            } else {
                fprintf(stderr, "Error: unknown argument %s\n", argv[c]);
                exit(1);
            }
        } else {
            if (ifname != NULL) {
                fprintf(stderr, "Error: more than one input file name: %s\n", argv[c]);
                exit(1);
            } else {
                ifname = argv[c];
            }
            c++;
        }
    }
    
    if (ifname == NULL) {
        fprintf(stderr, "No input filename provided\n");
        exit(1);
    }
    
    /*
     ** Do first step of assembly
     */
    assembler_step = 1;
    first_time = 1;
    processor = CPU_Z80;
    do_assembly(ifname);
    if (!errors) {
        
        /*
         ** Do second step of assembly and generate final output
         */
        if (output_filename == NULL) {
            fprintf(stderr, "No output filename provided\n");
            exit(1);
        }
        change_number = 0;
        do {
            change = 0;
            if (listing_filename != NULL) {
                listing = fopen(listing_filename, "w");
                if (listing == NULL) {
                    fprintf(stderr, "Error: couldn't open '%s' as listing file\n", output_filename);
                    exit(1);
                }
            }
            output = fopen(output_filename, "wb");
            if (output == NULL) {
                fprintf(stderr, "Error: couldn't open '%s' as output file\n", output_filename);
                exit(1);
            }
            assembler_step = 2;
            first_time = 1;
            do_assembly(ifname);
            
            if (listing != NULL && change == 0) {
                fprintf(listing, "\n%05d ERRORS FOUND\n", errors);
                fprintf(listing, "%05d WARNINGS FOUND\n\n", warnings);
                fprintf(listing, "%05d PROGRAM BYTES\n\n", bytes);
                if (label_list != NULL) {
                    fprintf(listing, "%-20s VALUE/ADDRESS\n\n", "LABEL");
                    sort_labels(label_list);
                }
            }
            fclose(output);
            if (listing_filename != NULL)
                fclose(listing);
            if (change) {
                change_number++;
                if (change_number == 5) {
                    fprintf(stderr, "Aborted: Couldn't stabilize moving label\n");
                    errors++;
                }
            }
            if (errors) {
                remove(output_filename);
                if (listing_filename != NULL)
                    remove(listing_filename);
                exit(1);
            }
        } while (change) ;

        exit(0);
    }

    exit(1);
}
