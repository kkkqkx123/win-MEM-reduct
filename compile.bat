@echo off
echo Setting up Visual Studio environment...

REM Try to find and load the Visual Studio environment
if exist "D:\softwares\Visual Studio\VC\Auxiliary\Build\vcvars64.bat" (
    call "D:\softwares\Visual Studio\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo Visual Studio environment not found!
    exit /b 1
)

echo Compiling project...
msbuild memreduct.sln /p:Configuration=Release /p:Platform=x64

pause