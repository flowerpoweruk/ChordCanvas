; Generated CC_Inputs includes exact owned payload entries and uninstall paths.
; There are no downloads, driver installs, application closures or restart jobs.
#ifndef CC_Version
  #error CC_Version is required
#endif
#ifndef CC_FileVersion
  #error CC_FileVersion is required
#endif
#ifndef CC_Inputs
  #error CC_Inputs is required
#endif
#ifndef CC_OutputDir
  #error CC_OutputDir is required
#endif

[Setup]
AppId={{EE56A5B5-24E6-4F83-B3AF-1C73BCD93594}
AppName=ChordCanvas
AppVersion={#CC_Version}
AppPublisher=ChordCanvas
AppPublisherURL=https://github.com/flowerpoweruk/ChordCanvas
UninstallDisplayName=ChordCanvas
DefaultDirName={commonpf64}\ChordCanvas
UninstallFilesDir={app}
SetupArchitecture=x64
ArchitecturesAllowed=x64os
ArchitecturesInstallIn64BitMode=x64os
MinVersion=10.0.22000
PrivilegesRequired=admin
UsePreviousAppDir=no
UsePreviousGroup=no
UsePreviousTasks=no
DisableWelcomePage=yes
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DisableFinishedPage=yes
CreateAppDir=yes
AllowNoIcons=yes
CloseApplications=no
RestartApplications=no
AlwaysRestart=no
UninstallLogging=no
OutputBaseFilename={#CC_Output}
OutputDir={#CC_OutputDir}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
VersionInfoVersion={#CC_FileVersion}
VersionInfoProductVersion={#CC_Version}
VersionInfoDescription=ChordCanvas {#CC_Output}

#include CC_Inputs

[Code]
var
  TransactionStarted, Committed: Boolean;
  NativeFailure: Integer;
  UninstallMutex: THandle;

function CC_Supported: Integer;
  external 'CC_Supported@files:ChordCanvasInstaller.dll stdcall setuponly';
function CC_Begin(Source, AppDirectory: String; Update: Integer): Integer;
  external 'CC_Begin@files:ChordCanvasInstaller.dll stdcall setuponly';
function CC_Finalise: Integer;
  external 'CC_Finalise@files:ChordCanvasInstaller.dll stdcall setuponly';
function CC_Abort: Integer;
  external 'CC_Abort@files:ChordCanvasInstaller.dll stdcall setuponly';
function CC_UninstallCheck(AppDirectory: String): Integer;
  external 'CC_UninstallCheck@{app}\ChordCanvasInstaller.dll stdcall uninstallonly delayload';
function CreateMutex(Security: LongWord; Owner: Boolean; Name: String): THandle;
  external 'CreateMutexW@kernel32.dll stdcall';
function WaitForSingleObject(Handle: THandle; Milliseconds: LongWord): LongWord;
  external 'WaitForSingleObject@kernel32.dll stdcall';
function ReleaseMutex(Handle: THandle): Boolean;
  external 'ReleaseMutex@kernel32.dll stdcall';
function CloseHandle(Handle: THandle): Boolean;
  external 'CloseHandle@kernel32.dll stdcall';

function ErrorText(Code: Integer): String;
begin
  case Code of
    -2: Result := 'ChordCanvas is not installed. Run Setup.exe first.';
    -3: Result := 'A newer ChordCanvas version is installed. This package cannot downgrade it.';
    -4: Result := 'ChordCanvas files are in use or inaccessible. Save and close Live, then retry.';
    -5: Result := 'ChordCanvas could not restore every installation file. Save and close Live, then run this package again to recover.';
    -10, -11: Result := 'ChordCanvas requires Windows 11 on native x64 hardware.';
    -12: Result := 'ChordCanvas uses its fixed Windows installation directory.';
    -13: Result := 'This package contains inconsistent release files. Obtain the complete package.';
  else
    Result := 'ChordCanvas could not proceed safely. Existing files and recovery information have been preserved.';
  end;
end;

function InitializeSetup: Boolean;
var Code: Integer;
begin
  Code := CC_Supported;
  Result := Code = 0;
  if not Result then MsgBox(ErrorText(Code), mbError, MB_OK);
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
var Code: Integer;
begin
  NeedsRestart := False;
  Result := '';
  if TransactionStarted then Exit;
  ExtractTemporaryFiles('{tmp}\Payload\*');
  Code := CC_Begin(ExpandConstant('{tmp}\Payload\ChordCanvas.vst3'), ExpandConstant('{app}'), {#CC_Update});
  if Code = 1 then begin
    Result := 'ChordCanvas is already up to date.';
    Exit;
  end;
  if Code <> 0 then begin Result := ErrorText(Code); Exit; end;
  TransactionStarted := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var Code: Integer;
begin
  if (CurStep = ssDone) and TransactionStarted then begin
    Code := CC_Finalise;
    if Code <> 0 then begin
      NativeFailure := 66;
      MsgBox(ErrorText(Code), mbError, MB_OK);
      Exit;
    end;
    Committed := True;
    TransactionStarted := False;
    if not WizardSilent then MsgBox('ChordCanvas installation completed.', mbInformation, MB_OK);
  end;
end;

procedure DeinitializeSetup;
var Code: Integer;
begin
  if TransactionStarted and not Committed then begin
    Code := CC_Abort;
    if Code <> 0 then begin
      NativeFailure := 67;
      MsgBox(ErrorText(Code), mbError, MB_OK);
    end;
  end;
end;

function GetCustomSetupExitCode: Integer;
begin
  Result := NativeFailure;
end;

function InitializeUninstall: Boolean;
var Code: Integer; Wait: LongWord;
begin
  Result := False;
  UninstallMutex := CreateMutex(0, False, 'Global\ChordCanvas.InstallBundle.EE56A5B5-24E6-4F83-B3AF-1C73BCD93594.v1');
  if UninstallMutex = 0 then begin MsgBox(ErrorText(-1), mbError, MB_OK); Exit; end;
  Wait := WaitForSingleObject(UninstallMutex, 1000);
  if (Wait <> 0) and (Wait <> $80) then begin
    CloseHandle(UninstallMutex); UninstallMutex := 0;
    MsgBox('Another ChordCanvas installation is running. Retry when it finishes.', mbError, MB_OK); Exit;
  end;
  try
    Code := CC_UninstallCheck(ExpandConstant('{app}'));
    UnloadDLL(ExpandConstant('{app}\ChordCanvasInstaller.dll'));
    Result := Code = 0;
    if not Result then MsgBox(ErrorText(Code), mbError, MB_OK);
  except
    MsgBox(ErrorText(-1), mbError, MB_OK);
    Result := False;
  end;
end;

procedure DeinitializeUninstall;
begin
  if UninstallMutex <> 0 then begin ReleaseMutex(UninstallMutex); CloseHandle(UninstallMutex); UninstallMutex := 0; end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var Code: Integer;
begin
  if CurUninstallStep = usUninstall then begin
    { Recheck after confirmation, immediately before Inno starts deletion. }
    Code := CC_UninstallCheck(ExpandConstant('{app}'));
    UnloadDLL(ExpandConstant('{app}\ChordCanvasInstaller.dll'));
    if Code <> 0 then RaiseException(ErrorText(Code));
  end;
end;
