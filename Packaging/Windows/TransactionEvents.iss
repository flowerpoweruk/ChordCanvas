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
  if not Result then SuppressibleMsgBox(ErrorText(Code), mbError, MB_OK, IDOK);
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
      SuppressibleMsgBox(ErrorText(Code), mbError, MB_OK, IDOK);
      Exit;
    end;
    Committed := True;
    TransactionStarted := False;
    if not WizardSilent then SuppressibleMsgBox('ChordCanvas installation completed.', mbInformation, MB_OK, IDOK);
  end;
end;

procedure DeinitializeSetup;
var Code: Integer;
begin
  if TransactionStarted and not Committed then begin
    Code := CC_Abort;
    if Code <> 0 then begin
      NativeFailure := 67;
      SuppressibleMsgBox(ErrorText(Code), mbError, MB_OK, IDOK);
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
  if UninstallMutex = 0 then begin SuppressibleMsgBox(ErrorText(-1), mbError, MB_OK, IDOK); Exit; end;
  Wait := WaitForSingleObject(UninstallMutex, 1000);
  if (Wait <> 0) and (Wait <> $80) then begin
    CloseHandle(UninstallMutex); UninstallMutex := 0;
    SuppressibleMsgBox('Another ChordCanvas installation is running. Retry when it finishes.', mbError, MB_OK, IDOK); Exit;
  end;
  try
    Code := CC_UninstallCheck(ExpandConstant('{app}'));
    UnloadDLL(ExpandConstant('{app}\ChordCanvasInstaller.dll'));
    Result := Code = 0;
    if not Result then SuppressibleMsgBox(ErrorText(Code), mbError, MB_OK, IDOK);
  except
    SuppressibleMsgBox(ErrorText(-1), mbError, MB_OK, IDOK);
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
