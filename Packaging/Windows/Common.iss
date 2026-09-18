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

#include "TransactionEvents.iss"
