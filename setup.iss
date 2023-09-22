#define AppName "Homebrew Navigator"
#define AppVersion "0.0.1"
#define AppPublisher "dani@gatunes"
#define AppURL "https://navigator.gatunes.com/"
#define AppExeName "navigator"

[Setup]
AppId={{5599F2AF-8909-4D34-8F6E-86EC1EF20C93}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
DefaultDirName={autopf}\{#AppName}
DisableDirPage=auto
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
OutputBaseFilename={#AppExeName}-setup
OutputDir=build\Release

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "build\Release\{#AppExeName}.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}.exe"

[Run]
Filename: "{app}\{#AppExeName}.exe"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent
