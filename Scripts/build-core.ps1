param([string]$ToolsRoot = (Join-Path $env:LOCALAPPDATA 'ChordCanvasDev\Tools'),[string]$Configuration='Release')
$ErrorActionPreference='Stop'
$ccRoot=Split-Path $PSScriptRoot -Parent
$ccCompiler=Join-Path $ToolsRoot 'llvm-mingw-20260908-ucrt-x86_64\bin\clang++.exe'
$ccCmake=Join-Path $ToolsRoot 'cmake-4.4.3-windows-x86_64\bin\cmake.exe'
$ccMake=Join-Path $ToolsRoot 'llvm-mingw-20260908-ucrt-x86_64\bin\mingw32-make.exe'
& $ccCmake -S $ccRoot -B (Join-Path $ccRoot 'build/core') -G 'MinGW Makefiles' "-DCMAKE_CXX_COMPILER=$ccCompiler" "-DCMAKE_MAKE_PROGRAM=$ccMake" "-DCMAKE_BUILD_TYPE=$Configuration" '-DCHORDCANVAS_BUILD_PLUGIN=OFF'
if($LASTEXITCODE){throw 'Core configure failed'}
& $ccCmake --build (Join-Path $ccRoot 'build/core') --parallel 4
if($LASTEXITCODE){throw 'Core build failed'}
& (Join-Path $ToolsRoot 'cmake-4.4.3-windows-x86_64\bin\ctest.exe') --test-dir (Join-Path $ccRoot 'build/core') --output-on-failure
if($LASTEXITCODE){throw 'Core tests failed'}
