$ErrorActionPreference='Stop'
$ccRoot=Split-Path $PSScriptRoot -Parent
$ccJuce=Join-Path $ccRoot '_deps/JUCE'
$ccExpected='72782788ce18c2d4d760b28e0921d6ffc6431102'
if(!(Test-Path -LiteralPath $ccJuce)) {
    & git clone --depth 1 --branch 9.0.2 https://github.com/juce-framework/JUCE.git $ccJuce
    if($LASTEXITCODE){throw 'JUCE download failed'}
}
$ccActual=& git -C $ccJuce rev-parse HEAD
if($LASTEXITCODE -or $ccActual -ne $ccExpected){throw 'JUCE checkout is not the approved pinned revision; no existing checkout was overwritten'}
Write-Output "Verified JUCE 9.0.2 at $ccActual; framework route AGPLv3"
