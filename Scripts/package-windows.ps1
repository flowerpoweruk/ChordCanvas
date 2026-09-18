param(
    [string]$PluginBundle,
    [string]$InstallerHelper,
    [string]$BuildReceipt,
    [string]$Compiler = (Join-Path (Split-Path $PSScriptRoot -Parent) 'build/tools/InnoSetup7/ISCC.exe'),
    [string]$OutputDirectory,
    [Parameter(Mandatory=$true)][string]$InnoLicenseFile,
    [Parameter(Mandatory=$true)][string]$PayloadInspector,
    [switch]$DistributionRightsConfirmed
)
$ErrorActionPreference='Stop'
$ccRoot=Split-Path $PSScriptRoot -Parent
$ccRelease=Get-Content -LiteralPath (Join-Path $ccRoot 'release/current.json') -Raw | ConvertFrom-Json
if($ccRelease.version -notmatch '^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$'){throw 'Only numeric release SemVer is supported'}
$ccParts=@($ccRelease.version.Split('.') | ForEach-Object { [uint32]$_ })
if(@($ccParts | Where-Object {$_ -gt 65535}).Count -or $ccRelease.buildNumber -lt 0 -or $ccRelease.buildNumber -gt 65535){throw 'Release version is outside Windows numeric bounds'}
if($ccRelease.installerId -cne '{EE56A5B5-24E6-4F83-B3AF-1C73BCD93594}' -or $ccRelease.manufacturerCode -cne 'ChCv' -or $ccRelease.pluginCode -cne 'Cc01'){throw 'Stable product identity mismatch'}
$ccFileVersion=$ccRelease.version+'.'+$ccRelease.buildNumber
if(!$PluginBundle){$PluginBundle=Join-Path $ccRoot 'build/plugin-msvc/Source/Plugin/ChordCanvas_artefacts/Release/VST3/ChordCanvas.vst3'}
if(!$InstallerHelper){$InstallerHelper=Join-Path $ccRoot 'build/plugin-msvc/Release/ChordCanvasInstaller.dll'}
if(!$BuildReceipt){$BuildReceipt=Join-Path $ccRoot 'build/plugin-msvc/build-inputs.json'}
function AssertPlain([string]$Path) {
    $ccFull=[IO.Path]::GetFullPath($Path)
    while($ccFull){
        if(Test-Path -LiteralPath $ccFull){
            $ccItem=Get-Item -LiteralPath $ccFull -Force
            if($ccItem.Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'Reparse-point package input/output refused'}
        }
        $ccFull=[IO.Path]::GetDirectoryName($ccFull)
    }
}
function AssertBinary([string]$Path) {
    AssertPlain $Path
    $ccStream=[IO.File]::OpenRead($Path)
    try {
        $ccReader=[IO.BinaryReader]::new($ccStream)
        if($ccReader.ReadUInt16() -ne 0x5a4d){throw 'Package binary is not PE'}
        $ccStream.Position=60;$ccOffset=$ccReader.ReadUInt32()
        if($ccOffset -lt 64 -or $ccOffset -gt 1048576 -or $ccOffset+26 -gt $ccStream.Length){throw 'Invalid PE header'}
        $ccStream.Position=$ccOffset
        if($ccReader.ReadUInt32() -ne 0x4550 -or $ccReader.ReadUInt16() -ne 0x8664){throw 'Package requires AMD64 PE'}
        $ccStream.Position=$ccOffset+22
        if(!($ccReader.ReadUInt16() -band 0x2000) -or $ccReader.ReadUInt16() -ne 0x20b){throw 'Package requires PE32+ DLL'}
    } finally {$ccStream.Dispose()}
    $ccInfo=[Diagnostics.FileVersionInfo]::GetVersionInfo([IO.Path]::GetFullPath($Path))
    $ccActual='{0}.{1}.{2}.{3}' -f $ccInfo.FileMajorPart,$ccInfo.FileMinorPart,$ccInfo.FileBuildPart,$ccInfo.FilePrivatePart
    if($ccActual -cne $ccFileVersion -or $ccInfo.ProductVersion -cne $ccRelease.version){throw "Stale or inconsistent binary version: expected $ccFileVersion / $($ccRelease.version), observed $ccActual / $($ccInfo.ProductVersion)"}
}
# Fail stale builds before copying or creating any package output.
AssertPlain $PluginBundle
AssertBinary (Join-Path $PluginBundle 'Contents/x86_64-win/ChordCanvas.vst3')
AssertBinary $InstallerHelper
foreach($ccInput in @($Compiler,$PayloadInspector,$InnoLicenseFile)){AssertPlain $ccInput;if(!(Test-Path -LiteralPath $ccInput -PathType Leaf)){throw 'Missing package tool/legal input'}}
if((Get-FileHash -LiteralPath $InnoLicenseFile -Algorithm SHA256).Hash.ToLowerInvariant() -cne '2e5346868c2a18434489824e11d65c3031620f792fefc415d05f19cd441abf5c'){throw 'Expected the unmodified installed Inno Setup 7.1.0 licence'}
$ccCompilerHash=(Get-FileHash -LiteralPath $Compiler -Algorithm SHA256).Hash.ToLowerInvariant()
if($ccCompilerHash -cne 'd06ebd38f38e3cee60a3c50cc45bd449d77e0bc6a5cabc607ea9886808e4de1a'){throw 'Expected the verified pinned Inno Setup 7.1.0 x64 compiler'}
if(!$DistributionRightsConfirmed){throw 'Confirm actual compiler/runtime/dependency distribution clearance before creating product packages'}
$ccStatus=& git -C $ccRoot status --porcelain --untracked-files=normal
if($LASTEXITCODE -or $ccStatus){throw 'Product package inputs require a clean committed source tree'}
$ccCommit=(& git -C $ccRoot rev-parse HEAD).Trim()
if($LASTEXITCODE -or $ccCommit -notmatch '^[0-9a-f]{40}$'){throw 'Cannot establish source provenance'}
AssertPlain $BuildReceipt
$ccBuild=Get-Content -LiteralPath $BuildReceipt -Raw | ConvertFrom-Json
if($ccBuild.sourceCommit -cne $ccCommit -or $ccBuild.sourceDirty -isnot [bool] -or $ccBuild.sourceDirty -or $ccBuild.version -cne $ccRelease.version -or $ccBuild.buildNumber -ne $ccRelease.buildNumber -or $ccBuild.configuration -cne 'Release' -or $ccBuild.tests -cne 'PASS' -or $ccBuild.juceRevision -cne '72782788ce18c2d4d760b28e0921d6ffc6431102'){throw 'Build receipt does not prove these committed Release inputs'}
foreach($ccPair in @(@((Join-Path $PluginBundle 'Contents/x86_64-win/ChordCanvas.vst3'),$ccBuild.pluginSha256),@((Join-Path $PluginBundle 'Contents/Resources/moduleinfo.json'),$ccBuild.moduleInfoSha256),@($InstallerHelper,$ccBuild.helperSha256),@((Join-Path $ccRoot 'release/current.json'),$ccBuild.releaseMetadataSha256))){
    if((Get-FileHash -LiteralPath $ccPair[0] -Algorithm SHA256).Hash.ToLowerInvariant() -cne $ccPair[1]){throw 'Build receipt input hash mismatch'}
}
$ccModule=Get-Content -LiteralPath (Join-Path $PluginBundle 'Contents/Resources/moduleinfo.json') -Raw | ConvertFrom-Json
if($ccModule.Name -cne 'ChordCanvas' -or $ccModule.Version -cne $ccRelease.version -or $ccModule.'Factory Info'.Vendor -cne 'ChordCanvas'){throw 'VST3 SDK metadata does not match release'}
$ccIds=@($ccModule.Classes | ForEach-Object {$_.CID})
if($ccIds.Count -ne 2 -or $ccIds -cnotcontains 'ABCDEF019182FAEB4368437643633031' -or $ccIds -cnotcontains 'ABCDEF011234ABCD4368437643633031'){throw 'Stable VST3 class IDs differ'}
$ccInstrument=@($ccModule.Classes | Where-Object {$_.CID -ceq 'ABCDEF019182FAEB4368437643633031'})
if($ccInstrument.Count -ne 1 -or $ccInstrument[0].Category -cne 'Audio Module Class' -or $ccInstrument[0].'Sub Categories' -cnotcontains 'Instrument'){throw 'VST3 instrument category missing'}
if(!$OutputDirectory){$OutputDirectory=Join-Path $ccRoot ('dist/'+$ccRelease.version+'/'+$ccCommit)}
$ccOutput=[IO.Path]::GetFullPath($OutputDirectory)
$ccDist=[IO.Path]::GetFullPath((Join-Path $ccRoot 'dist'))+[IO.Path]::DirectorySeparatorChar
if(!$ccOutput.StartsWith($ccDist,[StringComparison]::OrdinalIgnoreCase)){throw 'Package output must be inside project dist'}
AssertPlain $ccOutput
if(Test-Path -LiteralPath $ccOutput){throw 'Existing package output is immutable; choose a new empty output folder'}
[IO.Directory]::CreateDirectory($ccOutput) | Out-Null
$ccPayload=Join-Path $ccOutput 'inputs/ChordCanvas.vst3'
[IO.Directory]::CreateDirectory($ccPayload) | Out-Null
$ccAllowed=@('Contents/x86_64-win/ChordCanvas.vst3','Contents/Resources/moduleinfo.json')
$ccSourceFiles=@(Get-ChildItem -LiteralPath $PluginBundle -Recurse -Force -File)
foreach($ccSource in $ccSourceFiles){
    AssertPlain $ccSource.FullName
    $ccRel=[IO.Path]::GetRelativePath([IO.Path]::GetFullPath($PluginBundle),$ccSource.FullName).Replace('\','/')
    if($ccAllowed -cnotcontains $ccRel){throw 'Unreviewed file in VST3 build output; packaging stopped'}
    $ccTarget=Join-Path $ccPayload $ccRel
    [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($ccTarget)) | Out-Null
    Copy-Item -LiteralPath $ccSource.FullName -Destination $ccTarget
}
$ccLegal=Join-Path $ccPayload 'Contents/Resources/Licenses'
& (Join-Path $PSScriptRoot 'prepare-notices.ps1') -OutputDirectory $ccLegal
Copy-Item -LiteralPath $InnoLicenseFile -Destination (Join-Path $ccLegal 'Inno-Setup.txt')
Copy-Item -LiteralPath $InstallerHelper -Destination (Join-Path $ccOutput 'inputs/ChordCanvasInstaller.dll')
Copy-Item -LiteralPath (Join-Path $ccRoot 'release/current.json') -Destination (Join-Path $ccPayload 'Contents/Resources/release.json')
$ccUtf8=[Text.UTF8Encoding]::new($false)
$ccEntries=@(Get-ChildItem -LiteralPath $ccPayload -Recurse -File | ForEach-Object {
    $ccName=[IO.Path]::GetRelativePath($ccPayload,$_.FullName).Replace('\','/')
    if($ccName.Length -gt 240 -or $ccName -match '[^\x20-\x7e]'){throw 'Payload v1 requires bounded ASCII paths'}
    [pscustomobject]@{relative=$ccName;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()}
} | Sort-Object relative)
$ccReceipt=@('ChordCanvasPayload1','EE56A5B5-24E6-4F83-B3AF-1C73BCD93594',$ccRelease.version,'Contents/x86_64-win/ChordCanvas.vst3')
$ccReceipt+=@($ccEntries | ForEach-Object {$_.sha256+"`t"+$_.relative})
[IO.File]::WriteAllText((Join-Path $ccPayload 'chordcanvas.payload'),($ccReceipt -join "`n")+"`n",$ccUtf8)
& $PayloadInspector $ccPayload
if($LASTEXITCODE){throw 'Native complete payload validation failed'}
function QuoteInno([string]$Value){return '"'+$Value.Replace('"','""')+'"'}
$ccDefinitions=@('[Files]',('Source: '+(QuoteInno (Join-Path $ccOutput 'inputs/ChordCanvasInstaller.dll'))+'; Flags: dontcopy noencryption'))
$ccAll=@($ccEntries.relative)+@('chordcanvas.payload')
$ccDirs=[Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach($ccRel in $ccAll){
    $ccParent=[IO.Path]::GetDirectoryName($ccRel.Replace('/','\'))
    $ccTmp='{tmp}\Payload\ChordCanvas.vst3'+$(if($ccParent){'\'+$ccParent}else{''})
    $ccDefinitions+=('Source: '+(QuoteInno (Join-Path $ccPayload $ccRel))+'; DestDir: '+(QuoteInno $ccTmp)+'; Flags: dontcopy noencryption')
    while($ccParent){[void]$ccDirs.Add($ccParent);$ccParent=[IO.Path]::GetDirectoryName($ccParent)}
}
$ccDefinitions+=@('','[UninstallDelete]')
foreach($ccRel in $ccAll){$ccDefinitions+=('Type: files; Name: '+(QuoteInno ('{commoncf64}\VST3\ChordCanvas.vst3\'+$ccRel.Replace('/','\'))))}
foreach($ccDir in @($ccDirs | Sort-Object {$_.Split('\').Count} -Descending)){$ccDefinitions+=('Type: dirifempty; Name: '+(QuoteInno ('{commoncf64}\VST3\ChordCanvas.vst3\'+$ccDir)))}
$ccDefinitions+=@('Type: dirifempty; Name: "{commoncf64}\VST3\ChordCanvas.vst3"','Type: files; Name: "{app}\ChordCanvasInstaller.dll"','Type: files; Name: "{app}\chordcanvas.install"','Type: dirifempty; Name: "{app}"')
$ccInclude=Join-Path $ccOutput 'inputs/payload.iss'
[IO.File]::WriteAllText($ccInclude,($ccDefinitions -join "`n")+"`n",$ccUtf8)
foreach($ccMode in @('Setup','Updater')){
    & $Compiler ('/DCC_Version='+$ccRelease.version) ('/DCC_FileVersion='+$ccFileVersion) ('/DCC_Inputs='+$ccInclude) ('/DCC_OutputDir='+$ccOutput) (Join-Path $ccRoot ('Packaging/Windows/'+$ccMode+'.iss'))
    if($LASTEXITCODE){throw "$ccMode compilation failed; no accepted package is claimed"}
}
$ccArtifacts=@(foreach($ccName in @('Setup.exe','Updater.exe')){
    $ccFile=Get-Item -LiteralPath (Join-Path $ccOutput $ccName)
    [ordered]@{file=$ccName;sha256=(Get-FileHash -LiteralPath $ccFile.FullName -Algorithm SHA256).Hash.ToLowerInvariant();bytes=$ccFile.Length;signature=(Get-AuthenticodeSignature -LiteralPath $ccFile.FullName).Status.ToString()}
})
$ccManifest=[ordered]@{product='ChordCanvas';version=$ccRelease.version;fileVersion=$ccFileVersion;channel=$ccRelease.channel;sourceCommit=$ccCommit;installerId=$ccRelease.installerId;status='UNACCEPTED CANDIDATE';hostTests='NOT RUN against these packages';packageTests='NOT RUN';compilerVersion='Inno Setup 7.1.0 x64';compilerSha256=$ccCompilerHash;payload=$ccEntries;installerHelperSha256=(Get-FileHash -LiteralPath $InstallerHelper).Hash.ToLowerInvariant();innoLicenseSha256=(Get-FileHash -LiteralPath $InnoLicenseFile).Hash.ToLowerInvariant();artifacts=$ccArtifacts}
[IO.File]::WriteAllText((Join-Path $ccOutput 'release-manifest.json'),($ccManifest | ConvertTo-Json -Depth 8)+"`n",$ccUtf8)
[IO.File]::WriteAllText((Join-Path $ccOutput 'SHA256SUMS.txt'),(@($ccArtifacts | ForEach-Object {$_.sha256+'  '+$_.file}) -join "`n")+"`n",$ccUtf8)
Write-Output 'Compiled both offline package candidates. Installed host/package acceptance is still required.'
