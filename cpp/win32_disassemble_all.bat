@ECHO OFF
PUSHD build
MKDIR output
PUSHD output
SET ASM_DIRECTORY=..\..\..\computer_enhance\perfaware\part1\

FOR %%F IN ("%ASM_DIRECTORY%*") DO (
    IF "%%~xF"=="" (
        ECHO Disassembling %ASM_DIRECTORY% %%~nF
        ..\win32_main.exe %ASM_DIRECTORY% %%~nF
    )
)

POPD
POPD
