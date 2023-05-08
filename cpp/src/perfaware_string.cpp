#include "perfaware_types.h"

string StringI(string S, u32 FromIndex, u32 ToIndex)
{
    string Result;
    Result.Size = ToIndex - FromIndex;
    Result.Data = S.Data + FromIndex;
    return Result;
}

u32 StringCopy(char* Dst, u32 DstOffset, char* Src, u32 SrcOffset, u32 SrcCount)
{
    for (u32 Index = 0; Index < SrcCount; ++Index)
    {
        Dst[DstOffset + Index] = Src[SrcOffset + Index];
    }
    return DstOffset + SrcCount;
}

u32 inline StringCopy(string Dst, u32 DstOffset, char* Src, u32 SrcOffset, u32 SrcCount)
{
    return StringCopy((char*)Dst.Data, DstOffset, Src, SrcOffset, SrcCount);
}

u32 inline StringCopy(string Dst, u32 DstOffset, char* Src)
{
    return StringCopy((char*)Dst.Data, DstOffset, Src, 0, StringLength(Src));
}

u32 inline StringCopy(string Dst, u32 DstOffset, string Src)
{
    return StringCopy((char*)Dst.Data, DstOffset, (char*)Src.Data, 0, Src.Size);
}

b32 StringCompare(u8* A, u32 SizeA, u8* B, u32 SizeB)
{
    if (SizeA != SizeB)
    {
        return false;
    }
    
    for (u32 Index = 0; Index < SizeA; ++Index)
    {
        if (A[Index] != B[Index])
        {
            return false;
        }
    }
    
    return true;
}

inline b32 StringCompare(string A, string B)
{
    b32 Result = StringCompare(A.Data, A.Size, B.Data, B.Size);
    return Result;
}

struct string_list
{
    u32 Count;
    string* Strings;
};

inline string_list StringList(memory_arena *Arena, u32 Count)
{
    string_list Result;
    Result.Count = Count;
    Result.Strings = ArenaPushArray(string, Arena, Count);
    return Result;
}

string_list StringSplit(memory_arena *Arena, string S, u8 Delimiter)
{
    string_list Result = {};
    
    u32 MatchCount = 0;
    for (u32 Index = 0; Index < S.Size; ++Index)
    {
        u8 C = S.Data[Index];
        u32 Match = (C == Delimiter) || (Index == S.Size - 1);
        if (Match)
        {
            ++MatchCount;
        }
    }
    Result = StringList(Arena, MatchCount);
    
    u32 PrevStartIndex = 0;
    u32 MatchIndex = 0;
    for (u32 Index = 0; Index < S.Size; ++Index)
    {
        u8 C = S.Data[Index];
        
        u32 Match = (C == Delimiter) || (Index == S.Size - 1);
        if (Match)
        {
            string NewString;
            NewString.Size = Index - PrevStartIndex + ((C == Delimiter) ? 0 : 1);
            NewString.Data = S.Data + PrevStartIndex;
            Result.Strings[MatchIndex++] = NewString;
            PrevStartIndex = Index + 1;
        }
    }
    
    return Result;
}

i32 StringFirstIndexOf(string S, u32 Offset, u8 Delimiter)
{
    for (u32 Index = Offset; Index < S.Size; ++Index)
    {
        u8 C = S.Data[Index];
        if (C == Delimiter)
        {
            return Index;
        }
    }
    return -1;
}

inline void StringLowerCase(string String)
{
    for (u32 Index = 0; Index < String.Size; ++Index)
    {
        u8 Character = String.Data[Index];
        if ('A' <= Character && Character <= 'Z')
        {
            String.Data[Index] += 32;
        }
    }
}

#define MAX_STRING_LENGTH_I32 11
u32 StringFromI32(string S, u32 Offset, i32 Value, u32 MinLength, b32 LeadingZeros)
{
    u8 Characters[MAX_STRING_LENGTH_I32] = {};
    u32 Size = 0;
    b32 Negative = false;
    if (Value < 0)
    {
        Negative = true;
        Value = -Value;
    }
    if (Value == 0)
    {
        Characters[Size++] = '0';
    }
    while (Value > 0)
    {
        u8 Character = 48 + Value % 10;
        Characters[Size] = Character;
        ++Size;
        Value /= 10;
    }
    if (Negative)
    {
        if (MinLength > 0)
        {
            --MinLength;
        }
    }
    for (u32 Index = Size; Index < MinLength; ++Index)
    {
        u8 Filler = LeadingZeros ? '0' : ' ';
        Characters[Index] = Filler;
        ++Size;
    }
    if (Negative)
    {
        Characters[Size++] = '-';
    }
    u32 LastIndex = 0;
    for (u32 Index = 0; Index < Size; ++Index)
    {
        LastIndex = Offset + Index;
        S.Data[LastIndex] = Characters[Size - 1 - Index];
    }
    return LastIndex + 1;
}

u32 StringFromI32(string S, u32 Offset, i32 Value)
{
    u32 Result = StringFromI32(S, Offset, Value, 0, false);
    return Result;
}
