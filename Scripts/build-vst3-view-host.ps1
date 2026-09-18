param([string]$ToolsRoot=(Join-Path $env:LOCALAPPDATA 'ChordCanvasDev/Tools'))
$ErrorActionPreference='Stop'
$ccRoot=Split-Path $PSScriptRoot -Parent
$ccSdk=Join-Path $ccRoot '_deps/JUCE/modules/juce_audio_processors_headless/format_types/VST3_SDK'
$ccRevision=(& git -C (Join-Path $ccRoot '_deps/JUCE') rev-parse HEAD).Trim()
if($LASTEXITCODE -or $ccRevision -cne '72782788ce18c2d4d760b28e0921d6ffc6431102'){throw 'Expected the inspected JUCE 9.0.2 SDK checkout'}
$ccCompiler=Join-Path $ToolsRoot 'llvm-mingw-20260908-ucrt-x86_64/bin/clang++.exe'
$ccOutput=Join-Path $ccRoot 'build/vst3_view_host.exe'
[IO.Directory]::CreateDirectory((Split-Path $ccOutput -Parent)) | Out-Null
& $ccCompiler -std=c++20 -O2 -g -static -municode -D_WIN32_WINNT=0x0A00 -D_M_AMD64=100 -DNOMINMAX -DWIN32_LEAN_AND_MEAN ('-I'+$ccSdk) (Join-Path $ccRoot 'Tests/Integration/Vst3ViewHost.cpp') -o $ccOutput -lole32 -luser32
if($LASTEXITCODE){throw 'Development VST3 view host compilation failed'}
Write-Output 'Compiled development-only VST3 view host. Supply an explicit native module; no audio processing, Live identity or host acceptance is provided.'
