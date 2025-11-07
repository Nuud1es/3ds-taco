#!/bin/bash

# 3DS-TACO Build Script
# Automatically downloads devkitPro toolchain and compiles the application

set -e  # Exit on error

echo "================================"
echo "3DS-TACO Build Script"
echo "================================"
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Detect OS
OS="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    OS="windows"
fi

echo -e "${BLUE}Detected OS: $OS${NC}"
echo ""

# Check if devkitPro is installed
check_devkitpro() {
    if [ -z "$DEVKITPRO" ]; then
        if [ -d "/opt/devkitpro" ]; then
            export DEVKITPRO=/opt/devkitpro
            export DEVKITARM=/opt/devkitpro/devkitARM
        elif [ -d "$HOME/.local/share/devkitpro" ]; then
            export DEVKITPRO=$HOME/.local/share/devkitpro
            export DEVKITARM=$HOME/.local/share/devkitpro/devkitARM
        else
            return 1
        fi
    fi

    if [ -d "$DEVKITARM" ]; then
        echo -e "${GREEN}✓ devkitARM found at: $DEVKITARM${NC}"
        return 0
    else
        return 1
    fi
}

# Install devkitPro on Linux
install_devkitpro_linux() {
    echo -e "${YELLOW}Installing devkitPro for Linux...${NC}"

    # Download the package
    if ! command -v wget &> /dev/null; then
        echo -e "${RED}wget not found. Please install wget first.${NC}"
        echo "  Ubuntu/Debian: sudo apt-get install wget"
        echo "  Fedora: sudo dnf install wget"
        echo "  Arch: sudo pacman -S wget"
        exit 1
    fi

    # Download devkitpro-pacman
    echo "Downloading devkitpro-pacman..."
    wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman.amd64.deb -O /tmp/devkitpro-pacman.deb

    # Install the package
    echo "Installing devkitpro-pacman (requires sudo)..."
    sudo dpkg -i /tmp/devkitpro-pacman.deb
    rm /tmp/devkitpro-pacman.deb

    # Update package database
    echo "Updating package database..."
    sudo dkp-pacman -Sy

    # Install 3ds-dev
    echo "Installing 3ds-dev package..."
    sudo dkp-pacman -S --noconfirm 3ds-dev

    # Set environment variables
    export DEVKITPRO=/opt/devkitpro
    export DEVKITARM=/opt/devkitpro/devkitARM

    echo -e "${GREEN}✓ devkitPro installed successfully!${NC}"
}

# Install devkitPro on macOS
install_devkitpro_macos() {
    echo -e "${YELLOW}Installing devkitPro for macOS...${NC}"

    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        echo -e "${RED}Homebrew not found. Please install Homebrew first:${NC}"
        echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi

    # Tap devkitPro
    echo "Tapping devkitpro/tools..."
    brew tap devkitpro/tools

    # Install devkitPro
    echo "Installing devkitPro pacman..."
    brew install devkitpro-pacman

    # Install 3ds-dev
    echo "Installing 3ds-dev package..."
    sudo dkp-pacman -S --noconfirm 3ds-dev

    # Set environment variables
    export DEVKITPRO=/opt/devkitpro
    export DEVKITARM=/opt/devkitpro/devkitARM

    echo -e "${GREEN}✓ devkitPro installed successfully!${NC}"
}

# Install devkitPro on Windows (MSYS2)
install_devkitpro_windows() {
    echo -e "${YELLOW}Installing devkitPro for Windows...${NC}"

    # Check if we're in MSYS2
    if ! command -v pacman &> /dev/null; then
        echo -e "${RED}Error: This script requires MSYS2 on Windows.${NC}"
        echo "Please download and install MSYS2 from: https://www.msys2.org/"
        echo "Then run this script from the MSYS2 terminal."
        exit 1
    fi

    # Download and install devkitpro keyring
    echo "Installing devkitpro keyring..."
    wget https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-keyring.pkg.tar.xz
    pacman -U --noconfirm devkitpro-keyring.pkg.tar.xz
    rm devkitpro-keyring.pkg.tar.xz

    # Add devkitPro repository to pacman.conf if not already added
    if ! grep -q "\[dkp-libs\]" /etc/pacman.conf; then
        echo "Adding devkitPro repositories to pacman.conf..."
        cat >> /etc/pacman.conf << EOF

[dkp-libs]
Server = https://downloads.devkitpro.org/packages

[dkp-windows]
Server = https://downloads.devkitpro.org/packages/windows
EOF
    fi

    # Update package database
    echo "Updating package database..."
    pacman -Sy

    # Install 3ds-dev
    echo "Installing 3ds-dev package..."
    pacman -S --noconfirm 3ds-dev

    # Set environment variables
    export DEVKITPRO=/opt/devkitpro
    export DEVKITARM=/opt/devkitpro/devkitARM

    echo -e "${GREEN}✓ devkitPro installed successfully!${NC}"
}

# Main installation logic
if ! check_devkitpro; then
    echo -e "${YELLOW}devkitPro not found. Installing...${NC}"
    echo ""

    case $OS in
        linux)
            install_devkitpro_linux
            ;;
        macos)
            install_devkitpro_macos
            ;;
        windows)
            install_devkitpro_windows
            ;;
        *)
            echo -e "${RED}Unsupported OS: $OS${NC}"
            echo "Please manually install devkitPro from: https://devkitpro.org/"
            exit 1
            ;;
    esac

    echo ""
else
    echo -e "${GREEN}✓ devkitPro is already installed${NC}"
    echo ""
fi

# Verify installation
if ! check_devkitpro; then
    echo -e "${RED}Error: devkitPro installation failed or not found.${NC}"
    exit 1
fi

# Add to PATH if needed
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    export PATH=$DEVKITARM/bin:$PATH
fi

# Verify compiler
if ! command -v arm-none-eabi-gcc &> /dev/null; then
    echo -e "${RED}Error: arm-none-eabi-gcc not found in PATH.${NC}"
    echo "DEVKITARM is set to: $DEVKITARM"
    exit 1
fi

echo -e "${GREEN}✓ Compiler verified: $(arm-none-eabi-gcc --version | head -n1)${NC}"
echo ""

# Build the application
echo -e "${BLUE}Building 3DS-TACO...${NC}"
echo "================================"

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    make clean
fi

# Build
echo "Compiling..."
if make; then
    echo ""
    echo -e "${GREEN}================================${NC}"
    echo -e "${GREEN}✓ Build successful!${NC}"
    echo -e "${GREEN}================================${NC}"
    echo ""
    echo -e "${BLUE}Output files:${NC}"
    if [ -f "3ds-taco.3dsx" ]; then
        FILE_SIZE=$(du -h "3ds-taco.3dsx" | cut -f1)
        echo -e "  ${GREEN}✓${NC} 3ds-taco.3dsx (${FILE_SIZE})"
    fi
    if [ -f "3ds-taco.elf" ]; then
        echo -e "  ${GREEN}✓${NC} 3ds-taco.elf"
    fi
    if [ -f "3ds-taco.smdh" ]; then
        echo -e "  ${GREEN}✓${NC} 3ds-taco.smdh"
    fi
    echo ""
    echo -e "${BLUE}Next steps:${NC}"
    echo "  1. Copy 3ds-taco.3dsx to your SD card's /3ds/ folder"
    echo "  2. Copy config.ini.example to SD:/3ds/3ds-taco/config.ini"
    echo "  3. Edit the config file with your OBD-II adapter settings"
    echo "  4. Launch from Homebrew Launcher on your 3DS"
    echo ""
else
    echo ""
    echo -e "${RED}================================${NC}"
    echo -e "${RED}✗ Build failed!${NC}"
    echo -e "${RED}================================${NC}"
    echo ""
    echo -e "${YELLOW}Common issues:${NC}"
    echo "  - Make sure all dependencies are installed"
    echo "  - Check that DEVKITPRO and DEVKITARM are set correctly"
    echo "  - Try running 'make clean' and building again"
    echo ""
    exit 1
fi

# Save environment variables for future use
ENV_FILE="$HOME/.bashrc"
if [[ "$OS" == "macos" ]]; then
    ENV_FILE="$HOME/.bash_profile"
fi

if ! grep -q "DEVKITPRO" "$ENV_FILE" 2>/dev/null; then
    echo ""
    echo -e "${YELLOW}Adding devkitPro environment variables to $ENV_FILE...${NC}"
    cat >> "$ENV_FILE" << 'EOF'

# devkitPro environment variables
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=$DEVKITPRO/devkitARM
export PATH=$DEVKITARM/bin:$PATH
EOF
    echo -e "${GREEN}✓ Environment variables added. Please run 'source $ENV_FILE' or restart your terminal.${NC}"
fi

echo -e "${GREEN}All done! Happy hacking! 🎮🚗${NC}"
