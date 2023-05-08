#include <Windows.h>
#include "win32_perfaware_platform.cpp"
#include "perfaware_string.cpp"
#include "perfaware_instruction.h"

int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd)
{
    memory_arena Arena = Win32InitMemoryArena(Megabytes(16));
    
    buffer InstructionSpec = Win32OpenAndReadFile("..\\resources\\perfaware_instruction.spec");
    string_list InstructionSpecLines = StringSplit(&Arena, InstructionSpec, '\n');
    
    u32 InstructionTypeCount = 0;
    u32 InstructionCount = 0;
    for (u32 LineIndex = 0; LineIndex < InstructionSpecLines.Count; ++LineIndex)
    {
        string Line = InstructionSpecLines.Strings[LineIndex];
        
        if (Line.Data[0] == '$')
        {
            InstructionTypeCount++;
        }
        else if (Line.Data[0] == '+')
        {
            InstructionCount++;
        }
    }
    
    string CurrentInstructionName;
    string_list InstructionTypeNames = StringList(&Arena, InstructionTypeCount);
    instruction_type *InstructionIndexToType = ArenaPushArray(instruction_type, &Arena, InstructionCount);
    instruction_bit_field *InstructionBitFields = ArenaPushArray(instruction_bit_field, &Arena, InstructionCount * BIT_FIELD_COUNT);
    u32 InstructionNameIndex = 0;
    u32 InstructionIndex = 0;
    
    for (u32 LineIndex = 0; LineIndex < InstructionSpecLines.Count; ++LineIndex)
    {
        string Line = InstructionSpecLines.Strings[LineIndex];
        
        if (Line.Data[0] == '$')
        {
            CurrentInstructionName = StringI(Line, 1, Line.Size);
            InstructionTypeNames.Strings[InstructionNameIndex++] = CurrentInstructionName;
        }
        else if (Line.Data[0] == '+')
        {
            InstructionIndexToType[InstructionIndex] = (instruction_type)(InstructionNameIndex - 1);
            string_list BitFieldSplit = StringSplit(&Arena, Line, ' ');
            u32 BitFieldIndex = 0;
            u32 Offset = 0;
            for (u32 BitFieldSplitIndex = 0; BitFieldSplitIndex < BitFieldSplit.Count; ++BitFieldSplitIndex)
            {
                string BitFieldString = BitFieldSplit.Strings[BitFieldSplitIndex];
                instruction_bit_field InstructionBitField = {};
                if (BitFieldString.Data[0] == '0' || BitFieldString.Data[1] == '0')
                {
                    InstructionBitField.BitFieldType = InstructionBitFieldType_Bits;
                    InstructionBitField.Size = BitFieldString.Size;
                }
                else if (BitFieldString.Data[0] == 'd')
                {
                    InstructionBitField.BitFieldType = InstructionBitFieldType_Direction;
                    InstructionBitField.Size = 1;
                }
                else if (BitFieldString.Data[0] == 'w')
                {
                    InstructionBitField.BitFieldType = InstructionBitFieldType_Word;
                    InstructionBitField.Size = 1;
                }
                else if (BitFieldString.Data[0] == '|')
                {
                    // TODO(alex): check consistency
                    // TODO(alex): check if last?
                }
                
                if (InstructionBitField.BitFieldType != InstructionBitFieldType_None)
                {
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
    LastIndex = StringCopy(Output, LastIndex, "enum instruction_type : u32\n");
    LastIndex = StringCopy(Output, LastIndex, "{\n");
    LastIndex = StringCopy(Output, LastIndex, "    InstructionType_None = 0,\n");
    
    for (u32 Index = 0; Index < InstructionTypeNames.Count; ++Index)
    {
        LastIndex = StringCopy(Output, LastIndex, "    InstructionType_");
        string InstructionName = InstructionTypeNames.Strings[Index];
        LastIndex = StringCopy(Output, LastIndex, InstructionName);
        LastIndex = StringCopy(Output, LastIndex, ",\n");
    }
    
    LastIndex = StringCopy(Output, LastIndex, "    InstructionType_Count\n");
    LastIndex = StringCopy(Output, LastIndex, "};\n");
    LastIndex = StringCopy(Output, LastIndex, "\n");
    
    LastIndex = StringCopy(Output, LastIndex, "char *InstructionTypeToName[] = {\n");
    LastIndex = StringCopy(Output, LastIndex, "    0,\n");
    for (u32 Index = 0; Index < InstructionTypeNames.Count; ++Index)
    {
        string InstructionName = InstructionTypeNames.Strings[Index];
        LastIndex = StringCopy(Output, LastIndex, "\"");
        LastIndex = StringCopy(Output, LastIndex, InstructionName);
        LastIndex = StringCopy(Output, LastIndex, "\",\n");
    }
    LastIndex = StringCopy(Output, LastIndex, "    0\n");
    LastIndex = StringCopy(Output, LastIndex, "};\n");
    LastIndex = StringCopy(Output, LastIndex, "\n");
    
    LastIndex = StringCopy(Output, LastIndex, "instruction_encoding InstructionList[] = {\n");
    for (InstructionIndex = 0; InstructionIndex < InstructionCount; ++InstructionIndex)
    {
        LastIndex = StringCopy(Output, LastIndex, "    {");
        LastIndex = StringCopy(Output, LastIndex, "InstructionType_");
        LastIndex = StringCopy(Output, LastIndex, InstructionTypeNames.Strings[InstructionIndexToType[InstructionIndex]]);
        LastIndex = StringCopy(Output, LastIndex, ", {");
        for (u32 BitFieldIndex = 0; BitFieldIndex < BIT_FIELD_COUNT; ++BitFieldIndex)
        {
            instruction_bit_field BitField = InstructionBitFields[BIT_FIELD_COUNT * InstructionIndex + BitFieldIndex];
            if (BitField.BitFieldType > InstructionBitFieldType_None)
            {
                LastIndex = StringCopy(Output, LastIndex, "{");
                LastIndex = StringCopy(Output, LastIndex, "},");
            }
        }
        LastIndex = StringCopy(Output, LastIndex, "}");
        LastIndex = StringCopy(Output, LastIndex, "},\n");
    }
    LastIndex = StringCopy(Output, LastIndex, "};\n");
    
    LastIndex = StringCopy(Output, LastIndex, "\n");
    LastIndex = StringCopy(Output, LastIndex, "#endif\n");
    Win32CreateAndWriteFile("..\\src\\perfaware_instruction_set.h", Output, LastIndex);
    
    return 0;
}