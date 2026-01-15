#!/bin/sh

# Get the absolute path of the script's directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Define the correct `shared/lib` directory
LIB_DIR="$SCRIPT_DIR/../shared/lib"

# Function to check if a package is installed
is_package_installed() {
    dpkg-query -W -f='${Status}' "$1" 2>/dev/null | grep -q "ok installed"
}

# List of required packages
packages="apt-transport-https ca-certificates gnupg software-properties-common wget gcc-arm-none-eabi ninja-build cmake git"

# Update package lists
echo "Updating package lists..."
sudo apt-get update -qq

all_installed=true

# Check if all required packages are installed
for package in $packages; do
    if ! is_package_installed "$package"; then
        all_installed=false
        break
    fi
done

# Install missing packages if necessary
if $all_installed; then
    echo "All required packages are already installed."
else
    echo "Installing missing packages..."
    for package in $packages; do
        if ! is_package_installed "$package"; then
            echo "Installing $package..."
            sudo apt-get install -y "$package"
        fi
    done
    echo "All required packages are now installed."
fi

# Ensure the `shared/lib` directory exists
echo "Ensuring $LIB_DIR exists..."
mkdir -p "$LIB_DIR"

# Move to the correct directory
cd "$LIB_DIR" || { echo "Failed to navigate to $LIB_DIR"; exit 1; }

# List of repositories to clone or update
repos=(
    "https://github.com/eclipse-threadx/threadx.git"
    "https://github.com/eclipse-threadx/netxduo.git"
)

# Clone or update repositories
echo "Cloning or updating required repositories into $LIB_DIR..."

for repo in "${repos[@]}"; do
    repo_name=$(basename "$repo" .git)
    target_dir="$LIB_DIR/$repo_name"

    if [ -d "$target_dir" ]; then
        echo "Updating existing repository: $repo_name..."
        cd "$target_dir" || { echo "Failed to enter $target_dir"; exit 1; }
        git pull origin main || { echo "Failed to update $repo_name"; exit 1; }
        cd "$LIB_DIR"
    else
        echo "Cloning $repo into $target_dir..."
        git clone --depth=1 "$repo" "$target_dir"
    fi
done

echo "All repositories have been successfully updated in $LIB_DIR."
