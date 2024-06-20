; Inno Setup Script
; Define the application name, version, and other details
[Setup]
AppName=Cortex
AppVersion=1.0
DefaultDirName={pf}\Cortex
DefaultGroupName=Cortex
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
Source: "node_modules\sqlite3\*"; DestDir: "{app}\node_modules\sqlite3"; Flags: ignoreversion recursesubdirs createallsubdirs

; Define the icons to be created
[Icons]
Name: "{group}\Cortex"; Filename: "{app}\cortex.exe"

; Define the run section to execute the application after installation
[Run]
Filename: "cmd"; Parameters: "/c setx PATH ""%PATH%;{app}"""; StatusMsg: "Updating system PATH environment variable..."; Flags: runhidden
Filename: "cmd"; Parameters: "/c cortex init"; StatusMsg: "Initializing Cortex..."; Flags: runhidden waituntilterminated
Filename: "{app}\cortex.exe"; Description: "{cm:LaunchProgram,Cortex}"; Flags: nowait postinstall skipifsilent

; Define the tasks section (optional, for additional tasks like creating desktop icons)
[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; Flags: unchecked

; Define icons for the additional tasks
[Icons]
Name: "{commondesktop}\Cortex"; Filename: "{app}\cortex.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Cortex"; Filename: "{app}\cortex.exe"; Tasks: quicklaunchicon
