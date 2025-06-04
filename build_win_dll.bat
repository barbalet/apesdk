@echo off
REM build.bat
REM =============================================================
REM    Copyright 1996-2024 Tom Barbalet. All rights reserved.
REM    Permission is hereby granted, free of charge, to any person
REM    obtaining a copy of this software and associated documentation
REM    files (the "Software"), to deal in the Software without
REM    restriction, including without limitation the rights to use,
REM    copy, modify, merge, publish, distribute, sublicense, and/or
REM    sell copies of the Software, and to permit persons to whom the
REM    Software is furnished to do so, subject to the following
REM    conditions:
REM
REM    The above copyright notice and this permission notice shall be
REM    included in all copies or substantial portions of the Software.
REM
REM    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
REM    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
REM    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
REM    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
REM    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
REM    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
REM    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
REM    OTHER DEALINGS IN THE SOFTWARE.
REM
REM    This software is a continuing work of Tom Barbalet, begun on
REM    13 June 1996. No apes or cats were harmed in the writing of
REM    this software.

setlocal

REM Set compiler flags based on the input arguments
if "%1"=="--debug" (
    set CFLAGS=/Zi /Od
) else (
    set CFLAGS=/O2
)

REM Set command line define based on the input arguments
if "%1"=="--additional" (
    set COMMANDLINEE=/DNOTHING_NEEDED_HERE
) else (
    set COMMANDLINEE=/DCOMMAND_LINE_EXPLICIT
)



REM Call the subroutine to compile all C files in each directory
call :compile_dir toolkit
call :compile_dir script
call :compile_dir render
call :compile_dir sim
call :compile_dir gui
call :compile_dir dll
call :compile_dir entity
call :compile_dir universe

REM Link all object files to create the executable
cl %CFLAGS% %COMMANDLINEE% /LD *.obj /Fe:apesdk.dll

REM Clean up object files
del *.obj
exit /b

REM Subroutine to compile all C files in a specified directory
:compile_dir
setlocal
set DIR=%1
for %%f in (%DIR%\*.c) do (
    cl %CFLAGS% %COMMANDLINEE% /c %%f
    if errorlevel 1 exit /b 1
)
endlocal
exit /b 0
