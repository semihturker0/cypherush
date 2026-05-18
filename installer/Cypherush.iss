; Cypherush - Copyright (C) 2026 semihturker0. All Rights Reserved.
; Proprietary software. See LICENSE for terms.
;
; Inno Setup 6 script. Build via build_installer.ps1.

#define MyAppName "Cypherush"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "semihturker0"
#define MyAppURL "https://github.com/semihturker0/cypherush"
#define MyAppExeName "cypherush_client.exe"

[Setup]
AppId={{B7E4F2A1-9C3D-4E8B-A5F6-1D2C3B4A5E6F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}/issues
AppUpdatesURL={#MyAppURL}/releases
AppCopyright=Copyright (C) 2026 semihturker0
DefaultDirName={autopf}\Cypherush
DefaultGroupName=Cypherush
DisableProgramGroupPage=yes
LicenseFile=..\LICENSE
OutputDir=output
OutputBaseFilename=CypherushSetup-v1.0.0
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
; SetupIconFile expects an .ico file; the project ships an SVG only,
; which Inno Setup does not accept. Using the default Inno icon.
; SetupIconFile=..\client\resources\icons\cypherush.svg
UninstallDisplayIcon={app}\cypherush_client.exe

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "turkish"; MessagesFile: "compiler:Languages\Turkish.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 6.1

[Files]
Source: "staging\cypherush_client.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "staging\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "staging\platforms\*"; DestDir: "{app}\platforms"; Flags: ignoreversion recursesubdirs
Source: "staging\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs
Source: "staging\iconengines\*"; DestDir: "{app}\iconengines"; Flags: ignoreversion recursesubdirs
Source: "staging\styles\*"; DestDir: "{app}\styles"; Flags: ignoreversion recursesubdirs
Source: "staging\translations\*"; DestDir: "{app}\translations"; Flags: ignoreversion recursesubdirs skipifsourcedoesntexist
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\NOTICE.md"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\Cypherush"; Filename: "{app}\cypherush_client.exe"
Name: "{group}\{cm:UninstallProgram,Cypherush}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Cypherush"; Filename: "{app}\cypherush_client.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\cypherush_client.exe"; Description: "Launch Cypherush"; Flags: nowait postinstall skipifsilent
