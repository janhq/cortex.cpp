FROM mcr.microsoft.com/windows/server:ltsc2022

SHELL ["powershell", "-Command", "$ErrorActionPreference = 'Stop';$ProgressPreference='silentlyContinue';"]

ARG RUNNER_VERSION=2.298.2

RUN Invoke-WebRequest \
      -Uri 'https://aka.ms/install-powershell.ps1' \
      -OutFile install-powershell.ps1; \
    powershell -ExecutionPolicy Unrestricted -File ./install-powershell.ps1 -AddToPath

RUN Invoke-WebRequest \
      -Uri https://github.com/actions/runner/releases/download/v$env:RUNNER_VERSION/actions-runner-win-x64-$env:RUNNER_VERSION.zip \
      -OutFile runner.zip; \
    Expand-Archive -Path C:/runner.zip -DestinationPath C:/actions-runner; \
    Remove-Item -Path C:\runner.zip; \
    setx /M PATH $(${Env:PATH} + \";${Env:ProgramFiles}\Git\bin\")

# Install Chocolatey
RUN Set-ExecutionPolicy Bypass -Scope Process -Force; \
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; \
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Upgrade Git
RUN choco install git -y; \
    git --version

# Install 7zip
RUN choco install 7zip -y; \
    7z --help

# Install cmake and add to path
RUN choco install cmake.install -y --installargs '"ADD_CMAKE_TO_PATH=System"'

RUN cmake --version

# Install MSBuild and add to path
RUN choco install visualstudio2022buildtools -y --package-parameters '"--add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.Windows10SDK.19041 --add Microsoft.VisualStudio.Component.VC.14.29.CLI.Support --add Microsoft.VisualStudio.Component.VC.14.29.CMake.Project --add Microsoft.VisualStudio.Component.VC.14.29.MFC --add Microsoft.VisualStudio.Component.VC.14.29.MSBuild --add Microsoft.VisualStudio.Component.VC.14.29.VCRedist.x86.x64 --add Microsoft.VisualStudio.Component.VC.14.29.VSTools.x86.x64 --add Microsoft.VisualStudio.Component.VC.14.29.VC.ASAN --add Microsoft.VisualStudio.Component.VC.14.29.VC.CLI.Support --add Microsoft.VisualStudio.Component.VC.14.29.VC.CMake.Project --add Microsoft.VisualStudio.Component.VC.14.29.VC.FxCop --add Microsoft.VisualStudio.Component.VC.14.29.VC.MFC --add Microsoft.VisualStudio.Component.VC.14.29.VC.MSBuild --add Microsoft.VisualStudio.Component.VC.14.29.VC.Redist.14.Latest --add Microsoft.VisualStudio.Component.VC.14.29.VC.Tools.x86.x64 --add Microsoft.VisualStudio.Component.VC.14.29.VC.VCUnitTest --add Microsoft.VisualStudio.Component.VC.14.29.VC.x86.x64 --add Microsoft.VisualStudio.Component.VC.14.29.VC.x86.x64.Latest --add Microsoft.VisualStudio.Component.VC.14.29.VisualStudioCppSDK --add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Win81 --add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Win10 --add Microsoft.VisualStudio.Component.Windows10SDK.19041 --add Microsoft.VisualStudio.ComponentGroup.NativeDesktop.LegacyBuildTools --add Microsoft.VisualStudio.Component.VC.ATL --add Microsoft.VisualStudio.Component.VC.CLI.Support --add Microsoft.VisualStudio.Component.VC.CoreIde --add Microsoft.VisualStudio.Component.VC.DiagnosticTools --add Microsoft.VisualStudio.Component.VC.Llvm.Clang --add Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset --add Microsoft.VisualStudio.Component.VC.Llvm.Cmake --add Microsoft.VisualStudio.Component.VC.Llvm.Llvm --add Microsoft.VisualStudio.Component.VC.Llvm.Toolset --add Microsoft.VisualStudio.Component.VC.MFC --add Microsoft.VisualStudio.Component.VC.MSBuild --add Microsoft.VisualStudio"'

RUN choco install gzip -y;

# Install cuda toolkit 12.0.4
RUN choco install cuda --version=11.4.2.47141 -y

# Copy integrated tools to MSBuild
RUN Copy-Item -Path 'C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.4\extras\visual_studio_integration\MSBuildExtensions\*' -Destination 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Microsoft\VC\v170\BuildCustomizations'
RUN Copy-Item -Path 'C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.4\extras\visual_studio_integration\MSBuildExtensions\*' -Destination 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Microsoft\VC\v160\BuildCustomizations'


ADD runner.ps1 C:/runner.ps1
CMD ["pwsh", "-ExecutionPolicy", "Unrestricted", "-File", ".\\runner.ps1"]