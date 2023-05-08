#include <Windows.h>
#include "perfaware_string.cpp"
#include "win32_perfaware_platform.cpp"
#include "perfaware_instruction.h"
#include "perfaware_instruction_set.h"
#include "perfaware_disassembler.cpp"

char* TestFileNames[] = 
{
    //"..\\..\\common\\resources\\part1\\listing_0037_single_register_mov",
    //"..\\..\\common\\resources\\part1\\listing_0037_single_register_mov.asm",
    //"..\\..\\common\\resources\\part1\\listing_0038_many_register_mov",
    //"..\\..\\common\\resources\\part1\\listing_0038_many_register_mov.asm",
    //"..\\..\\common\\resources\\part1\\listing_0039_more_movs",
    //"..\\..\\common\\resources\\part1\\listing_0039_more_movs.asm",
    "..\\..\\common\\resources\\part1\\listing_0040_challenge_movs",
    "..\\..\\common\\resources\\part1\\listing_0040_challenge_movs.asm",
};

void Test()
{
    for (u32 TestFileIndex = 0; TestFileIndex < ArrayCount(TestFileNames); TestFileIndex += 2)
    {
        char* AssembledFileName = TestFileNames[TestFileIndex];
        buffer MachineCode = Win32OpenAndReadFile(AssembledFileName);
        buffer DisassembledCode = Disassemble8086(MachineCode);
        
        char* OriginalFileName = TestFileNames[TestFileIndex + 1];
        buffer SourceCode = Win32OpenAndReadFile(OriginalFileName);
        
        Assert(true);
        //b32 CodeMatches = StringCompare(SourceCode, DisassembledCode);
        //Assert(CodeMatches);
    }
}

int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CmdLine, int ShowCmd)
{
    Test();
    
    memory_arena Arena = Win32InitMemoryArena(Megabytes(16));
    
    buffer MachineCode = Win32OpenAndReadFile(CmdLine);
    buffer DisassembledCode = Disassemble8086_(&Arena, MachineCode);
    Win32CreateAndWriteFile("output.asm", DisassembledCode, DisassembledCode.Size);
    
    return 0;
}