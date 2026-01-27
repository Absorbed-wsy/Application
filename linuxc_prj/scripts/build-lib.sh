#!/bin/bash

# Generic library build script
# Supports cross-compilation for different packages with individual configurations

set -e

# Default configuration
CURRENT_USER=$(whoami)
TOOLCHAIN_DIR="/home/$CURRENT_USER/SDK/Toolchain/aarch64-rk-linux_gcc-10.3.1/bin"
TARGET_PLATFORM="aarch64-none-linux-gnu"
INSTALL_PATH="/home/$CURRENT_USER/SDK/Toolchain/aarch64-rk-linux_gcc-10.3.1/aarch64-none-linux-gnu/libc"

BUILD_DIR="build"
LIB_DIR="lib"
CONFIG_DIR="lib"

# Function to display usage
usage() {
    echo "Usage: $0 [options] <package_name>"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -c, --clean             Clean build directory for specified package"
    echo "  -j <num>, --jobs <num>  Number of parallel jobs (default: auto)"
    echo ""
    echo "Available packages:"
    for pkg in "$LIB_DIR"/*.tar.gz; do
        if [[ -f "$pkg" ]]; then
            pkg_name=$(basename "$pkg" .tar.gz)
            echo "  $pkg_name"
        fi
    done
    echo ""
    echo "Package-specific configurations are loaded from $CONFIG_DIR/<package_name>.conf"
}

# Function to load package configuration
load_package_config() {
    local package_name=$1
    local config_file="$CONFIG_DIR/${package_name}.conf"
    
    if [[ -f "$config_file" ]]; then
        echo "Loading configuration from $config_file"
        source "$config_file"
    else
        echo "Warning: No configuration file found for $package_name"
        echo "Using default build process"
        BUILD_TYPE="autotools"
    fi
}

# Function to extract package
extract_package() {
    local package_name=$1
    local archive_file="$LIB_DIR/${package_name}.tar.gz"
    
    if [[ ! -f "$archive_file" ]]; then
        echo "Error: Archive file $archive_file not found"
        exit 1
    fi
    
    echo "Extracting $archive_file to $BUILD_DIR"
    tar -xf "$archive_file" -C "$BUILD_DIR"
}

# setup environment
setup_env() {
    export CROSS_COMPILE="${TARGET_PLATFORM}-"
    export CC="${CROSS_COMPILE}gcc"
    export CXX="${CROSS_COMPILE}g++"
    export AR="${CROSS_COMPILE}ar"
    export AS="${CROSS_COMPILE}as"
    export LD="${CROSS_COMPILE}ld"
    export STRIP="${CROSS_COMPILE}strip"
    export PATH="$TOOLCHAIN_DIR:$PATH"
    
    echo "Environment setup:"
    echo "  TOOLCHAIN_DIR: $TOOLCHAIN_DIR"
    echo "  TARGET_PLATFORM: $TARGET_PLATFORM"
    echo "  INSTALL_PATH: $INSTALL_PATH"
    echo "  CC: $CC"
    echo "  CXX: $CXX"
    echo "  AR: $AR"
    echo "  LD: $LD"
    echo "  STRIP: $STRIP"
}

# Function to build package using autotools
build_autotools() {
    local package_name=$1
    local build_dir="$BUILD_DIR/$package_name"
    local jobs=$2

    cd "$build_dir"
    
    # Run pre-extract hook if defined
    if type pre_extract >/dev/null 2>&1; then
        pre_extract
    fi
    
    # Setup basic environment for autogen.sh
    export PATH="$TOOLCHAIN_DIR:$PATH"
    
    # Run autogen.sh if it exists
    if [[ -f "./autogen.sh" ]]; then
        echo "Running autogen.sh"
        ./autogen.sh
    fi
    
    # Setup full cross-compilation environment AFTER autogen.sh
    setup_env
    
    # Run pre-configure hook if defined
    if type pre_configure >/dev/null 2>&1; then
        pre_configure
    fi
    
    # Use custom configure if defined, otherwise use default
    if type custom_configure >/dev/null 2>&1; then
        echo "Running custom configure"
        custom_configure
    else
        echo "Configuring $package_name"
        ./configure \
            --host="$TARGET_PLATFORM" \
            --prefix="$INSTALL_PATH" \
            $CONFIGURE_OPTS
    fi
    
    # Run pre-build hook if defined
    if type pre_build >/dev/null 2>&1; then
        pre_build
    fi
    
    # Build and install
    echo "Building $package_name with $jobs jobs"
    make -j"$jobs"
    
    echo "Installing $package_name"
    make install
    
    # Run post-build hook if defined
    if type post_build >/dev/null 2>&1; then
        post_build
    fi
    
    cd - > /dev/null
}

# Function to clean build directory for specific package
clean_package_build() {
    local package_name=$1
    local package_build_dir="$BUILD_DIR/$package_name"
    
    if [[ -d "$package_build_dir" ]]; then
        echo "Cleaning build directory for $package_name"
        rm -rf "$package_build_dir"
    else
        echo "Build directory for $package_name does not exist"
    fi
}

# Main function
main() {
    # Parse command line arguments
    local package_name=""
    local clean_build_flag=false
    local jobs=$(nproc)
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                usage
                exit 0
                ;;
            -c|--clean)
                if [[ $# -gt 1 ]]; then
                    package_name="$2"
                    clean_build_flag=true
                    shift 2
                else
                    echo "Error: --clean requires a package name"
                    usage
                    exit 1
                fi
                ;;
            -j|--jobs)
                if [[ $# -gt 1 ]]; then
                    jobs="$2"
                    shift 2
                else
                    echo "Error: --jobs requires a number"
                    usage
                    exit 1
                fi
                ;;
            -*)
                echo "Error: Unknown option $1"
                usage
                exit 1
                ;;
            *)
                if [[ -z "$package_name" ]]; then
                    package_name="$1"
                else
                    echo "Error: Multiple package names specified"
                    usage
                    exit 1
                fi
                shift
                ;;
        esac
    done
    
    # Validate package name
    if [[ -z "$package_name" ]]; then
        echo "Error: Package name is required"
        usage
        exit 1
    fi
    
    # Check if package archive exists
    if [[ ! -f "$LIB_DIR/${package_name}.tar.gz" ]]; then
        echo "Error: Package $package_name not found in $LIB_DIR"
        echo "Available packages:"
        ls "$LIB_DIR"/*.tar.gz 2>/dev/null | sed 's|.*/||' | sed 's/\.tar\.gz$//' || echo "No packages found"
        exit 1
    fi
    
    # If only cleaning is requested
    if [[ "$clean_build_flag" == true && "$package_name" != "" ]]; then
        clean_package_build "$package_name"
        exit 0
    fi
    
    echo "Building package: $package_name"
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    mkdir -p "$INSTALL_PATH"
    
    # Load package-specific configuration
    load_package_config "$package_name"
    
    # Extract package
    extract_package "$package_name"
    
    # Build package based on build type
    if type custom_build >/dev/null 2>&1; then
        echo "Using custom build function"
        custom_build "$package_name" "$jobs"
    else
        echo "Building with autotools"
        build_autotools "$package_name" "$jobs"
    fi
    
    echo "Build completed successfully"
    echo "Package installed to: $INSTALL_PATH"
}

# Run main function with all arguments
main "$@"