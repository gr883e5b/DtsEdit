@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

IF EXIST "%~dp0" CD /D "%~dp0"

rem 複数ファイルのループ処理
SET ARGV=%*
FOR %%I IN (!ARGV!) DO CALL :DTSEDIT "%%~I"
GOTO :EOF


:DTSEDIT
SET OUT_FILE=%~dpn1.nodelay%~x1
SET TC_FILE=%~dpn1.timecode.txt

DtsEdit "%~dpnx1" -o "%TC_FILE%"
DtsEdit "%~dpnx1" -tc "%TC_FILE%" -o "%OUT_FILE%" -mlt 4

del "%TC_FILE%"
GOTO :EOF

