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

; Define the languages section
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

; Define the files to be installed
[Files]
Source: "cortex-nightly.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "cortex-server-nightly.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "vulkan-1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion

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
  CmdLine, CortexInstallCmd, CortexStopServerCmd: String;
  ResultCode: Integer;
  i: Integer;
  SkipPostInstall: Boolean;
begin
  SkipPostInstall := False;
  
  // Loop through all parameters to check for /SkipPostInstall
  for i := 1 to ParamCount do
  begin
    if ParamStr(i) = '/SkipPostInstall' then
    begin
      SkipPostInstall := True;
    end;
  end;

  ExpandedAppDir := ExpandConstant('{app}');

  // Set the maximum value for the progress bar to 100 (representing 100%)
  WizardForm.ProgressGauge.Max := 100;

  // Set the progress bar to 80%
  WizardForm.ProgressGauge.Position := 80;
  WizardForm.ProgressGauge.Update;
  
  // Add {app} to PATH
  CmdLine := Format('setx PATH "%s;%%PATH%%"', [ExpandedAppDir]);
  Exec('cmd.exe', '/C ' + CmdLine, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  // If the /SkipPostInstall flag is set, exit after setting the PATH
  if SkipPostInstall then
  begin
    Log('Skipping post-install actions after setting the PATH.');
    Exit;
  end;

  // Update status message for downloading llamacpp engine
  WizardForm.StatusLabel.Caption := 'Downloading llama.cpp engine and dependencies ...';
  WizardForm.StatusLabel.Update;

  // Set the progress bar to 85% after adding to PATH
  WizardForm.ProgressGauge.Position := 85;
  WizardForm.ProgressGauge.Update;

  // Download llamacpp engine by default
  CortexInstallCmd := Format('"%s\cortex-nightly.exe" engines install llama-cpp', [ExpandedAppDir]);
  Exec('cmd.exe', '/C ' + CortexInstallCmd, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  // Stop server
  CortexStopServerCmd := Format('"%s\cortex-nightly.exe" stop', [ExpandedAppDir]);
  Exec('cmd.exe', '/C ' + CortexStopServerCmd, '', SW_HIDE, ewWaitUntilTerminated, ResultCode);

  // Set the progress bar to 90% after downloading the engine
  WizardForm.ProgressGauge.Position := 90;
  WizardForm.ProgressGauge.Update;

  // Clear status message after completion
  WizardForm.StatusLabel.Caption := '';
  WizardForm.StatusLabel.Update;

  // Set the progress bar to 100% after completion
  WizardForm.ProgressGauge.Position := 100;
  WizardForm.ProgressGauge.Update;
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