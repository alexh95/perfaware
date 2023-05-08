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
};

struct instruction_bit_field
{
    instruction_bit_field_type BitFieldType;
    u32 Size;
    u32 Offset;
};

#define BIT_FIELD_COUNT 16

struct instruction_encoding
{
    instruction_type Type;
    instruction_bit_field Fields[BIT_FIELD_COUNT];
};

#endif
