#include "perfaware_disassembler.h"

u8 MakeBitFieldMask(u32 Size)
{
    u8 Result = 0;
    
    for (u32 Index = 0; Index < Size; ++Index)
    {
        Result = (Result << 1) | 1;
    }
    
    return Result;
}

u8 GetBitFieldValue(instruction_bit_field InstructionBitField, u32 CurrentMachineCodeIndex, buffer MachineCode)
{
    u32 ByteOffset = InstructionBitField.Offset / 8;
    u32 SubbyteOffset = InstructionBitField.Offset % 8;
    u8 InstructionByte = MachineCode.Data[CurrentMachineCodeIndex + ByteOffset];
    u8 BitFieldShifted = InstructionByte >> (8 - InstructionBitField.Size - SubbyteOffset);
    u8 BitFieldMask = MakeBitFieldMask(InstructionBitField.Size);
    u8 Result = BitFieldShifted & BitFieldMask;
    return Result;
}

instruction ValidateInstructionAndFillValues(instruction_encoding InstructionEncoding, u32 CurrentMachineCodeIndex, buffer MachineCode)
{
    instruction Result = {};
    Result.Type = InstructionEncoding.Type;
    Result.Mod = 4;
    u32 UsedBits = 0;
    
    for (u32 InstructionBitFieldIndex = 0;
         InstructionBitFieldIndex < BIT_FIELD_COUNT;
         ++InstructionBitFieldIndex)
    {
        instruction_bit_field InstructionBitField = InstructionEncoding.Fields[InstructionBitFieldIndex];
        if (InstructionBitField.Type > InstructionBitFieldType_None)
        {
            u8 BitFieldValue = GetBitFieldValue(InstructionBitField, CurrentMachineCodeIndex, MachineCode);
            UsedBits += InstructionBitField.Size;
            if (InstructionBitField.Type == InstructionBitFieldType_Bits)
            {
                if (BitFieldValue != InstructionBitField.Value)
                {
                    return {InstructionType_None};
                }
            }
            /*else
            {
                InstructionBitField.Value = BitFieldValue;
            }*/
            else if (InstructionBitField.Type == InstructionBitFieldType_Direction)
            {
                Result.Direction = BitFieldValue;
            }
            else if (InstructionBitField.Type == InstructionBitFieldType_Word)
            {
                Result.Word = BitFieldValue;
            }
            else if (InstructionBitField.Type == InstructionBitFieldType_Reg)
            {
                instruction_operand Operand = {};
                Operand.Type = OperandType_Register;
                Operand.Value = BitFieldValue;
                Result.Operands[Result.OperandCount++] = Operand;
            }
            else if (InstructionBitField.Type == InstructionBitFieldType_Mod)
            {
                Result.Mod = BitFieldValue;
            }
            // NOTE(alex): R/M is always followed by a displacement
            else if (InstructionBitField.Type == InstructionBitFieldType_RM)
            {
                Assert(Result.Mod < 4);
                
                Result.RM = BitFieldValue;
                
                /*if (Result.Mod == 0)
                {
                    
                }
                else */if (Result.Mod == 1)
                {
                    u8 ImmediateValue = GetBitFieldValue(InstructionBitField, CurrentMachineCodeIndex, MachineCode);
                    UsedBits += 8;
                    
                    instruction_operand Operand = {};
                    Operand.Type = OperandType_Immediate;
                    Operand.Value = ImmediateValue;
                    Result.Operands[Result.OperandCount++] = Operand;
                }
                else if (Result.Mod == 2 || (Result.Mod == 0 && BitFieldValue == 6))
                {
                    u8 ImmediateValueLow = GetBitFieldValue(InstructionBitField, CurrentMachineCodeIndex, MachineCode);
                    UsedBits += 8;
                    u8 ImmediateValueHigh = GetBitFieldValue(InstructionBitField, CurrentMachineCodeIndex, MachineCode);
                    UsedBits += 8;
                    u8 ImmediateValue = (ImmediateValueHigh << 8) | ImmediateValueLow;
                    
                    instruction_operand Operand = {};
                    Operand.Type = OperandType_Immediate;
                    Operand.Value = ImmediateValue;
                    Result.Operands[Result.OperandCount++] = Operand;
                }
                if (Result.Mod == 3)
                {
                    instruction_operand Operand = {};
                    Operand.Type = OperandType_Register;
                    Operand.Value = BitFieldValue;
                    Result.Operands[Result.OperandCount++] = Operand;
                }
            }
            else if (InstructionBitField.Type == InstructionBitFieldType_Data)
            {
                instruction_operand Operand = {};
                Operand.Type = OperandType_Immediate;
                Operand.Value = BitFieldValue;
                Result.Operands[Result.OperandCount++] = Operand;
            }
            else if (InstructionBitField.Type == InstructionBitFieldType_DataW)
            {
                if (Result.Word)
                {
                    // NOTE(alex): the last operand is assumed to be the one we are interested in
                    instruction_operand Operand = Result.Operands[Result.OperandCount - 1];
                    Operand.Value |= BitFieldValue << 8;
                    Result.Operands[Result.OperandCount - 1] = Operand;
                }
                else
                {
                    // TODO(alex): make this better
                    UsedBits -= 8;
                }
            }
            else
            {
                InvalidCodePath;
            }
        }
    }
    
    Assert(UsedBits % 8 == 0);
    Result.ByteCount = UsedBits / 8;
    return Result;
}

instruction DecodeInstruction(u32 CurrentMachineCodeIndex, buffer MachineCode)
{
    for (u32 InstructionEncodingIndex = 0;
         InstructionEncodingIndex < ArrayCount(InstructionEncodingList);
         ++InstructionEncodingIndex)
    {
        instruction_encoding InstructionEncoding = InstructionEncodingList[InstructionEncodingIndex];
        instruction Instruction =
            ValidateInstructionAndFillValues(InstructionEncoding, CurrentMachineCodeIndex, MachineCode);
        if (Instruction.Type > InstructionType_None)
        {
            return Instruction;
        }
    }
    
    InvalidCodePath;
    return {};
}

u32 WriteInstructionOperand(string Output, u32 Offset, instruction Instruction, u32 OperandIndex)
{
    instruction_operand Operand = Instruction.Operands[OperandIndex];
    
    if (Operand.Type == OperandType_Immediate)
    {
        Offset = StringFromI32(Output, Offset, Operand.Value);
    }
    else if (Operand.Type == OperandType_Register)
    {
        char *Name = RegisterName[Operand.Value][Instruction.Word];
        Offset = StringCopy(Output, Offset, Name);
    }
    
    return Offset;
}

u32 WriteInstruction(string Output, u32 Offset, instruction Instruction)
{
    Offset = StringCopy(Output, Offset, InstructionTypeToName[Instruction.Type]);
    
    if (Instruction.OperandCount > 0)
    {
        // NOTE(alex): Reg is considered to always be the first operand
        // TODO(alex): immediate inst should default direction 1!!
        u32 OperandIndex = Instruction.Direction == 1 ? 0 : 1;
        Offset = StringCopy(Output, Offset, " ");
        Offset = WriteInstructionOperand(Output, Offset, Instruction, OperandIndex);
    }
    if (Instruction.OperandCount > 1)
    {
        u32 OperandIndex = Instruction.Direction == 1 ? 1 : 0;
        Offset = StringCopy(Output, Offset, ", ");
        Offset = WriteInstructionOperand(Output, Offset, Instruction, OperandIndex);
    }
    
    Offset = StringCopy(Output, Offset, "\n");
    return Offset;
}

buffer Disassemble8086(memory_arena *Arena, buffer MachineCode)
{
    buffer Result = {};
    Result.Data = ArenaPushArray(u8, Arena, Kilobytes(8));
    Result.Size = StringCopy(Result, Result.Size, "BITS 16\n\n");
    
    u32 CurrentMachineCodeIndex = 0;
    while (CurrentMachineCodeIndex < MachineCode.Size)
    {
        instruction Instruction = DecodeInstruction(CurrentMachineCodeIndex, MachineCode);
        CurrentMachineCodeIndex += Instruction.ByteCount;
        Result.Size = WriteInstruction(Result, Result.Size, Instruction);
    }
    
    return Result;
}