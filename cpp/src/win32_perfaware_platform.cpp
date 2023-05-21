#include "perfaware_types.h"

memory_arena Win32InitMemoryArena(umm Size) {
    memory_arena Result = {};
    Result.Size = Size;
    Result.Base = (u8*)VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return Result;
}

PLATFORM_OPEN_AND_READ_FILE(Win32OpenAndReadFile)
{
    char ZeroTerminatedFileName[1024] = {};
    StringCopyZero(ZeroTerminatedFileName, FileName);
    
    HANDLE FileHandle = CreateFileA(ZeroTerminatedFileName, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        InvalidCodePath;
    }
    BY_HANDLE_FILE_INFORMATION FileInformation = {};
    if (GetFileInformationByHandle(FileHandle, &FileInformation) == 0)
    {
        InvalidCodePath;
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
    char ZeroTerminatedFileName[1024] = {};
    StringCopyZero(ZeroTerminatedFileName, FileName);
    
    HANDLE FileHandle = CreateFileA(ZeroTerminatedFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        InvalidCodePath;
    }
    
    WriteFile(FileHandle, Buffer.Data, (u32)Size, 0, 0);
    CloseHandle(FileHandle);
}
