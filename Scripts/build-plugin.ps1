param([string]$ToolsRoot = (Join-Path $env:LOCALAPPDATA 'ChordCanvasDev\Tools'),[ValidateSet('Release','Debug')][string]$Configuration='Release',[string]$BuildDirectory='build/plugin-msvc',[switch]$VisualStudioLicenseConfirmed)
$ErrorActionPreference='Stop'
if(!$VisualStudioLicenseConfirmed){throw 'Confirm a valid Visual Studio Community/Professional/Enterprise licence covering project development before using -VisualStudioLicenseConfirmed. Standalone Build Tools terms only exempt third-party open-source dependencies.'}
$ccRoot=Split-Path $PSScriptRoot -Parent
& (Join-Path $PSScriptRoot 'fetch-dependencies.ps1')
$ccVswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if(!(Test-Path -LiteralPath $ccVswhere)){throw 'Microsoft C++ Build Tools is required. JUCE 9 does not support MinGW.'}
$ccInstallations=& $ccVswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json
if($LASTEXITCODE -or !$ccInstallations){throw 'No supported Microsoft x64 C++ toolchain was detected.'}
$ccInstallation=$ccInstallations | Select-Object -First 1
$ccMajor=[int]($ccInstallation.installationVersion.Split('.')[0])
if($ccMajor -notin @(17,18)){throw "Unverified Visual Studio generation: $ccMajor"}
$ccGenerator=if($ccMajor -eq 18){'Visual Studio 18 2026'}else{'Visual Studio 17 2022'}
$ccCmake=Join-Path $ToolsRoot 'cmake-4.4.3-windows-x86_64\bin\cmake.exe'
$ccBuild=[IO.Path]::GetFullPath((Join-Path $ccRoot $BuildDirectory))
$ccAllowedRoot=[IO.Path]::GetFullPath((Join-Path $ccRoot 'build'))+[IO.Path]::DirectorySeparatorChar
if(!$ccBuild.StartsWith($ccAllowedRoot,[StringComparison]::OrdinalIgnoreCase)){throw 'BuildDirectory must be inside the ignored workspace build folder'}
& $ccCmake -S $ccRoot -B $ccBuild -G $ccGenerator -A x64 "-DCMAKE_GENERATOR_INSTANCE=$($ccInstallation.installationPath)" '-DCMAKE_SYSTEM_VERSION=10.0.26100.0' '-DCHORDCANVAS_BUILD_PLUGIN=ON'
if($LASTEXITCODE){throw 'VST3 configure failed'}
& $ccCmake --build $ccBuild --config $Configuration --target ChordCanvas_VST3 core_tests log_tests theory_probe export_cache_tests package_version_tests payload_tests transaction_tests --parallel 3
if($LASTEXITCODE){throw 'VST3 build failed'}
& $ccCmake --build $ccBuild --config $Configuration --target RUN_TESTS
if($LASTEXITCODE){throw 'Microsoft-toolchain tests failed'}
Write-Output 'VST3 compilation and core tests completed; scan, GUI, host and installer acceptance still require actual evidence.'
