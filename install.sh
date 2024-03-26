#!/bin/bash

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

# Function to download and install nitro
install_nitro() {
    rm -rf /tmp/nitro
    rm /tmp/nitro.tar.gz
    echo "Downloading Nitro version $VERSION... from $1"
    curl -sL "$1" -o /tmp/nitro.tar.gz
    tar -xzvf /tmp/nitro.tar.gz -C /tmp
    ls /tmp/nitro

    # Copying files to /usr/local/bin
    for file in /tmp/nitro/*; do
        chmod +x "$file"
        cp "$file" /usr/local/bin/
    done
}

# Function to create uninstall script
create_uninstall_script() {
    echo '#!/bin/bash' > /tmp/uninstall_nitro.sh
    echo 'if [ "$(id -u)" != "0" ]; then' >> /tmp/uninstall_nitro.sh
    echo '    echo "This script must be run as root. Please run again with sudo."' >> /tmp/uninstall_nitro.sh
    echo '    exit 1' >> /tmp/uninstall_nitro.sh
    echo 'fi' >> /tmp/uninstall_nitro.sh
    for file in /tmp/nitro/*; do
        echo "rm /usr/local/bin/$(basename "$file")" >> /tmp/uninstall_nitro.sh
    done
    echo "rm /usr/local/bin/uninstall_nitro.sh" >> /tmp/uninstall_nitro.sh
    echo 'echo "Nitro remove successfully."' >> /tmp/uninstall_nitro.sh
    chmod +x /tmp/uninstall_nitro.sh
    mv /tmp/uninstall_nitro.sh /usr/local/bin/
}

# Determine OS and architecture
OS=$(uname -s)
VERSION="latest"
GPU=""

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
    esac
done

# Notify if GPU option is not supported
if [ "$GPU" == "-cuda" ] && [ "$OS" == "Darwin" ]; then
    echo "GPU option is only supported on Linux or Windows."
    exit 1
fi

# Construct GitHub API URL and get latest version if not specified
if [ "$VERSION" == "latest" ]; then
    API_URL="https://api.github.com/repos/janhq/nitro/releases/latest"
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
        FILE_NAME="nitro-${VERSION}-linux-amd64${GPU}.tar.gz"
        ;;
    Darwin)
        ARCH_FORMAT="mac-universal"
        FILE_NAME="nitro-${VERSION}-${ARCH_FORMAT}.tar.gz"
        ;;
    *)
        echo "Unsupported OS."
        exit 1
        ;;
esac

DOWNLOAD_URL="https://github.com/janhq/nitro/releases/download/v${VERSION}/${FILE_NAME}"

# Download, install, and create uninstall script
install_nitro "$DOWNLOAD_URL"
create_uninstall_script

echo "Nitro installed successfully."
