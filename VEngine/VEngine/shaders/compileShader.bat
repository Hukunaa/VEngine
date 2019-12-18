@echo off

setlocal
set GLSL_COMPILER=glslangValidator.exe
set SOURCE_FOLDER=""
set BINARIES_FOLDER="bin/"

:: raygen shaders
%GLSL_COMPILER% -V -S rgen %SOURCE_FOLDER%ray_gen.glsl -o %BINARIES_FOLDER%ray_gen.spv

:: closest hit shaders
%GLSL_COMPILER% -V -S rchit %SOURCE_FOLDER%ray_chit.glsl -o %BINARIES_FOLDER%ray_chit.spv

:: miss shaders
%GLSL_COMPILER% -V -S rmiss %SOURCE_FOLDER%ray_miss.glsl -o %BINARIES_FOLDER%ray_miss.spv

:: miss shaders
%GLSL_COMPILER% -V -S rmiss %SOURCE_FOLDER%ray_smiss.glsl -o %BINARIES_FOLDER%ray_smiss.spv
pause