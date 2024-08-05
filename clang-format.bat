@echo off
set DIRECTORIES=.\RHI;
for /r %DIRECTORIES% %%f in (*.cpp *.hpp) do (
    echo Formatting file: "%%~nf"
    clang-format.exe -i "%%f" > nul
)
