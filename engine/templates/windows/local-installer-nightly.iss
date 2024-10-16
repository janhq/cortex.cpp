; Define the application name, version, and other details
[Setup]
AppName=cortexcpp-nightly
AppVersion=1.0
DefaultDirName={localappdata}\cortexcpp-nightly
DefaultGroupName=cortexcpp-nightly
OutputDir=.
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=lowest
AllowNoIcons=yes
DisableDirPage=yes

; Define the languages section
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

; Define the files to be installed
[Files]
Source: "cortex-nightly.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "cortex-server-nightly.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "dependencies\*"; DestDir: "{tmp}\dependencies"; Flags: ignoreversion recursesubdirs

; Define the icons to be created
[Icons]
Name: "{group}\cortexcpp-nightly"; Filename: "{app}\cortex-nightly.exe"

; Define the uninstall run section to execute commands before uninstallation
[UninstallRun]
Filename: "{app}\cortex-nightly.exe"; Parameters: "stop"; StatusMsg: "Stopping cortexcpp-nightly service..."; Flags: runhidden

; Combine all Pascal scripting code in one [Code] section
[Code]
procedure AddToUserPathAndInstallEngines;
var
  ExpandedAppDir: String;
  TempFolder: String;
  CmdLine, CortexInstallCmd: String;
  ResultCode: Integer;
begin
  ExpandedAppDir := ExpandConstant('{app}');
  TempFolder := ExpandConstant('{tmp}\dependencies');  // Path to the extracted dependencies folder

  // Add {app} to PATH
  CmdLine := Format('setx PATH "%s;%%PATH%%"', [ExpandedAppDir]);
  Exec('cmd.exe', '/C ' + CmdLine, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  // Check if the parameter /SkipPostInstall is passed
  if ParamStr(1) = '/SkipPostInstall' then
  begin
    Log('Skipping post-install actions.');
    Exit;  // Exit the procedure without doing anything
  end;

  // Update status message for installing engines
  WizardForm.StatusLabel.Caption := 'Installing engines from offline package...';
  WizardForm.StatusLabel.Update;

  // Run the installation command with the -s option pointing to TempFolder
  CortexInstallCmd := Format('"%s\cortex-nightly.exe" engines install llama-cpp -s "%s"', [ExpandedAppDir, TempFolder]);
  Exec('powershell.exe', '-Command ' + CortexInstallCmd, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  // Clear the status message after completion
  WizardForm.StatusLabel.Caption := '';
  WizardForm.StatusLabel.Update;

  // Remove the temp folder after the installation is complete
  DelTree(ExpandConstant('{tmp}\dependencies'), True, True, True);
end;

procedure DeleteCurrentUserCortexFolderAndConfig;
var
  UserCortexFolder: String;
  UserCortexConfig: String;
  ShouldDelete: Integer;
begin
  UserCortexFolder := ExpandConstant('{%USERPROFILE}\cortexcpp-nightly');
  UserCortexConfig := ExpandConstant('{%USERPROFILE}\.cortexrc-nightly');
  
  if DirExists(UserCortexFolder) or FileExists(UserCortexConfig) then
  begin
    ShouldDelete := MsgBox('Do you want to delete the application data in cortexcpp-nightly and the .cortexrc-nightly config file (this will remove all user data)?', mbConfirmation, MB_YESNO);
    
    if ShouldDelete = idYes then
    begin
      if DirExists(UserCortexFolder) then
      begin
        DelTree(UserCortexFolder, True, True, True);
      end;

      if FileExists(UserCortexConfig) then
      begin
        DeleteFile(UserCortexConfig);
      end;
    end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    AddToUserPathAndInstallEngines;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    DeleteCurrentUserCortexFolderAndConfig;
  end;
end;
