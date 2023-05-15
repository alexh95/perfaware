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
};

struct instruction_bit_field
{
    instruction_bit_field_type BitFieldType;
    u32 Size;
    u32 Offset;
    u32 Value;
};

#define BIT_FIELD_COUNT 16

struct instruction_encoding
{
    instruction_type Type;
    u32 ByteCount;
    instruction_bit_field Fields[BIT_FIELD_COUNT];
};

#endif
