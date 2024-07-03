# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12;

Write-Host "`nChecking for installed packages..."

# Paths, file names, and hash values for verification
$gccarm_path = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/'
$gccarm_file = 'gcc-arm-none-eabi-10.3-2021.10-win32.exe'
$gccarm_name = 'GNU Arm Embedded Toolchain'
$gccarm_hash = '3BEBF304C59F9CC9F544EE5ED38B27DF3019177B0548627C97F5F8BB02300395'

$cmake_path = 'https://github.com/Kitware/CMake/releases/download/v3.21.4'
$cmake_file = 'cmake-3.21.4-windows-i386.msi'
$cmake_name = 'CMake v3.21.4'
$cmake_hash = 'C769ECE7EC8E91529F4CC938F088B4699A1F5CD39B7F44158D1AA0AB6E76AB74'

$termite_path = 'https://www.compuphase.com/software'
$termite_file = 'termite-3.4.exe'
$termite_name = 'Termite v3.4'
$termite_hash = 'CA440B6C7F6EAA812BA5F8BF42AED86E02022CA50A1C72585168C9B671D0FE19'

# Check installations
$gccarm_installed = Test-Path "C:\Program Files (x86)\GNU Arm Embedded Toolchain"
$cmake_installed = Get-Command cmake -ErrorAction SilentlyContinue
$termite_installed = Test-Path "C:\Program Files (x86)\Termite"

if ($gccarm_installed -and $cmake_installed -and $termite_installed) {
    Write-Host "All required packages are already installed."
    exit 0
}

Write-Host "`nDownloading packages..."

$wc = New-Object System.Net.WebClient
$wc.Headers['User-Agent'] = "Mozilla/4.0"

# Download and install GCC-ARM if not installed
if (-not $gccarm_installed) {
    $localPath = "$env:TEMP\$gccarm_file"
    Write-Host "(1/3) Downloading $gccarm_name..."
    if (-not (Test-Path $localPath) -or ((Get-FileHash $localPath).Hash -ne $gccarm_hash)) {
        $wc.DownloadFile("$gccarm_path$gccarm_file", $localPath)
        Write-Host "Installing $gccarm_name..."
        Start-Process -FilePath $localPath -ArgumentList "/S" -Wait
    }
}

# Download and install CMake if not installed
if (-not $cmake_installed) {
    $localPath = "$env:TEMP\$cmake_file"
    Write-Host "(2/3) Downloading $cmake_name..."
    if (-not (Test-Path $localPath) -or ((Get-FileHash $localPath).Hash -ne $cmake_hash)) {
        $wc.DownloadFile("$cmake_path$cmake_file", $localPath)
        Write-Host "Installing $cmake_name..."
        Start-Process -FilePath $localPath -ArgumentList "ADD_CMAKE_TO_PATH=System /passive" -Wait
    }
}

# Download and install Termite if not installed
if (-not $termite_installed) {
    $localPath = "$env:TEMP\$termite_file"
    Write-Host "(3/3) Downloading $termite_name..."
    if (-not (Test-Path $localPath) -or ((Get-FileHash $localPath).Hash -ne $termite_hash)) {
        $wc.DownloadFile("$termite_path$termite_file", $localPath)
        Write-Host "Installing $termite_name..."
        Start-Process -FilePath $localPath -ArgumentList "/S" -Wait
    }
}

Write-Host "`nInstallation complete!"
