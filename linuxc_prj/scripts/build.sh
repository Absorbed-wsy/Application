#!/bin/bash

# Linux C Project Build Script
# Features: Check build environment, support native compilation and cross-compilation, support Debug/Release modes

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script Information
SCRIPT_NAME="$(basename "$0")"
SCRIPT_VERSION="1.0.0"
# Detect project root directory (supports symbolic links)
SCRIPT_DIR="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

# Default Parameters
BUILD_TYPE="Release"
TARGET_ARCH="native"
CLEAN_BUILD=false
VERBOSE=false
SHOW_HELP=false

# Show Help Information
show_help() {
    cat << EOF
${SCRIPT_NAME} - Linux C Project Build Script v${SCRIPT_VERSION}

Usage: ${SCRIPT_NAME} [options]

Options:
    -a, --arch ARCH     Target architecture (native|arm) [default: native]
    -t, --type TYPE     Build type (Debug|Release|RelWithDebInfo|MinSizeRel) [default: Release]
    -c, --clean         Clean build directory
    -v, --verbose       Verbose output
    -h, --help          Show this help message

Examples:
    ${SCRIPT_NAME}                    # Default: Native Release build
    ${SCRIPT_NAME} -a arm -t Debug    # ARM architecture Debug build
    ${SCRIPT_NAME} --clean            # Clean build directory
    ${SCRIPT_NAME} -v                 # Verbose output mode

EOF
}

# Print colored messages
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if command exists
check_command() {
    if ! command -v "$1" &> /dev/null; then
        print_error "Command '$1' not found, please install it first"
        return 1
    fi
    return 0
}

# Check build environment
check_environment() {
    print_info "Checking build environment..."
    
    local required_commands=("gcc" "g++" "cmake" "make")
    local missing_commands=()
    
    for cmd in "${required_commands[@]}"; do
        if ! check_command "$cmd"; then
            missing_commands+=("$cmd")
        fi
    done
    
    if [ ${#missing_commands[@]} -gt 0 ]; then
        print_error "Missing required build tools: ${missing_commands[*]}"
        print_info "Please run the following command to install: sudo apt install gcc g++ cmake make"
        return 1
    fi
    
    # Check optional tools
    local optional_commands=("gdb" "strace" "git" "unzip")
    for cmd in "${optional_commands[@]}"; do
        if ! command -v "$cmd" &> /dev/null; then
            print_warning "Optional tool '$cmd' not installed"
        fi
    done
    
    print_success "Build environment check passed"
    return 0
}

# Check ARM cross-compilation environment
check_arm_toolchain() {
    print_info "Checking ARM cross-compilation environment..."
    
    local toolchain_file="${PROJECT_ROOT}/arm-toolchain.cmake"
    if [ ! -f "$toolchain_file" ]; then
        print_error "ARM toolchain file does not exist: $toolchain_file"
        return 1
    fi
    
    # Check if toolchain path exists
    local toolchain_dir=$(grep "TOOLCHAIN_DIR" "$toolchain_file" | head -1 | cut -d'"' -f2)
    if [ -n "$toolchain_dir" ] && [ ! -d "$toolchain_dir" ]; then
        print_warning "ARM toolchain directory does not exist: $toolchain_dir"
        print_info "Please ensure the cross-compilation toolchain is properly installed"
    fi
    
    print_success "ARM cross-compilation environment check completed"
    return 0
}

# Clean build directory
clean_build() {
    print_info "Cleaning build directory..."
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_success "Build directory cleaned"
    else
        print_warning "Build directory does not exist, no need to clean"
    fi
}

# Parse command line arguments
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -a|--arch)
                TARGET_ARCH="$2"
                shift 2
                ;;
            -t|--type)
                BUILD_TYPE="$2"
                shift 2
                ;;
            -c|--clean)
                CLEAN_BUILD=true
                shift
                ;;
            -v|--verbose)
                VERBOSE=true
                shift
                ;;
            -h|--help)
                SHOW_HELP=true
                shift
                ;;
            *)
                print_error "Unknown parameter: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# Validate arguments
validate_arguments() {
    # Validate architecture parameter
    case "$TARGET_ARCH" in
        native|arm)
            ;;
        *)
            print_error "Unsupported architecture: $TARGET_ARCH"
            print_info "Supported architectures: native, arm"
            return 1
            ;;
    esac
    
    # Validate build type
    case "$BUILD_TYPE" in
        Debug|Release|RelWithDebInfo|MinSizeRel)
            ;;
        *)
            print_error "Unsupported build type: $BUILD_TYPE"
            print_info "Supported build types: Debug, Release, RelWithDebInfo, MinSizeRel"
            return 1
            ;;
    esac
    
    return 0
}

# Execute compilation
build_project() {
    print_info "Starting project compilation..."
    print_info "Target architecture: $TARGET_ARCH"
    print_info "Build type: $BUILD_TYPE"
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR" || {
        print_error "Cannot enter build directory: $BUILD_DIR"
        return 1
    }
    
    # Configure CMake
    local cmake_cmd="cmake"
    
    # Add architecture-related parameters
    if [ "$TARGET_ARCH" = "arm" ]; then
        cmake_cmd="$cmake_cmd -DCMAKE_TOOLCHAIN_FILE=${PROJECT_ROOT}/arm-toolchain.cmake"
    fi
    
    # Add build type parameters
    cmake_cmd="$cmake_cmd -DCMAKE_BUILD_TYPE=${BUILD_TYPE}"
    
    # Add verbose output
    if [ "$VERBOSE" = true ]; then
        cmake_cmd="$cmake_cmd -DCMAKE_VERBOSE_MAKEFILE=ON"
    fi
    
    cmake_cmd="$cmake_cmd .."
    
    print_info "Executing CMake configuration: $cmake_cmd"
    if ! eval "$cmake_cmd"; then
        print_error "CMake configuration failed"
        return 1
    fi
    
    # Execute compilation
    print_info "Executing compilation..."
    local make_cmd="make"
    if [ "$VERBOSE" = true ]; then
        make_cmd="$make_cmd VERBOSE=1"
    fi
    
    if ! eval "$make_cmd"; then
        print_error "Compilation failed"
        return 1
    fi
    
    print_success "Compilation completed"
    
    # Display generated executable file information
    if [ -f "main_app" ]; then
        print_info "Generated executable file: ${BUILD_DIR}/main_app"
        file "main_app"
    fi
    
    return 0
}

# Main function
main() {
    print_info "Linux C Project Build Script v${SCRIPT_VERSION}"
    
    # Parse arguments
    parse_arguments "$@"
    
    if [ "$SHOW_HELP" = true ]; then
        show_help
        exit 0
    fi
    
    # Validate arguments
    if ! validate_arguments; then
        exit 1
    fi
    
    # Clean build directory (if -c parameter is specified, clean and exit)
    if [ "$CLEAN_BUILD" = true ]; then
        clean_build
        print_success "Clean operation completed"
        exit 0
    fi
    
    # Check environment (only check when compilation is needed)
    if ! check_environment; then
        exit 1
    fi
    
    # If ARM compilation, check cross-compilation environment
    if [ "$TARGET_ARCH" = "arm" ]; then
        if ! check_arm_toolchain; then
            print_warning "ARM cross-compilation environment check failed, but will continue to attempt compilation"
        fi
    fi
    
    # Execute compilation
    if ! build_project; then
        print_error "Compilation process failed"
        exit 1
    fi
    
    print_success "All operations completed"
}

# Script entry point
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi