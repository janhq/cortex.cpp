#!/bin/bash -e

# Check for root privileges
if [ "$(id -u)" != "0" ]; then
    echo "This script must be run as root. Please run again with sudo."
    exit 1
fi

# Check and suggest installing jq and tar if not present
check_install_jq_tar() {
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    NC='\033[0m' # No Color

    if ! command -v jq &> /dev/null; then
        echo -e "${RED}jq could not be found ...${NC}"
        if [[ "$OS" == "Linux" ]]; then
            echo -e "${GREEN}Please run the command below to install jq then rerun this script${NC}"
            echo "$ sudo apt-get install jq"
            exit 1
        elif [[ "$OS" == "Darwin" ]]; then
            echo -e "${GREEN}Please run the command below to install jq then rerun this script${NC}"
            echo "$ brew install jq"
            exit 1
        fi
    fi

    if ! command -v tar &> /dev/null; then
        echo -e "${RED}tar could not be found ...${NC}"
        if [[ "$OS" == "Linux" ]]; then
            echo -e "${GREEN}Please run the command below to install tar then rerun this script${NC}"
            echo "$ sudo apt install tar gzip"
            exit 1
        elif [[ "$OS" == "Darwin" ]]; then
            echo -e "${GREEN}Please run the command below to install tar then rerun this script${NC}"
            echo "$ brew install gnu-tar"
            exit 1
        fi
    fi
}

determine_avx_support() {
    if grep -q avx512 /proc/cpuinfo; then
        echo "-avx512"
    elif grep -q avx2 /proc/cpuinfo; then
        echo "-avx2"
    elif grep -q avx /proc/cpuinfo; then
        echo "-avx"
    else
        echo ""
    fi
}

# Function to download and install cortex-cpp
install_cortex-cpp() {
    rm -rf /tmp/cortex-cpp
    rm /tmp/cortex-cpp.tar.gz
    echo "Downloading cortex-cpp version $VERSION... from $1"
    curl -sL "$1" -o /tmp/cortex-cpp.tar.gz
    tar -xzvf /tmp/cortex-cpp.tar.gz -C /tmp
    ls /tmp/cortex-cpp

    # Copying files to /usr/local/bin
    for file in /tmp/cortex-cpp/*; do
        chmod +x "$file"
        cp "$file" /usr/local/bin/
    done
}

# Function to create uninstall script
create_uninstall_script() {
    echo '#!/bin/bash' > /tmp/uninstall_cortex-cpp.sh
    echo 'if [ "$(id -u)" != "0" ]; then' >> /tmp/uninstall_cortex-cpp.sh
    echo '    echo "This script must be run as root. Please run again with sudo."' >> /tmp/uninstall_cortex-cpp.sh
    echo '    exit 1' >> /tmp/uninstall_cortex-cpp.sh
    echo 'fi' >> /tmp/uninstall_cortex-cpp.sh
    for file in /tmp/cortex-cpp/*; do
        echo "rm /usr/local/bin/$(basename "$file")" >> /tmp/uninstall_cortex-cpp.sh
    done
    echo "rm /usr/local/bin/uninstall_cortex-cpp.sh" >> /tmp/uninstall_cortex-cpp.sh
    echo 'echo "cortex-cpp remove successfully."' >> /tmp/uninstall_cortex-cpp.sh
    chmod +x /tmp/uninstall_cortex-cpp.sh
    mv /tmp/uninstall_cortex-cpp.sh /usr/local/bin/
}

# Determine OS and architecture
OS=$(uname -s)
ARCH=$(uname -m)
VERSION="latest"
GPU=""
AVX=""

check_install_jq_tar

# Parse arguments
for arg in "$@"
do
    case $arg in
        --gpu)
        GPU="-cuda"
        shift
        ;;
        --version)
        VERSION="$2"
        shift
        shift
        ;;
        --avx)
        AVX="-avx"
        shift
        ;;
        --avx2)
        AVX="-avx2"
        shift
        ;;
        --avx512)
        AVX="-avx512"
        shift
        ;;
    esac
done

# Notify if GPU option is not supported
if [ "$GPU" == "-cuda" ] && [ "$OS" == "Darwin" ]; then
    echo "GPU option is only supported on Linux or Windows."
    exit 1
fi

# There are only two supported CUDA version. Let it fail if unsupported.
if [ "$GPU" == "-cuda" ]; then
    export PATH="/usr/local/cuda/bin:$PATH"
    CUDA_MAJ=$(nvcc --version | grep -oP "(?<=release )\d+") || 0
    if [[ $CUDA_MAJ == 12 ]]; then
        CUDA_VERSION="-12-0";
    elif [[ $CUDA_MAJ == 11 ]]; then
        CUDA_VERSION="-11-7"
    fi
fi

# Construct GitHub API URL and get latest version if not specified
if [ "$VERSION" == "latest" ]; then
    API_URL="https://api.github.com/repos/menloresearch/cortex.cpp/releases/latest"
    VERSION=$(curl -s $API_URL | jq -r ".tag_name" | sed 's/^v//')
fi

# Check if version is empty
if [ -z "$VERSION" ]; then
    echo "Failed to fetch latest version."
    exit 1
fi

# Construct download URL based on OS, ARCH, GPU and VERSION
case $OS in
    Linux)
        if [ -z "$AVX" ]; then
            AVX=$(determine_avx_support)
        fi
        FILE_NAME="cortex-cpp-${VERSION}-linux-amd64${AVX}${GPU}${CUDA_VERSION}.tar.gz"
        ;;
    Darwin)
        ARCH_FORMAT="mac-universal"
        FILE_NAME="cortex-cpp-${VERSION}-${ARCH_FORMAT}.tar.gz"
        ;;
    *)
        echo "Unsupported OS."
        exit 1
        ;;
esac

DOWNLOAD_URL="https://github.com/menloresearch/cortex/releases/download/v${VERSION}/${FILE_NAME}"

# Check AVX support
if [ -z "$AVX" ] && [ "$OS" == "Linux" ]; then
    echo "AVX is not supported on this system."
    exit 1
fi

# Remove existing cortex-cpp installation
echo "Removing existing cortex-cpp installation..."
rm -rf /usr/local/bin/cortex-cpp

# Download, install, and create uninstall script
install_cortex-cpp "$DOWNLOAD_URL"
create_uninstall_script

echo "cortex-cpp installed successfully."
