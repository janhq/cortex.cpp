; Inno Setup Script
; Define the application name, version, and other details
[Setup]
AppName=Cortexso
AppVersion=1.0
Publisher=Homebrew Computer Company Pte Ltd
DefaultDirName={pf}\Cortexso
DefaultGroupName=Cortexso
OutputDir=.
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=admin

; Define the languages section
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

; Define the files to be installed
[Files]
Source: "cortex.exe"; DestDir: "{app}"; Flags: ignoreversion

; Define the icons to be created
[Icons]
Name: "{group}\Cortexso"; Filename: "{app}\cortex.exe"

; Define the run section to execute the application after installation
[Run]
Filename: "cmd"; Parameters: "/c setx PATH ""%PATH%;{app}"""; StatusMsg: "Updating system PATH environment variable..."; Flags: runhidden
Filename: "{app}\cortex.exe"; Description: "{cm:LaunchProgram,Cortexso}"; Flags: nowait postinstall skipifsilent

; Define the tasks section (optional, for additional tasks like creating desktop icons)
[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; Flags: unchecked

; Define icons for the additional tasks
[Icons]
Name: "{commondesktop}\Cortexso"; Filename: "{app}\cortex.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Cortexso"; Filename: "{app}\cortex.exe"; Tasks: quicklaunchicon
