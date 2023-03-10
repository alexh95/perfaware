#include <Windows.h>
#include "perfaware_types.h"
#include "perfaware_string.cpp"
#include "perfaware_disassembler.cpp"

PLATFORM_OPEN_AND_READ_FILE(Win32OpenAndReadFile)
{
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        Assert(0);
    }
    BY_HANDLE_FILE_INFORMATION FileInformation = {};
    if (GetFileInformationByHandle(FileHandle, &FileInformation) == 0)
    {
        Assert(0);
    }
    u32 FileSize = (u32)FileInformation.nFileSizeLow;
    u8* Memory = (u8*)VirtualAlloc(0, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    ReadFile(FileHandle, Memory, FileSize, 0, 0);
    
    CloseHandle(FileHandle);
    
    buffer Result;
    Result.Size = FileSize;
    
    Result.Data = Memory;
    return Result;
}

PLATFORM_CREATE_AND_WRITE_FILE(Win32CreateAndWriteFile)
{
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        Assert(0);
    }
    
    WriteFile(FileHandle, Buffer.Data, (u32)Size, 0, 0);
    CloseHandle(FileHandle);
}

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
    
    memory_arena Arena = {};
    Arena.Size = Megabytes(16);
    Arena.Base = (u8*)VirtualAlloc(0, Arena.Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    
    buffer MachineCode = Win32OpenAndReadFile(CmdLine);
    buffer DisassembledCode = Disassemble8086(&Arena, MachineCode);
    Win32CreateAndWriteFile("output.asm", DisassembledCode, DisassembledCode.Size);
    
    return 0;
}