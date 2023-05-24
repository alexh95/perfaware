@ECHO OFF

SET CompilerOptions=-MTd -Oi -Od -WX -W4 -wd4100 -wd4201 -wd4530 -nologo -Gm- -GR -fp:fast -EHa- -EHsc- -Z7
SET LinkerFlags=-incremental:no User32.lib

IF NOT EXIST build MKDIR build

PUSHD build
cl %CompilerOptions% ..\src\win32_perfaware_pre_main.cpp /link %LinkerFlags%
.\win32_perfaware_pre_main.exe
cl %CompilerOptions% ..\src\win32_main.cpp /link %LinkerFlags%
POPD
