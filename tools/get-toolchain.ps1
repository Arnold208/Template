# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.
 
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12;
 
Write-Host "`nChecking for installed packages..."
 
# Paths and file names
$gccarm_path = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/'
$gccarm_file = 'gcc-arm-none-eabi-10.3-2021.10-win32.exe'
$gccarm_url = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-win32.exe?rev=29bb46cfa0434fbda93abb33c1d480e6&hash=B2C5AAE07841929A0D0BF460896D6E52'
$gccarm_name = 'GNU Arm Embedded Toolchain'
 
$cmake_path = 'https://github.com/Kitware/CMake/releases/download/v3.31.5/'
$cmake_file = 'cmake-3.31.5-windows-i386.msi'
$cmake_url = 'https://github.com/Kitware/CMake/releases/download/v3.31.5/cmake-3.31.5-windows-i386.msi'
$cmake_name = 'CMake v3.31.5'
 
# Check if GNU Arm Toolchain and CMake are installed
$gccarm_installed = Test-Path "C:\Program Files (x86)\GNU Arm Embedded Toolchain"
$cmake_installed = Get-Command cmake -ErrorAction SilentlyContinue
 
if ($gccarm_installed -and $cmake_installed) {
    Write-Host "All required packages are already installed."
    exit 0
}
 
Write-Host "`nDownloading required packages..."
 
$wc = New-Object System.Net.WebClient
$wc.Headers['User-Agent'] = "Mozilla/4.0"
 
# Function to download a file with retry logic
function Download-File {
    param (
        [string]$url,
        [string]$destination
    )
 
    $maxRetries = 3
    $retry = 0
    $success = $false
 
    while ($retry -lt $maxRetries -and -not $success) {
        try {
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
 
# Download and install GNU Arm Toolchain if not installed
if (-not $gccarm_installed) {
    $localPath = "$env:TEMP\$gccarm_file"
    Write-Host "(1/2) Downloading $gccarm_name..."
    Download-File $gccarm_url $localPath
 
    Write-Host "Installing $gccarm_name..."
    Start-Process -FilePath $localPath -ArgumentList "/S" -Wait
}
 
# Download and install CMake if not installed
if (-not $cmake_installed) {
    $localPath = "$env:TEMP\$cmake_file"
    Write-Host "(2/2) Downloading $cmake_name..."
    Download-File $cmake_url $localPath
 
    Write-Host "Installing $cmake_name..."
    Start-Process -FilePath $localPath -ArgumentList "ADD_CMAKE_TO_PATH=System /passive" -Wait
}
 
Write-Host "`nInstallation complete."
