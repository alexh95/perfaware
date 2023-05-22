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

u32 GetNextBytes(buffer MachineCode, u32 ByteOffset, b32 Word)
{
    u32 Result = MachineCode.Data[ByteOffset];
    if (Word)
    {
        Result |= MachineCode.Data[ByteOffset + 1] << 8;
    }
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
                
                Assert(UsedBits % 8 == 0);
                u32 UsedBytes = UsedBits / 8;
                
                if (Result.Mod == 0)
                {
                    instruction_operand Operand = {};
                    Operand.Type = OperandType_EffectiveAddressCalculation;
                    
                    if (BitFieldValue == 6)
                    {
                        u32 ImmediateValue = GetNextBytes(MachineCode, CurrentMachineCodeIndex + UsedBytes, true);
                        UsedBits += 16;
                        
                        Operand.Value = ImmediateValue;
                    }
                    
                    Result.Operands[Result.OperandCount++] = Operand;
                }
                else if (Result.Mod == 1)
                {
                    u32 ImmediateValue = GetNextBytes(MachineCode, CurrentMachineCodeIndex + UsedBytes, false);
                    UsedBits += 8;
                    
                    instruction_operand Operand = {};
                    Operand.Type = OperandType_EffectiveAddressCalculation;
                    Operand.Value = ImmediateValue;
                    Result.Operands[Result.OperandCount++] = Operand;
                }
                else if (Result.Mod == 2)
                {
                    u32 ImmediateValue = GetNextBytes(MachineCode, CurrentMachineCodeIndex + UsedBytes, true);
                    UsedBits += 16;
                    
                    instruction_operand Operand = {};
                    Operand.Type = OperandType_EffectiveAddressCalculation;
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
                Result.Direction = true;
                
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
        instruction Instruction = ValidateInstructionAndFillValues(InstructionEncoding, CurrentMachineCodeIndex, MachineCode);
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
        s32 Value;
        if (Instruction.Word)
        {
            Value = (s16)Operand.Value;
        }
        else
        {
            Value = (s8)Operand.Value;
        }
        Offset = StringFromS32(Output, Offset, Value);
    }
    else if (Operand.Type == OperandType_Register)
    {
        char *Name = RegisterName[Operand.Value][Instruction.Word];
        Offset = StringCopy(Output, Offset, Name);
    }
    else if (Operand.Type == OperandType_EffectiveAddressCalculation)
    {
        if (Instruction.Mod == 0)
        {
            if (Instruction.RM == 6)
            {
                Offset = StringCopy(Output, Offset, "[");
                Offset = StringFromS32(Output, Offset, (s16)Operand.Value);
                Offset = StringCopy(Output, Offset, "]");
            }
            else
            {
                char *EAC = EffectiveAddressCalculation[Instruction.RM];
                Offset = StringCopy(Output, Offset, "[");
                Offset = StringCopy(Output, Offset, EAC);
                Offset = StringCopy(Output, Offset, "]");
            }
        }
        else if (Instruction.Mod == 1)
        {
            char *EAC = EffectiveAddressCalculation[Instruction.RM];
            Offset = StringCopy(Output, Offset, "[");
            Offset = StringCopy(Output, Offset, EAC);
            if (Operand.Value > 0)
            {
                s8 Value = (s8)Operand.Value;
                if (Value > 0)
                {
                    Offset = StringCopy(Output, Offset, " + ");
                }
                else
                {
                    Offset = StringCopy(Output, Offset, " - ");
                }
                Offset = StringFromS32(Output, Offset, ABS(Value));
            }
            Offset = StringCopy(Output, Offset, "]");
        }
        else if (Instruction.Mod == 2)
        {
            char *EAC = EffectiveAddressCalculation[Instruction.RM];
            Offset = StringCopy(Output, Offset, "[");
            Offset = StringCopy(Output, Offset, EAC);
            if (Operand.Value > 0)
            {
                s16 Value = (s16)Operand.Value;
                if (Value > 0)
                {
                    Offset = StringCopy(Output, Offset, " + ");
                }
                else
                {
                    Offset = StringCopy(Output, Offset, " - ");
                }
                Offset = StringFromS32(Output, Offset, ABS(Value));
            }
            Offset = StringCopy(Output, Offset, "]");
        }
        else
        {
            InvalidCodePath;
        }
    }
    
    return Offset;
}

u32 WriteInstruction(string Output, u32 Offset, instruction Instruction)
{
    Offset = StringCopy(Output, Offset, InstructionTypeToName[Instruction.Type]);
    
    if (Instruction.OperandCount > 0)
    {
        // NOTE(alex): Reg is considered to always be the first operand
        u32 OperandIndex = Instruction.Direction ? 0 : 1;
        Offset = StringCopy(Output, Offset, " ");
        Offset = WriteInstructionOperand(Output, Offset, Instruction, OperandIndex);
    }
    if (Instruction.OperandCount > 1)
    {
        u32 OperandIndex = Instruction.Direction ? 1 : 0;
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