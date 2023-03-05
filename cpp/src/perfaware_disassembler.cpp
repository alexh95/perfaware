enum opcode
{
    MOV
};

static inline opcode DecodeOpcode(u8 Byte)
{
    return MOV;
}

static inline b32 DecodeDirectionFlag(u8 Byte)
{
    return Byte & 0x2;
}

static inline b32 DecodeWordFlag(u8 Byte)
{
    return Byte & 0x1;
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
        Result = (Byte & 0x07) << 1;
    }
    else
    {
        // 00111000
        Result = (Byte & 0x38) >> 2;
    }
    if (WordFlag)
    {
        Result = Result | 1;
    }
    return Result;
}

buffer Disassemble8086(memory_arena* Arena, buffer MachineCode)
{
    buffer Result = {};
    Result.Data = ArenaPushArray(u8, Arena, 4096);
    Result.Size = StringCopy(Result, Result.Size, "bits 16\r\n\r\n", 0, 11);
    
    i32 Count = MachineCode.Size;
    u8* Data = MachineCode.Data;
    while (Count > 0)
    {
        u8 Byte0 = Data[0];
        u8 Byte1 = Data[1];
        
        opcode Opcode = DecodeOpcode(Byte0);
        b32 DirectionFlag = DecodeDirectionFlag(Byte0);
        b32 WordFlag = DecodeWordFlag(Byte0);
        
        u32 RegisterField = DecodeRegisterField(Byte1, WordFlag, false);
        u32 RMField = DecodeRegisterField(Byte1, WordFlag, true);
        
        switch (Opcode)
        {
            case MOV:
            {
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
                Result.Size = StringCopy(Result, Result.Size, "\r\n", 0, 2);
            } break;
            default: InvalidCodePath;
        }
        
        Count -= 2;
        Data += 2;
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
