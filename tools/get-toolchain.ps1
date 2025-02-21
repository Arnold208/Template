# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12;

Write-Host "`nChecking for installed packages..."

# Paths, file names, and URLs
$gccarm_path = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/'
$gccarm_file = 'gcc-arm-none-eabi-10.3-2021.10-win32.exe'
$gccarm_url = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-win32.exe'
$gccarm_name = 'GNU Arm Embedded Toolchain'
$gccarm_hash = '3BEBF304C59F9CC9F544EE5ED38B27DF3019177B0548627C97F5F8BB02300395'

$cmake_path = 'https://github.com/Kitware/CMake/releases/download/v3.21.4'
$cmake_file = 'cmake-3.21.4-windows-i386.msi'
$cmake_url = 'https://github.com/Kitware/CMake/releases/download/v3.21.4/cmake-3.21.4-windows-i386.msi'
$cmake_name = 'CMake v3.21.4'
$cmake_hash = 'C769ECE7EC8E91529F4CC938F088B4699A1F5CD39B7F44158D1AA0AB6E76AB74'

$termite_path = 'https://www.compuphase.com/software'
$termite_file = 'termite-3.4.exe'
$termite_url = 'https://www.compuphase.com/software/termite-3.4.exe'
$termite_name = 'Termite v3.4'
$termite_hash = 'CA440B6C7F6EAA812BA5F8BF42AED86E02022CA50A1C72585168C9B671D0FE19'

# Ensure the WebClient object is available
$wc = New-Object System.Net.WebClient
$wc.Headers['User-Agent'] = "Mozilla/4.0"

# Function to check if a package is installed
function Is-Installed {
    param ($path)
    return Test-Path $path
}

# Function to verify file hash
function Verify-FileHash {
    param ($filePath, $expectedHash)

    if (Test-Path $filePath) {
        $computedHash = (Get-FileHash -Path $filePath -Algorithm SHA256).Hash
        return $computedHash -eq $expectedHash
    }
    return $false
}

# Function to download a file with retry logic
function Download-File {
    param ($url, $destination, $expectedHash)

    $maxRetries = 3
    $retry = 0
    $success = $false

    while ($retry -lt $maxRetries -and -not $success) {
        try {
            if (Verify-FileHash $destination $expectedHash) {
                Write-Host "File already downloaded and verified: $destination"
                return
            }

            Write-Host "Downloading $destination..."
            $wc.DownloadFile($url, $destination)

            if (Verify-FileHash $destination $expectedHash) {
                Write-Host "File verified successfully: $destination"
                $success = $true
            } else {
                Write-Host "Hash verification failed. Retrying..."
                Remove-Item -Force $destination
            }
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
$gccarm_installed = Is-Installed "C:\Program Files (x86)\GNU Arm Embedded Toolchain"
$cmake_installed = Get-Command cmake -ErrorAction SilentlyContinue
$termite_installed = Is-Installed "C:\Program Files (x86)\Termite"

if ($gccarm_installed -and $cmake_installed -and $termite_installed) {
    Write-Host "All required packages are already installed."
} else {
    Write-Host "`nDownloading required packages..."

    # Download and install GNU Arm Toolchain if not installed
    if (-not $gccarm_installed) {
        $localPath = "$env:TEMP\$gccarm_file"
        Write-Host "(1/3) Downloading $gccarm_name..."
        Download-File $gccarm_url $localPath $gccarm_hash

        Write-Host "Installing $gccarm_name..."
        Start-Process -FilePath $localPath -ArgumentList "/S" -Wait
    }

    # Download and install CMake if not installed
    if (-not $cmake_installed) {
        $localPath = "$env:TEMP\$cmake_file"
        Write-Host "(2/3) Downloading $cmake_name..."
        Download-File $cmake_url $localPath $cmake_hash

        Write-Host "Installing $cmake_name..."
        Start-Process -FilePath $localPath -ArgumentList "ADD_CMAKE_TO_PATH=System /passive" -Wait
    }

    # Download and install Termite if not installed
    if (-not $termite_installed) {
        $localPath = "$env:TEMP\$termite_file"
        Write-Host "(3/3) Downloading $termite_name..."
        Download-File $termite_url $localPath $termite_hash

        Write-Host "Installing $termite_name..."
        Start-Process -FilePath $localPath -ArgumentList "/S" -Wait
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

# Define paths to add
$pathsToAdd = @(
    "C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.10\bin",
    "C:\Program Files (x86)\CMake\bin",
    "C:\Program Files\CMake\bin",
    "C:\Program Files (x86)\Termite"
)

# Add each path securely
foreach ($path in $pathsToAdd) {
    if (Is-Installed $path) {
        Add-ToSystemPath -NewPath $path
    }
}
Write-Host "`nSystem PATH update request sent. Restart your system for changes to take effect."
