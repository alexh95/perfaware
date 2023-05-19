#include "perfaware_disassembler.h"

enum opcode
{
    OP_MOV_RM_R = 0,
    OP_MOV_IM_RM,
    OP_MOV_I_R,
    OP_MOV_M_A,
    OP_MOV_A_M,
    OP_COUNT, OP_INVALID
};

static inline opcode DecodeOpcode(u8 Byte)
{
    if ((Byte & 0b11111100) == 0b10001000) return OP_MOV_RM_R;
    if ((Byte & 0b11111110) == 0b11000110) return OP_MOV_IM_RM;
    if ((Byte & 0b11110000) == 0b10110000) return OP_MOV_I_R;
    if ((Byte & 0b11111110) == 0b10100000) return OP_MOV_M_A;
    if ((Byte & 0b11111110) == 0b10100010) return OP_MOV_A_M;
    
    return OP_INVALID;
}

static inline b32 DecodeDirectionFlag(u8 Byte)
{
    return Byte & 0x2;
}

static inline u32 DecodeWordFlag(u8 Byte)
{
    return Byte & 0x1;
}

static inline u32 DecodeWordFlagIR(u8 Byte)
{
    return (Byte & 0b00001000) >> 3;
}

char* RegisterTable[] =
{
    "al", "ax",
    "cl", "cx",
    "dl", "dx",
    "bl", "bx",
    "ah", "sp",
    "ch", "bp",
    "dh", "si",
    "bh", "di",
};

static inline u32 DecodeRegisterField(u8 Byte, u32 WordFlag, b32 Shifted)
{
    u32 Result;
    if (Shifted)
    {
        // 00000111
        Result = (Byte & 0b00000111) << 1;
    }
    else
    {
        // 00111000
        Result = (Byte & 0b00111000) >> 2;
    }
    if (WordFlag)
    {
        Result = Result | 1;
    }
    return Result;
}

enum mode
{
    MOD_MM = 0,
    MOD_MM_8,
    MOD_MM_16,
    MOD_RM,
};

static inline mode DecodeMode(u8 Byte)
{
    return (mode)((Byte & 0b11000000) >> 6);
}

static inline u32 DecodeReg0(u8 Byte, u32 WordFlag)
{
    return ((Byte & 0b00000111) << 1) | WordFlag;
}

static inline u32 DecodeReg(u8 Byte, u32 WordFlag)
{
    return ((Byte & 0b00111000) >> 2) | WordFlag;
}

static inline u32 DecodeRM(u8 Byte)
{
    return Byte & 0b00000111;
}

static u32 StringEffectiveAddressCalculation(string* String, u32 RMField, mode Mode, i32 Address)
{
    switch (RMField)
    {
        case 0: 
        {
            String->Size = StringCopy(*String, String->Size, "[bx + si", 0, 8);
        } break;
        case 1:
        {
            String->Size = StringCopy(*String, String->Size, "[bx + di", 0, 8);
        } break;
        case 2:
        {
            String->Size = StringCopy(*String, String->Size, "[bp + si", 0, 8);
        } break;
        case 3:
        {
            String->Size = StringCopy(*String, String->Size, "[bp + di", 0, 8);
        } break;
        case 4:
        {
            String->Size = StringCopy(*String, String->Size, "[si", 0, 3);
        } break;
        case 5:
        {
            String->Size = StringCopy(*String, String->Size, "[di", 0, 3);
        } break;
        case 6:
        {
            String->Size = StringCopy(*String, String->Size, "[", 0, 1);
        } break;
        case 7:
        {
            String->Size = StringCopy(*String, String->Size, "[bx", 0, 3);
        } break;
        default: InvalidCodePath;
    }
    
    if (Mode == MOD_MM)
    {
        if (RMField == 6)
        {
            String->Size = StringFromI32(*String, String->Size, Address);
        }
    }
    else
    {
        if (RMField == 6)
        {
            String->Size = StringCopy(*String, String->Size, "bp", 0, 2);
        }
        if (Address > 0)
        {
            String->Size = StringCopy(*String, String->Size, " + ", 0, 3);
            String->Size = StringFromI32(*String, String->Size, Address);
        }
        else if (Address < 0)
        {
            String->Size = StringCopy(*String, String->Size, " - ", 0, 3);
            String->Size = StringFromI32(*String, String->Size, -Address);
        }
    }
    String->Size = StringCopy(*String, String->Size, "]", 0, 1);
    
    return String->Size;
}

buffer Disassemble8086(memory_arena* Arena, buffer MachineCode)
{
    buffer Result = {};
    Result.Data = ArenaPushArray(u8, Arena, 4096);
    Result.Size = StringCopy(Result, Result.Size, "bits 16\n\n", 0, 9);
    
    i32 Count = MachineCode.Size;
    u8* Data = MachineCode.Data;
    while (Count > 0)
    {
        u8 Byte0 = Data[0];
        
        opcode Opcode = DecodeOpcode(Byte0);
        
        switch (Opcode)
        {
            case OP_MOV_RM_R:
            {
                b32 DirectionFlag = DecodeDirectionFlag(Byte0);
                u32 WordFlag = DecodeWordFlag(Byte0);
                
                u8 Byte1 = Data[1];
                mode ModeField = DecodeMode(Byte1);
                switch (ModeField)
                {
                    case MOD_MM:
                    {
                        u32 RegisterField = DecodeReg(Byte1, WordFlag);
                        u32 RMField = DecodeRM(Byte1);
                        
                        u32 DirectAddress = 0;
                        if (RMField == 6)
                        {
                            DirectAddress = (Data[3] << 8) | Data[2];
                        }
                        
                        Result.Size = StringCopy(Result, Result.Size, "mov ", 0, 4);
                        if (DirectionFlag)
                        {
                            Result.Size = StringCopy(Result, Result.Size, RegisterTable[RegisterField], 0, 2);
                            Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                            Result.Size = StringEffectiveAddressCalculation(&Result, RMField, ModeField, DirectAddress);
                        }
                        else
                        {
                            Result.Size = StringEffectiveAddressCalculation(&Result, RMField, ModeField, DirectAddress);
                            Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                            Result.Size = StringCopy(Result, Result.Size, RegisterTable[RegisterField], 0, 2);
                        }
                        Result.Size = StringCopy(Result, Result.Size, "\n", 0, 1);
                        
                        Count -= 2;
                        Data += 2;
                        if (RMField == 6)
                        {
                            Count -= 2;
                            Data += 2;
                        }
                    } break;
                    case MOD_MM_8:
                    {
                        u32 RegisterField = DecodeReg(Byte1, WordFlag);
                        u32 RMField = DecodeRM(Byte1);
                        
                        i32 Address = (i8)Data[2];
                        Result.Size = StringCopy(Result, Result.Size, "mov ", 0, 4);
                        if (DirectionFlag)
                        {
                            Result.Size = StringCopy(Result, Result.Size, RegisterTable[RegisterField], 0, 2);
                            Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                            Result.Size = StringEffectiveAddressCalculation(&Result, RMField, ModeField, Address);
                        }
                        else
                        {
                            Result.Size = StringEffectiveAddressCalculation(&Result, RMField, ModeField, Address);
                            Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                            Result.Size = StringCopy(Result, Result.Size, RegisterTable[RegisterField], 0, 2);
                        }
                        Result.Size = StringCopy(Result, Result.Size, "\n", 0, 1);
                        
                        Count -= 3;
                        Data += 3;
                    } break;
                    case MOD_MM_16:
                    {
                        u32 RegisterField = DecodeReg(Byte1, WordFlag);
                        u32 RMField = DecodeRM(Byte1);
                        
                        i32 Address = (i16)((Data[3] << 8) | Data[2]);
                        Result.Size = StringCopy(Result, Result.Size, "mov ", 0, 4);
                        if (DirectionFlag)
                        {
                            Result.Size = StringCopy(Result, Result.Size, RegisterTable[RegisterField], 0, 2);
                            Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                            Result.Size = StringEffectiveAddressCalculation(&Result, RMField, ModeField, Address);
                        }
                        else
                        {
                            Result.Size = StringEffectiveAddressCalculation(&Result, RMField, ModeField, Address);
                            Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                            Result.Size = StringCopy(Result, Result.Size, RegisterTable[RegisterField], 0, 2);
                        }
                        Result.Size = StringCopy(Result, Result.Size, "\n", 0, 1);
                        
                        Count -= 4;
                        Data += 4;
                    } break;
                    case MOD_RM:
                    {
                        u32 RegisterField = DecodeReg(Byte1, WordFlag);
                        u32 RMField = DecodeReg0(Byte1, WordFlag);
                        
                        u32 DstRegisterIndex = RMField;
                        u32 SrcRegisterIndex = RegisterField;
                        
                        if (DirectionFlag)
                        {
                            DstRegisterIndex = RegisterField;
                            SrcRegisterIndex = RMField;
                        }
                        
                        Result.Size = StringCopy(Result, Result.Size, "mov ", 0, 4);
                        Result.Size = StringCopy(Result, Result.Size, RegisterTable[DstRegisterIndex], 0, 2);
                        Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                        Result.Size = StringCopy(Result, Result.Size, RegisterTable[SrcRegisterIndex], 0, 2);
                        Result.Size = StringCopy(Result, Result.Size, "\n", 0, 1);
                        
                        Count -= 2;
                        Data += 2;
                    } break;
                    default: InvalidCodePath;
                }
            } break;
            case OP_MOV_IM_RM:
            {
                u32 WordFlag = DecodeWordFlag(Byte0);
                
                u8 Byte1 = Data[1];
                mode ModeField = DecodeMode(Byte1);
                u32 RMField = DecodeRM(Byte1);
                
                i32 Address = 0;
                u32 DataOffset = 2;
                if (ModeField == MOD_MM_8)
                {
                    Address = (i8)Data[2];
                    DataOffset += 1;
                }
                else if (ModeField == MOD_MM_16 || RMField == 6)
                {
                    Address = (i16)((Data[3] << 8) | Data[2]);
                    DataOffset += 2;
                }
                
                i32 Immediate;
                if (WordFlag)
                {
                    Immediate = (Data[DataOffset + 1] << 8) | Data[DataOffset];
                    DataOffset += 2;
                }
                else
                {
                    Immediate = Data[DataOffset];
                    DataOffset += 1;
                }
                
                Result.Size = StringCopy(Result, Result.Size, "mov ", 0, 4);
                Result.Size = StringEffectiveAddressCalculation(&Result, RMField, ModeField, Address);
                Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                if (WordFlag)
                {
                    Result.Size = StringCopy(Result, Result.Size, "word ", 0, 5);
                }
                else
                {
                    Result.Size = StringCopy(Result, Result.Size, "byte ", 0, 5);
                }
                Result.Size = StringFromI32(Result, Result.Size, Immediate);
                Result.Size = StringCopy(Result, Result.Size, "\n", 0, 1);
                
                Count -= DataOffset;
                Data += DataOffset;
            } break;
            case OP_MOV_I_R:
            {
                u32 WordFlag = DecodeWordFlagIR(Byte0);
                u32 RegisterField = DecodeReg0(Byte0, WordFlag);
                
                u8 Byte1 = Data[1];
                i32 Immediate = Byte1;
                if (WordFlag)
                {
                    u8 Byte2 = Data[2];
                    Immediate = ((i32)Byte2 << 8) | Immediate;
                }
                
                Result.Size = StringCopy(Result, Result.Size, "mov ", 0, 4);
                Result.Size = StringCopy(Result, Result.Size, RegisterTable[RegisterField], 0, 2);
                Result.Size = StringCopy(Result, Result.Size, ", ", 0, 2);
                Result.Size = StringFromI32(Result, Result.Size, Immediate);
                Result.Size = StringCopy(Result, Result.Size, "\n", 0, 1);
                
                Count -= 2;
                Data += 2;
                if (WordFlag)
                {
                    Count -= 1;
                    Data += 1;
                }
            } break;
            case OP_MOV_M_A:
            {
                //u32 WordFlag = DecodeWordFlag(Byte0);
                
                u32 Address = (Data[2] << 8) | Data[1];
                
                Result.Size = StringCopy(Result, Result.Size, "mov ax, [", 0, 9);
                Result.Size = StringFromI32(Result, Result.Size, Address);
                Result.Size = StringCopy(Result, Result.Size, "]\n", 0, 2);
                
                Count -= 3;
                Data += 3;
            } break;
            case OP_MOV_A_M:
            {
                u32 Address = (Data[2] << 8) | Data[1];
                
                Result.Size = StringCopy(Result, Result.Size, "mov [", 0, 5);
                Result.Size = StringFromI32(Result, Result.Size, Address);
                Result.Size = StringCopy(Result, Result.Size, "], ax\n", 0, 6);
                
                Count -= 3;
                Data += 3;
            } break;
            default: InvalidCodePath;
        }
    }
    
    return Result;
}

buffer Disassemble8086(buffer MachineCode)
{
    memory_arena Arena = {};
    Arena.Size = Megabytes(16);
    Arena.Base = (u8*)VirtualAlloc(0, Arena.Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return Disassemble8086(&Arena, MachineCode);
}

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

instruction_bit_field MakeDisplacementBitField(u32 Offset, u32 CurrentMachineCodeIndex, buffer MachineCode)
{
    instruction_bit_field Result = {};
    Result.BitFieldType = InstructionBitFieldType_Displacement;
    Result.Size = 8;
    Result.Offset = Offset;
    Result.Value = GetBitFieldValue(Result, CurrentMachineCodeIndex, MachineCode);
    return Result;
}

instruction ValidateInstructionAndFillValues(instruction_encoding InstructionEncoding, u32 CurrentMachineCodeIndex, buffer MachineCode)
{
    instruction Result = {InstructionEncoding.Type};
    u32 ResultBitFieldIndex = 0;
    u32 UsedBits = 0;
    u8 ModValue = 255;
    
    for (u32 InstructionBitFieldIndex = 0;
         InstructionBitFieldIndex < BIT_FIELD_COUNT;
         ++InstructionBitFieldIndex)
    {
        instruction_bit_field InstructionBitField = InstructionEncoding.Fields[InstructionBitFieldIndex];
        if (InstructionBitField.BitFieldType > InstructionBitFieldType_None)
        {
            u8 BitFieldValue = GetBitFieldValue(InstructionBitField, CurrentMachineCodeIndex, MachineCode);
            if (InstructionBitField.BitFieldType == InstructionBitFieldType_Bits)
            {
                if (BitFieldValue != InstructionBitField.Value)
                {
                    return {InstructionType_None};
                }
            }
            else
            {
                InstructionBitField.Value = BitFieldValue;
            }
            
            Result.Fields[ResultBitFieldIndex++] = InstructionBitField;
            UsedBits += InstructionBitField.Size;
            
            if (InstructionBitField.BitFieldType == InstructionBitFieldType_Mod)
            {
                ModValue = BitFieldValue;
            }
            // NOTE(alex): R/M is always followed by a displacement
            if (InstructionBitField.BitFieldType == InstructionBitFieldType_RM)
            {
                Assert(ModValue < 255);
                
                if (ModValue == 1)
                {
                    Result.Fields[ResultBitFieldIndex++] = MakeDisplacementBitField(UsedBits, CurrentMachineCodeIndex, MachineCode);
                    UsedBits += 8;
                }
                else if (ModValue == 2 || (ModValue == 3 && BitFieldValue == 6))
                {
                    Result.Fields[ResultBitFieldIndex++] = MakeDisplacementBitField(UsedBits, CurrentMachineCodeIndex, MachineCode);
                    UsedBits += 8;
                    Result.Fields[ResultBitFieldIndex++] = MakeDisplacementBitField(UsedBits, CurrentMachineCodeIndex, MachineCode);
                    UsedBits += 8;
                }
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

u32 WriteInstructionOperand(string Output, u32 Offset, instruction_operand Operand)
{
    if (Operand.Type == OperandType_Register)
    {
        char *Name = RegisterName[Operand.RM][Operand.Wide];
        Offset = StringCopy(Output, Offset, Name);
    }
    
    return Offset;
}

u32 WriteInstruction(string Output, u32 Offset, instruction Instruction)
{
    Offset = StringCopy(Output, Offset, InstructionTypeToName[Instruction.Type]);
    
    if (Instruction.OperandCount > 0)
    {
        Offset = StringCopy(Output, Offset, " ");
        Offset = WriteInstructionOperand(Output, Offset, Instruction.Operands[0]);
    }
    if (Instruction.OperandCount > 1)
    {
        Offset = StringCopy(Output, Offset, ", ");
        Offset = WriteInstructionOperand(Output, Offset, Instruction.Operands[1]);
    }
    
    Offset = StringCopy(Output, Offset, "\n");
    return Offset;
}

buffer Disassemble8086_(memory_arena *Arena, buffer MachineCode)
{
    buffer Result = {};
    Result.Data = ArenaPushArray(u8, Arena, Kilobytes(8));
    Result.Size = StringCopy(Result, Result.Size, "bits 16\n\n");
    
    u32 CurrentMachineCodeIndex = 0;
    while (CurrentMachineCodeIndex < MachineCode.Size)
    {
        instruction Instruction = DecodeInstruction(CurrentMachineCodeIndex, MachineCode);
        CurrentMachineCodeIndex += Instruction.ByteCount;
        Result.Size = WriteInstruction(Result, Result.Size, Instruction);
    }
    
    return Result;
}