# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12;

Write-Host "`nChecking for installed packages..."

# Detect repository root (one level above script directory)
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$repoRoot = Split-Path -Parent $scriptDir
$submodule_threadx = "$repoRoot\shared\lib\threadx"
$submodule_netxduo = "$repoRoot\shared\lib\netxduo"

# Paths, file names, and URLs
$gccarm_14_path = 'https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/'
$gccarm_14_file = 'arm-gnu-toolchain-14.2.rel1-mingw-w64-i686-arm-none-eabi.exe'
$gccarm_14_url = 'https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-mingw-w64-i686-arm-none-eabi.exe'
$gccarm_14_name = 'GNU Arm Embedded Toolchain 14.2'
$gccarm_14_install_path = "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin"

$gccarm_10_path = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/'
$gccarm_10_file = 'gcc-arm-none-eabi-10.3-2021.10-win32.exe'
$gccarm_10_url = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-win32.exe'
$gccarm_10_name = 'GNU Arm Embedded Toolchain 10.3'

$cmake_path = 'https://github.com/Kitware/CMake/releases/download/v3.21.4'
$cmake_file = 'cmake-3.21.4-windows-i386.msi'
$cmake_url = 'https://github.com/Kitware/CMake/releases/download/v3.21.4/cmake-3.21.4-windows-i386.msi'
$cmake_name = 'CMake v3.21.4'
$cmake_install_path = "C:\Program Files (x86)\CMake\bin"

$ninja_path = 'https://github.com/ninja-build/ninja/releases/download/v1.12.1/'
$ninja_file = 'ninja-win.zip'
$ninja_url = 'https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip'
$ninja_name = 'Ninja Build System'
$ninja_install_path = "$env:USERPROFILE\Documents\Ninja"

# Ensure the WebClient object is available
$wc = New-Object System.Net.WebClient
$wc.Headers['User-Agent'] = "Mozilla/4.0"

# Function to check if a package is installed
function Is-Installed {
    param ($path)
    return Test-Path $path
}

# Function to download a file with retry logic
function Download-File {
    param ($url, $destination)

    $maxRetries = 3
    $retry = 0
    $success = $false

    while ($retry -lt $maxRetries -and -not $success) {
        try {
            if (Test-Path $destination) {
                Write-Host "File already downloaded: $destination"
                return
            }

            Write-Host "Downloading $destination..."
            $wc.DownloadFile($url, $destination)
            $success = $true
        } catch {
            Write-Host "Failed to download $destination. Retrying... ($($retry + 1)/$maxRetries)"
            Start-Sleep -Seconds 5
        }
        $retry++
    }

    if (-not $success) {
        throw "Failed to download $destination after $maxRetries attempts."
    }
}

# Define installation paths
$gccarm_14_installed = Is-Installed $gccarm_14_install_path
$gccarm_10_installed = Is-Installed "C:\Program Files (x86)\GNU Arm Embedded Toolchain"
$cmake_installed = Is-Installed $cmake_install_path
$ninja_installed = Is-Installed "$ninja_install_path\ninja.exe"

if ($gccarm_14_installed -and $gccarm_10_installed -and $cmake_installed -and $ninja_installed) {
    Write-Host "All required packages are already installed."
} else {
    Write-Host "`nDownloading required packages..."

    # Install GNU Arm Toolchain 14.2 and add to PATH
    if (-not $gccarm_14_installed) {
        $localPath = "$env:TEMP\$gccarm_14_file"
        Write-Host "(1/3) Downloading $gccarm_14_name..."
        Download-File $gccarm_14_url $localPath
        Write-Host "Installing $gccarm_14_name..."
        Start-Process -FilePath $localPath -ArgumentList "/S" -Wait
    }

    # Install GNU Arm Toolchain 10.3 (DO NOT ADD TO PATH)
    if (-not $gccarm_10_installed) {
        $localPath = "$env:TEMP\$gccarm_10_file"
        Write-Host "(2/3) Downloading $gccarm_10_name..."
        Download-File $gccarm_10_url $localPath
        Write-Host "Installing $gccarm_10_name..."
        Start-Process -FilePath $localPath -ArgumentList "/S" -Wait
    }

    # Install Ninja in User Documents and add to PATH
    if (-not $ninja_installed) {
        $localPath = "$env:TEMP\$ninja_file"
        Write-Host "(3/3) Downloading $ninja_name..."
        Download-File $ninja_url $localPath
        Write-Host "Extracting $ninja_name..."
        Expand-Archive -Path $localPath -DestinationPath $ninja_install_path -Force
    }

    Write-Host "`nInstallation complete!"
}

# Function to securely add directories to PATH
function Add-ToSystemPath {
    param ($NewPath)
    $currentPath = [System.Environment]::GetEnvironmentVariable("Path", [System.EnvironmentVariableTarget]::Machine)

    if ($currentPath -notlike "*$NewPath*") {
        Write-Host "Adding $NewPath to system PATH..."
        Start-Process powershell -ArgumentList "-NoProfile -ExecutionPolicy Bypass -Command `"Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Environment' -Name Path -Value '$currentPath;$NewPath'`"" -Verb RunAs
    } else {
        Write-Host "$NewPath is already in the system PATH."
    }
}

# Add required paths to the environment
$pathsToAdd = @(
    "C:\Program Files (x86)\Arm GNU Toolchain arm-none-eabi\14.2 rel1\bin",
    "C:\Program Files (x86)\CMake\bin",
    "$ninja_install_path"
)

foreach ($path in $pathsToAdd) {
    if (Is-Installed $path) {
        Add-ToSystemPath -NewPath $path
    }
}

# Move to the root directory before initializing Git submodules
Set-Location $repoRoot

# Initialize and update Git submodules
Write-Host "Initializing and updating Git submodules..."
git submodule init
git submodule update --recursive

# Verify submodules exist
if ((Test-Path $submodule_threadx) -and (Test-Path $submodule_netxduo)) {
    Write-Host "Submodules verified: threadx and netxduo are present."
} else {
    Write-Host "Error: Missing submodules. Please check your repository setup."
    exit 1
}

Write-Host "Setup complete! Restart your system for environment changes to take effect."
