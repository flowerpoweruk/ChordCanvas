; Disposable per-user integration fixture: not a product installer.
; The script events and production transaction/metadata code are shared.
[Setup]
AppId={{{#CC_FixtureId}}
AppName=ChordCanvas
AppVersion={#CC_Version}
AppPublisher=ChordCanvas
UninstallDisplayName=ChordCanvas
DefaultDirName={#CC_FixtureRoot}\ChordCanvas
UninstallFilesDir={app}
SetupArchitecture=x64
ArchitecturesAllowed=x64os
ArchitecturesInstallIn64BitMode=x64os
MinVersion=10.0.22000
PrivilegesRequired=lowest
UsePreviousAppDir=no
DisableWelcomePage=yes
DisableDirPage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DisableFinishedPage=yes
CreateAppDir=yes
AllowNoIcons=yes
CloseApplications=no
RestartApplications=no
UninstallLogging=no
OutputBaseFilename={#CC_Output}
OutputDir={#CC_OutputDir}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

#include CC_Inputs
#include "../../Packaging/Windows/TransactionEvents.iss"
