param([string]$ToolsRoot=(Join-Path $env:LOCALAPPDATA 'ChordCanvasDev\Tools'))
$ErrorActionPreference='Stop'
$ccRoot=Split-Path $PSScriptRoot -Parent
$ccCmake=Join-Path $ToolsRoot 'cmake-4.4.3-windows-x86_64\bin\cmake.exe'
$ccBuild=Join-Path $ccRoot 'build/core'
if (-not (Test-Path -LiteralPath (Join-Path $ccBuild 'CMakeCache.txt'))) { throw 'Run Scripts/build-core.ps1 before measuring audio' }
& $ccCmake --build $ccBuild --target audio_probe --parallel 3
if ($LASTEXITCODE) { throw 'Audio probe build failed' }
$ccOutput=Join-Path $ccBuild 'audio-measurements.json'
& (Join-Path $ccBuild 'audio_probe.exe') | Set-Content -LiteralPath $ccOutput -Encoding utf8
if ($LASTEXITCODE) { throw 'Audio signal or guarded C++ allocation check failed; inspect the measurement file' }
$ccResults=Get-Content -LiteralPath $ccOutput -Raw | ConvertFrom-Json
if ($ccResults.signalAndAllocationChecks -ne 'PASS' -or $ccResults.cases.Count -ne 72) { throw 'Incomplete audio measurements' }
[pscustomobject]@{
    Cases=$ccResults.cases.Count
    MaximumAverageCpuFraction=($ccResults.cases.cpuFraction | Measure-Object -Maximum).Maximum
    MaximumCallbackDeadlineFraction=($ccResults.cases.maxCallbackDeadlineFraction | Measure-Object -Maximum).Maximum
    Scope=$ccResults.scope
} | ConvertTo-Json
