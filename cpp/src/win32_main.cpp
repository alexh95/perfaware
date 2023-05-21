#include <Windows.h>
#include "perfaware_string.cpp"
#include "win32_perfaware_platform.cpp"
#include "perfaware_instruction.h"
#include "perfaware_instruction_set.h"
#include "perfaware_disassembler.cpp"

int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd)
{
    memory_arena Arena = Win32InitMemoryArena(Megabytes(16));
    
    string_list Args = StringSplit(&Arena, String(CmdLine), ' ');
    string SrcFile = ArenaPushString(&Arena, Args.Strings[0].Size + Args.Strings[1].Size);
    StringCopy(SrcFile, Args.Strings[0]);
    StringCopy(SrcFile, Args.Strings[0].Size, Args.Strings[1]);
    
    string DstFilePrefix = String("disassembled_");
    string DstFileExtension = String(".asm");
    string DstFile = ArenaPushString(&Arena, DstFilePrefix.Size + Args.Strings[1].Size + DstFileExtension.Size);
    StringCopy(DstFile, DstFilePrefix);
    u32 DstFileIndex = StringCopy(DstFile, DstFilePrefix.Size, Args.Strings[1]);
    StringCopy(DstFile, DstFileIndex, DstFileExtension);
    
    buffer MachineCode = Win32OpenAndReadFile(SrcFile);
    buffer DisassembledCode = Disassemble8086(&Arena, MachineCode);
    
    Win32CreateAndWriteFile(DstFile, DisassembledCode, DisassembledCode.Size);
    
    return 0;
}
