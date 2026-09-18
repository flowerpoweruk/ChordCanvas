param([Parameter(Mandatory=$true)][string]$OutputDirectory)
$ErrorActionPreference='Stop'
$ccRoot=Split-Path $PSScriptRoot -Parent
$ccJuce=Join-Path $ccRoot '_deps/JUCE'
$ccRevision=& git -C $ccJuce rev-parse HEAD
if($LASTEXITCODE -or $ccRevision -ne '72782788ce18c2d4d760b28e0921d6ffc6431102'){throw 'Notice inputs must be the pinned JUCE revision'}
$ccDestination=[IO.Path]::GetFullPath($OutputDirectory)
[IO.Directory]::CreateDirectory($ccDestination) | Out-Null
$ccInputs=[ordered]@{
    'JUCE.txt'='LICENSE.md'
    'VST3-SDK.txt'='modules/juce_audio_processors_headless/format_types/VST3_SDK/LICENSE.txt'
    'HarfBuzz.txt'='modules/juce_graphics/fonts/harfbuzz/COPYING'
    'SheenBidi-Apache-2.0.txt'='modules/juce_graphics/unicode/sheenbidi/LICENSE'
    'LunaSVG.txt'='modules/juce_graphics/drawables/lunasvg/LICENSE'
    'PlutoVG.txt'='modules/juce_graphics/drawables/lunasvg/plutovg/LICENSE'
    'libpng.txt'='modules/juce_graphics/image_formats/pnglib/LICENSE'
    'zlib.txt'='modules/juce_core/zip/zlib/LICENSE'
}
foreach($ccEntry in $ccInputs.GetEnumerator()) {
    Copy-Item -LiteralPath (Join-Path $ccJuce $ccEntry.Value) -Destination (Join-Path $ccDestination $ccEntry.Key)
}
Copy-Item -LiteralPath (Join-Path $ccRoot 'LICENSE') -Destination (Join-Path $ccDestination 'ChordCanvas-AGPL-3.0.txt')
$ccJpeg=Get-Content -LiteralPath (Join-Path $ccJuce 'modules/juce_graphics/image_formats/jpglib/README') -Raw
$ccLegalStart=$ccJpeg.IndexOf("LEGAL ISSUES`n============")
# Accept upstream LF or CRLF without changing the notice text itself.
if($ccLegalStart -lt 0){$ccLegalStart=$ccJpeg.IndexOf("LEGAL ISSUES`r`n============")}
$ccLegalEnd=$ccJpeg.IndexOf("REFERENCES",$ccLegalStart)
if($ccLegalStart -lt 0 -or $ccLegalEnd -le $ccLegalStart){throw 'Missing IJG legal terms; refuse incomplete notices'}
[IO.File]::WriteAllText((Join-Path $ccDestination 'Independent-JPEG-Group.txt'),$ccJpeg.Substring($ccLegalStart,$ccLegalEnd-$ccLegalStart),[Text.UTF8Encoding]::new($false))
$ccNotice=@'
ChordCanvas: copyright (C) 2026 ChordCanvas contributors. AGPL-3.0-only.
Corresponding source: https://github.com/flowerpoweruk/ChordCanvas
JUCE 9.0.2: copyright (c) Raw Material Software Limited; AGPLv3 route.
VST3 SDK 3.8.0: copyright (c) 2025 Steinberg Media Technologies GmbH; MIT.
HarfBuzz 14.2.1: copyrights and terms in HarfBuzz.txt.
SheenBidi 2.9.0: copyright (C) 2016-2025 Muhammad Tayyab Akram; Apache-2.0.
LunaSVG 3.5.0 and PlutoVG 1.3.2: copyright (c) 2020-2025 Samuel Ugochukwu; MIT.
libpng 1.6.58 and zlib 1.3.2: copyrights and terms in accompanying files.
This software is based in part on the work of the Independent JPEG Group.
PreSonus Plug-In Extensions: written and placed in the PUBLIC DOMAIN by
PreSonus Software Ltd. Provided AS IS; not part of an official third-party SDK.
Microsoft C/C++ runtime is linked in Release mode; no developer tools or debug
runtime files are included. Windows operating-system libraries and Segoe UI
remain supplied by Windows; no font file is copied or redistributed.
All four preview sounds use original synthesis; no third-party sound samples.
'@
[IO.File]::WriteAllText((Join-Path $ccDestination 'NOTICE.txt'),$ccNotice+"`n",[Text.UTF8Encoding]::new($false))
$ccReceipt=[ordered]@{juceRevision=$ccRevision;inputs=$ccInputs;files=@()}
foreach($ccFile in Get-ChildItem -LiteralPath $ccDestination -File | Sort-Object Name) {
    if($ccFile.Name -ne 'notice-manifest.json'){$ccReceipt.files+=@{file=$ccFile.Name;sha256=(Get-FileHash -LiteralPath $ccFile.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}}
}
[IO.File]::WriteAllText((Join-Path $ccDestination 'notice-manifest.json'),($ccReceipt | ConvertTo-Json -Depth 5)+"`n",[Text.UTF8Encoding]::new($false))
Write-Output 'Prepared pinned upstream notices; installer/runtime distribution review remains required before release.'
