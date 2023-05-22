#ifndef PERFAWARE_INSTRUCTION_H
#define PERFAWARE_INSTRUCTION_H

enum instruction_type : u32;

enum instruction_bit_field_type : u32
{
    InstructionBitFieldType_None = 0,
    InstructionBitFieldType_Bits,
    InstructionBitFieldType_Direction,
    InstructionBitFieldType_Word,
    InstructionBitFieldType_Mod,
    InstructionBitFieldType_Reg,
    InstructionBitFieldType_RM,
    InstructionBitFieldType_SR,
    InstructionBitFieldType_Data,
    InstructionBitFieldType_DataW,
    InstructionBitFieldType_AddrLo,
    InstructionBitFieldType_AddrHi,
    InstructionBitFieldType_Displacement,
};

char *InstructionBitFieldTypeStrings[] =
{
    "InstructionBitFieldType_None",
    "InstructionBitFieldType_Bits",
    "InstructionBitFieldType_Direction",
    "InstructionBitFieldType_Word",
    "InstructionBitFieldType_Mod",
    "InstructionBitFieldType_Reg",
    "InstructionBitFieldType_RM",
    "InstructionBitFieldType_SR",
    "InstructionBitFieldType_Data",
    "InstructionBitFieldType_DataW",
    "InstructionBitFieldType_AddrLo",
    "InstructionBitFieldType_AddrHi",
    "InstructionBitFieldType_Displacement",
};

struct instruction_bit_field
{
    instruction_bit_field_type Type;
    u32 Size;
    u32 Offset;
    u32 Value;
};

#define BIT_FIELD_COUNT 16

struct instruction_encoding
{
    instruction_type Type;
    instruction_bit_field Fields[BIT_FIELD_COUNT];
};

enum operand_type : u32
{
    OperandType_None = 0,
    OperandType_Immediate,
    OperandType_Register,
    OperandType_EffectiveAddressCalculation,
};

char *RegisterName[8][2] =
{
    {"AL", "AX"},
    {"CL", "CX"},
    {"DL", "DX"},
    {"BL", "BX"},
    {"AH", "SP"},
    {"CH", "BP"},
    {"DH", "SI"},
    {"BH", "DI"},
};

char *EffectiveAddressCalculation[8] =
{
    "BX + SI",
    "BX + DI",
    "BP + SI",
    "BP + DI",
    "SI",
    "DI",
    "BP",
    "BX",
};

struct instruction_operand
{
    operand_type Type;
    u32 Value;
};

struct instruction
{
    instruction_type Type;
    b32 Direction;
    b32 Word;
    u32 Mod;
    u32 RM;
    u32 OperandCount;
    instruction_operand Operands[2];
    u32 ByteCount;
};

#endif
