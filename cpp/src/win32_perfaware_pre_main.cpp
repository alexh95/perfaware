#include <Windows.h>
#include "perfaware_string.cpp"
#include "win32_perfaware_platform.cpp"
#include "perfaware_instruction.h"

u32 DecBinDec(u32 Value)
{
    u32 Result = 0;
    u32 FlippedResult = 0;
    u32 Count = 0;
    while (Value > 0)
    {
        FlippedResult = FlippedResult * 10 + (Value & 1);
        Value = Value >> 1;
        ++Count;
    }
    for (u32 Index = 0; Index < Count; ++Index)
    {
        Result = Result * 10 + (FlippedResult % 10);
        FlippedResult = FlippedResult / 10;
    }
    return Result;
}

int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd)
{
    memory_arena Arena = Win32InitMemoryArena(Megabytes(16));
    
    buffer InstructionSpec = Win32OpenAndReadFile(String("..\\resources\\perfaware_instruction.spec"));
    string_list InstructionSpecLines = StringSplit(&Arena, InstructionSpec, '\n');
    
    u32 InstructionOperationTypeCount = 0;
    u32 InstructionCount = 0;
    for (u32 LineIndex = 0; LineIndex < InstructionSpecLines.Count; ++LineIndex)
    {
        string Line = InstructionSpecLines.Strings[LineIndex];
        
        if (Line.Data[0] == '$')
        {
            InstructionOperationTypeCount++;
        }
        else if (Line.Data[0] == '+')
        {
            InstructionCount++;
        }
    }
    
    string CurrentInstructionName;
    string_list InstructionOperationTypeNames = StringList(&Arena, InstructionOperationTypeCount);
    instruction_operation_type *InstructionIndexToType = ArenaPushArray(instruction_operation_type, &Arena, InstructionCount);
    instruction_bit_field *InstructionBitFields = ArenaPushArray(instruction_bit_field, &Arena, InstructionCount * BIT_FIELD_COUNT);
    u32 InstructionNameIndex = 0;
    u32 InstructionIndex = 0;
    
    for (u32 LineIndex = 0; LineIndex < InstructionSpecLines.Count; ++LineIndex)
    {
        string Line = InstructionSpecLines.Strings[LineIndex];
        
        if (Line.Data[0] == '$')
        {
            CurrentInstructionName = StringI(Line, 1, Line.Size);
            InstructionOperationTypeNames.Strings[InstructionNameIndex++] = CurrentInstructionName;
        }
        else if (Line.Data[0] == '+')
        {
            InstructionIndexToType[InstructionIndex] = (instruction_operation_type)(InstructionNameIndex - 1);
            string_list BitFieldSplit = StringSplit(&Arena, Line, 1, ' ');
            u32 BitFieldIndex = 0;
            u32 Offset = 0;
            b32 ImplicitPresent = false;
            s32 ImplicitValue = 0;
            
            for (u32 BitFieldSplitIndex = 0; BitFieldSplitIndex < BitFieldSplit.Count; ++BitFieldSplitIndex)
            {
                string BitFieldString = BitFieldSplit.Strings[BitFieldSplitIndex];
                instruction_bit_field InstructionBitField = {};
                if (BitFieldString.Data[0] == '0' || BitFieldString.Data[0] == '1')
                {
                    InstructionBitField.Type = InstructionBitFieldType_Bits;
                    InstructionBitField.Size = BitFieldString.Size;
                    InstructionBitField.Value = StringToS32(BitFieldString);
                }
                else if (StringCompare(BitFieldString, "d"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_Direction;
                    InstructionBitField.Size = 1;
                }
                else if (StringCompare(BitFieldString, "s"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_SignExtension;
                    InstructionBitField.Size = 1;
                }
                else if (StringCompare(BitFieldString, "w"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_Word;
                    InstructionBitField.Size = 1;
                }
                else if (StringCompare(BitFieldString, "mod"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_Mod;
                    InstructionBitField.Size = 2;
                }
                else if (StringCompare(BitFieldString, "reg"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_Reg;
                    InstructionBitField.Size = 3;
                }
                else if (StringCompare(BitFieldString, "r/m"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_RM;
                    InstructionBitField.Size = 3;
                }
                else if (StringCompare(BitFieldString, "SR"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_SR;
                    InstructionBitField.Size = 2;
                }
                else if (StringCompare(BitFieldString, "data"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_Data;
                    InstructionBitField.Size = 8;
                }
                else if (StringCompare(BitFieldString, "data-w"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_DataW;
                    InstructionBitField.Size = 8;
                }
                else if (StringCompare(BitFieldString, "data-sw"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_DataSW;
                    InstructionBitField.Size = 8;
                }
                else if (StringCompare(BitFieldString, "addr-lo"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_AddrLo;
                    InstructionBitField.Size = 8;
                }
                else if (StringCompare(BitFieldString, "addr-hi"))
                {
                    InstructionBitField.Type = InstructionBitFieldType_AddrHi;
                    InstructionBitField.Size = 8;
                }
                else if (StringStartsWith(BitFieldString, "implicit"))
                {
                    u32 ImplicitParameterStartIndex = StringFirstIndexOf(BitFieldString, 0, ':');
                    string Parameter = StringI(BitFieldString, ImplicitParameterStartIndex + 1, BitFieldString.Size);
                    
                    ImplicitPresent = true;
                    ImplicitValue = StringToS32(Parameter);
                }
                else if (BitFieldString.Data[0] == '|')
                {
                    Assert(Offset % 8 == 0);
                }
                else if (BitFieldString.Data[0] != ' ')
                {
                    InvalidCodePath;
                }
                
                if (InstructionBitField.Type != InstructionBitFieldType_None)
                {
                    if (ImplicitPresent)
                    {
                        ImplicitPresent = false;
                        // TODO(alex): huh
                        InstructionBitField.Value = DecBinDec(ImplicitValue);
                        InstructionBitField.Size = 0;
                    }
                    
                    Assert(BitFieldIndex < BIT_FIELD_COUNT);
                    InstructionBitField.Offset = Offset;
                    Offset += InstructionBitField.Size;
                    InstructionBitFields[BIT_FIELD_COUNT * InstructionIndex + BitFieldIndex++] = InstructionBitField;
                }
            }
            ++InstructionIndex;
        }
    }
    
    string Output = ArenaPushString(&Arena, Kilobytes(4));
    u32 LastIndex = StringCopy(Output, 0, "#ifndef PERFAWARE_INSTRUCTION_SET_H\n");
    LastIndex = StringCopy(Output, LastIndex, "#define PERFAWARE_INSTRUCTION_SET_H\n");
    LastIndex = StringCopy(Output, LastIndex, "\n");
    LastIndex = StringCopy(Output, LastIndex, "enum instruction_operation_type : u32\n");
    LastIndex = StringCopy(Output, LastIndex, "{\n");
    LastIndex = StringCopy(Output, LastIndex, "    InstructionOperationType_None = 0,\n");
    
    for (u32 Index = 0; Index < InstructionOperationTypeNames.Count; ++Index)
    {
        LastIndex = StringCopy(Output, LastIndex, "    InstructionOperationType_");
        string InstructionOperationName = InstructionOperationTypeNames.Strings[Index];
        LastIndex = StringCopy(Output, LastIndex, InstructionOperationName);
        LastIndex = StringCopy(Output, LastIndex, ",\n");
    }
    
    LastIndex = StringCopy(Output, LastIndex, "    InstructionOperationType_Count,\n");
    LastIndex = StringCopy(Output, LastIndex, "};\n");
    LastIndex = StringCopy(Output, LastIndex, "\n");
    
    LastIndex = StringCopy(Output, LastIndex, "char *InstructionOperationTypeToName[] = {\n");
    LastIndex = StringCopy(Output, LastIndex, "    0,\n");
    for (u32 Index = 0; Index < InstructionOperationTypeNames.Count; ++Index)
    {
        string InstructionName = InstructionOperationTypeNames.Strings[Index];
        LastIndex = StringCopy(Output, LastIndex, "    \"");
        LastIndex = StringCopy(Output, LastIndex, InstructionName);
        LastIndex = StringCopy(Output, LastIndex, "\",\n");
    }
    LastIndex = StringCopy(Output, LastIndex, "    0,\n");
    LastIndex = StringCopy(Output, LastIndex, "};\n");
    LastIndex = StringCopy(Output, LastIndex, "\n");
    
    LastIndex = StringCopy(Output, LastIndex, "instruction_encoding InstructionEncodingList[] =\n");
    LastIndex = StringCopy(Output, LastIndex, "{\n");
    for (InstructionIndex = 0; InstructionIndex < InstructionCount; ++InstructionIndex)
    {
        LastIndex = StringCopy(Output, LastIndex, "    {\n");
        LastIndex = StringCopy(Output, LastIndex, "        InstructionOperationType_");
        LastIndex = StringCopy(Output, LastIndex, InstructionOperationTypeNames.Strings[InstructionIndexToType[InstructionIndex]]);
        LastIndex = StringCopy(Output, LastIndex, ",\n");
        LastIndex = StringCopy(Output, LastIndex, "        {\n");
        for (u32 BitFieldIndex = 0; BitFieldIndex < BIT_FIELD_COUNT; ++BitFieldIndex)
        {
            instruction_bit_field BitField = InstructionBitFields[BIT_FIELD_COUNT * InstructionIndex + BitFieldIndex];
            if (BitField.Type > InstructionBitFieldType_None)
            {
                LastIndex = StringCopy(Output, LastIndex, "            {");
                LastIndex = StringCopy(Output, LastIndex, InstructionBitFieldTypeStrings[BitField.Type]);
                LastIndex = StringCopy(Output, LastIndex, ", ");
                LastIndex = StringFromS32(Output, LastIndex, BitField.Size);
                LastIndex = StringCopy(Output, LastIndex, ", ");
                LastIndex = StringFromS32(Output, LastIndex, BitField.Offset);
                LastIndex = StringCopy(Output, LastIndex, ", 0b");
                LastIndex = StringFromS32(Output, LastIndex, BitField.Value);
                LastIndex = StringCopy(Output, LastIndex, "},\n");
            }
        }
        LastIndex = StringCopy(Output, LastIndex, "        },\n");
        LastIndex = StringCopy(Output, LastIndex, "    },\n");
    }
    LastIndex = StringCopy(Output, LastIndex, "};\n");
    
    LastIndex = StringCopy(Output, LastIndex, "\n");
    LastIndex = StringCopy(Output, LastIndex, "#endif\n");
    Win32CreateAndWriteFile(String("..\\src\\perfaware_instruction_set.h"), Output, LastIndex);
    
    return 0;
}