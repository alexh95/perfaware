@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

SET ASM_DIRECTORY=..\computer_enhance\perfaware\part1\
SET OUTPUT_DIR=.\build\output\
SET INPUT_FILE_PREFIX=disassembled_
SET OUTPUT_FILE_PREFIX=reassembled_

FOR %%F IN ("%OUTPUT_DIR%%INPUT_FILE_PREFIX%*") DO (
    IF "%%~xF"==".asm" (
        SET FILE_NAME=%%~nF
        SET PURE_FILE_NAME=!FILE_NAME:%INPUT_FILE_PREFIX%=!

        SET REASSEMBLED_FILE_NAME=%OUTPUT_DIR%%OUTPUT_FILE_PREFIX%!PURE_FILE_NAME!
        ECHO Reassembling %%~nF into !REASSEMBLED_FILE_NAME!
        nasm %%F -o !REASSEMBLED_FILE_NAME!
     
        SET ORIGINAL_FILE_NAME=%ASM_DIRECTORY%!PURE_FILE_NAME!
        ECHO Comparing !REASSEMBLED_FILE_NAME! and !ORIGINAL_FILE_NAME!
        FC /b !REASSEMBLED_FILE_NAME! !ORIGINAL_FILE_NAME! > NUL
        
        IF ERRORLEVEL 1 (
            ECHO XXX The files are different
        ) ELSE (
            ECHO === The files are the same
        )
    )
)

ENDLOCAL
