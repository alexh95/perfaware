@ECHO OFF

CALL vcvarsall x64
CALL win_debug_build.bat
START remedybg debug.rdbg
START 4ed project.4coder
