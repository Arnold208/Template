#!/bin/bash

# Get the absolute path of the script's directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Define the correct `shared/lib` directory
LIB_DIR="$SCRIPT_DIR/../shared/lib"

# Function to check if a package is installed via Homebrew
is_package_installed() {
    brew list $1 &>/dev/null
}

# Function to check if Homebrew is installed
is_homebrew_installed() {
    command -v brew &>/dev/null
}

# Function to check if Xcode Command Line Tools are installed
is_xcode_installed() {
    xcode-select -p &>/dev/null
}

# Required packages
packages="wget gcc-arm-none-eabi ninja cmake git"

# List of repositories to clone
repos=(
    "https://github.com/eclipse-threadx/threadx.git"
    "https://github.com/eclipse-threadx/netxduo.git"
)

# Install Xcode Command Line Tools if missing
if ! is_xcode_installed; then
    echo "Xcode Command Line Tools not found. Installing..."
    xcode-select --install
    # Wait for user to complete installation
    while ! is_xcode_installed; do
        sleep 5
    done
    echo "Xcode Command Line Tools installed."
fi

# Install Homebrew if missing
if ! is_homebrew_installed; then
    echo "Homebrew is not installed. Installing Homebrew..."
    
    # Detect if running on Apple Silicon or Intel
    if [[ $(uname -m) == "arm64" ]]; then
        # Apple Silicon (M1/M2/M3) Homebrew installation
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        eval "$(/opt/homebrew/bin/brew shellenv)"
    else
        # Intel Mac Homebrew installation
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
        eval "$(/usr/local/bin/brew shellenv)"
    fi

    # Verify installation
    if ! is_homebrew_installed; then
        echo "Homebrew installation failed. Please install it manually and re-run this script."
        exit 1
    fi
else
    echo "Homebrew is already installed."
fi

# Update Homebrew
echo "Updating Homebrew..."
brew update

# Install missing packages
echo "Checking required packages..."
for package in $packages; do
    if ! is_package_installed $package; then
        echo "Installing $package..."
        brew install $package
    fi
done

echo "All required packages are installed and up to date."

# Ensure the `shared/lib` directory exists
echo "Ensuring $LIB_DIR exists..."
mkdir -p "$LIB_DIR"

# Move to the correct directory
cd "$LIB_DIR" || { echo "Failed to navigate to $LIB_DIR"; exit 1; }

# Clone or update repositories
echo "Cloning or updating required repositories in $LIB_DIR..."

for repo in "${repos[@]}"; do
    repo_name=$(basename "$repo" .git)
    target_dir="$LIB_DIR/$repo_name"

    # Remove old repository if it exists
    if [ -d "$target_dir" ]; then
        echo "Removing old version of $repo_name..."
        rm -rf "$target_dir"
    fi

    # Clone the repository
    echo "Cloning $repo into $target_dir..."
    git clone --depth=1 "$repo" "$target_dir"
done

echo "All repositories have been successfully downloaded into $LIB_DIR."
