#!/bin/bash -e

# Check for root privileges
if [ "$(id -u)" != "0" ]; then
    echo "This script must be run as root. Please run again with sudo."
    exit 1
fi

# Determine architecture
ARCH=$(uname -m)
if [ "$ARCH" = "x86_64" ]; then
    ARCH="amd64"
elif [ "$ARCH" = "aarch64" ]; then
    ARCH="arm64"
else
    echo "Unsupported architecture: $ARCH"
    exit 1
fi

# Determine the home directory based on the user
USER_TO_RUN_AS=${SUDO_USER:-$(whoami)}
if [ "$USER_TO_RUN_AS" = "root" ]; then
    USER_HOME="/root"
else
    USER_HOME="/home/$USER_TO_RUN_AS"
fi

# Check and suggest installing jq and tar if not present
check_install_jq_tar() {
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    NC='\033[0m' # No Color

    if ! command -v jq &> /dev/null; then
        echo -e "${RED}jq could not be found ...${NC}"
        echo -e "${GREEN}Please install jq then rerun this script${NC}"
        exit 1
    fi

    if ! command -v tar &> /dev/null; then
        echo -e "${RED}tar could not be found ...${NC}"
        echo -e "${GREEN}Please install tar then rerun this script${NC}"
        exit 1
    fi
}

# Function to fetch the latest version based on channel
get_latest_version() {
  local channel=$1
  local tag_name
  case $channel in
    stable)
      tag_name=$(curl -s "https://api.github.com/repos/janhq/cortex.cpp/releases/latest" | grep -oP '"tag_name": "\K(.*)(?=")')
      ;;
    beta)
      tag_name=$(curl -s "https://api.github.com/repos/janhq/cortex.cpp/releases" | jq -r '.[] | select(.prerelease) | .tag_name' | head -n 1)
      ;;
    nightly)
      tag_name=$(curl -s "https://delta.jan.ai/cortex/latest/version.json" | jq -r '.tag_name')
      ;;
    *)
      echo "Invalid channel specified."
      exit 1
      ;;
  esac
  echo "${tag_name#v}"
}

# Default values
CHANNEL="stable"
VERSION=""
IS_UPDATE="false"
DEB_LOCAL="false"

# Function to parse command-line arguments
parse_args() {
  while [[ "$#" -gt 0 ]]; do
    case $1 in
      --channel)
        CHANNEL="$2"
        shift 2
        ;;
      --version)
        VERSION="$2"
        shift 2
        ;;
      --deb_local)
        DEB_LOCAL="true"
        shift 1
        ;;
      --is_update)
        IS_UPDATE="true"
        shift 1
        ;;
      *)
        echo "Unknown option: $1"
        exit 1
        ;;
    esac
  done
}

# Call parse_args function to handle options
parse_args "$@"

# Check if VERSION is empty and fetch latest if necessary
if [ -z "$VERSION" ]; then
    VERSION=$(get_latest_version $CHANNEL)
fi

# Set paths based on channel
case $CHANNEL in
  stable)
    CLI_BINARY_NAME="cortex"
    SERVER_BINARY_NAME="cortex-server"
    DATA_DIR="$USER_HOME/cortexcpp"
    UNINSTALL_SCRIPT="/usr/bin/cortex-uninstall.sh"
    CONFIGURATION_FILE="$USER_HOME/.cortexrc"
    DEB_APP_NAME="cortexcpp"
    ;;
  beta)
    CLI_BINARY_NAME="cortex-beta"
    SERVER_BINARY_NAME="cortex-server-beta"
    DATA_DIR="$USER_HOME/cortexcpp-beta"
    UNINSTALL_SCRIPT="/usr/bin/cortex-beta-uninstall.sh"
    CONFIGURATION_FILE="$USER_HOME/.cortexrc-beta"
    DEB_APP_NAME="cortexcpp-beta"
    ;;
  nightly)
    CLI_BINARY_NAME="cortex-nightly"
    SERVER_BINARY_NAME="cortex-server-nightly"
    DATA_DIR="$USER_HOME/cortexcpp-nightly"
    UNINSTALL_SCRIPT="/usr/bin/cortex-nightly-uninstall.sh"
    CONFIGURATION_FILE="$USER_HOME/.cortexrc-nightly"
    DEB_APP_NAME="cortexcpp-nightly"
    ;;
  *)
    echo "Invalid channel specified."
    exit 1
    ;;
esac

INSTALL_DIR="/usr/bin"

# Function to download and extract cortex
install_cortex() {
  local channel=$1
  local version=$2
  local is_deb=$3
  local url_binary=""
  local url_deb_local=""
  local url_deb_network=""

  case $channel in
    stable)
      url_binary="https://github.com/janhq/cortex.cpp/releases/download/v${version}/cortex-${version}-linux-${ARCH}.tar.gz"
      url_deb_local="https://github.com/janhq/cortex.cpp/releases/download/v${version}/cortex-${version}-linux-${ARCH}-local-installer.deb"
      url_deb_network="https://github.com/janhq/cortex.cpp/releases/download/v${version}/cortex-${version}-linux-${ARCH}-network-installer.deb"
      ;;
    beta)
      url_binary="https://github.com/janhq/cortex.cpp/releases/download/v${version}/cortex-${version}-linux-${ARCH}.tar.gz"
      url_deb_local="https://github.com/janhq/cortex.cpp/releases/download/v${version}/cortex-${version}-linux-${ARCH}-local-installer.deb"
      url_deb_network="https://github.com/janhq/cortex.cpp/releases/download/v${version}/cortex-${version}-linux-${ARCH}-network-installer.deb"
      ;;
    nightly)
      url_binary="https://delta.jan.ai/cortex/v${version}/linux-${ARCH}/cortex-nightly.tar.gz"
      url_deb_local="https://delta.jan.ai/cortex/v${version}/linux-${ARCH}/cortex-${version}-linux-${ARCH}-local-installer.deb"
      url_deb_network="https://delta.jan.ai/cortex/v${version}/linux-${ARCH}/cortex-${version}-linux-${ARCH}-network-installer.deb"
      ;;
  esac

  if [ "$is_deb" = "true" ]; then
    # Download the deb package
    if [ "$DEB_LOCAL" = "true" ]; then
      echo "Downloading cortex $channel version $version from $url_deb_local"
      curl -L $url_deb_local -o /tmp/cortex.deb
    else
      echo "Downloading cortex $channel version $version from $url_deb_network"
      curl -L $url_deb_network -o /tmp/cortex.deb
    fi

    # Install the deb package
    if [ "$IS_UPDATE" = "false" ]; then
      apt-get install -y /tmp/cortex.deb
    else
      echo -e "n\n" | SKIP_POSTINSTALL=true apt-get install -y --allow-downgrades /tmp/cortex.deb
    fi
    rm -f /tmp/cortex.deb
  else
    echo "Downloading cortex $channel version $version from $url_binary"
    curl -L $url_binary -o /tmp/cortex.tar.gz
    tar -xzvf /tmp/cortex.tar.gz -C /tmp
    chmod +x /tmp/cortex/*
    cp /tmp/cortex/cortex* /usr/bin/
    # Check is update or not
    if [ "$IS_UPDATE" = "false" ]; then
      su -c "$INSTALL_DIR/$CLI_BINARY_NAME engines install llama-cpp" $USER_TO_RUN_AS
      su -c "$INSTALL_DIR/$CLI_BINARY_NAME stop > /dev/null 2>&1" $USER_TO_RUN_AS
    fi
    rm -rf /tmp/cortex
    rm -f /tmp/cortex.tar.gz
  fi
}

# Function to create uninstall script
create_uninstall_script() {
  local is_deb=$1
  if [ "$is_deb" = "false" ]; then
    cat << EOF > $UNINSTALL_SCRIPT
#!/bin/bash
# Check for root privileges
if [ "\$(id -u)" != "0" ]; then
    echo "This script must be run as root. Please run again with sudo."
    exit 1
fi

echo "Stopping cortex..."
su -c "$INSTALL_DIR/$CLI_BINARY_NAME stop > /dev/null 2>&1" $USER_TO_RUN_AS
rm -f $INSTALL_DIR/$CLI_BINARY_NAME
rm -f $INSTALL_DIR/$SERVER_BINARY_NAME
rm -f $UNINSTALL_SCRIPT

echo "Do you want to delete the $DATA_DIR data folder and file $CONFIGURATION_FILE? (yes/no) [default: no]"
read -r answer
while true; do
    case "\$answer" in
        [yY][eE][sS]|[yY])
            echo "Deleting cortex data folders..."
            if [ -d "$DATA_DIR" ]; then
                echo "Removing $DATA_DIR"
                rm -rf "$DATA_DIR" > /dev/null 2>&1
            fi
            if [ -f "$CONFIGURATION_FILE" ]; then
                echo "Removing $CONFIGURATION_FILE"
                rm -f "$CONFIGURATION_FILE" > /dev/null 2>&1
            fi
            break
            ;;
        [nN][oO]|[nN]|"")
            echo "Keeping the 'cortex' data folders."
            break
            ;;
        *)
            echo "Invalid response. Please type 'yes', 'no', 'y', or 'n' (case-insensitive)."
            read -r answer
            ;;
    esac
done

EOF

  else
    cat << EOF > $UNINSTALL_SCRIPT
#!/bin/bash
# Check for root privileges
if [ "\$(id -u)" != "0" ]; then
    echo "This script must be run as root. Please run again with sudo."
    exit 1
fi

apt-get remove -y $DEB_APP_NAME
rm -f $UNINSTALL_SCRIPT
EOF
  fi

  chmod +x $UNINSTALL_SCRIPT
  echo "Uninstall script created at $UNINSTALL_SCRIPT"
}

# Run installation
check_install_jq_tar

IS_DEB="false"

# Check if apt-get command is available
if command -v apt-get &> /dev/null; then
  if [ "$IS_UPDATE" = "true" ]; then
    # check if cortexcpp deb package is installed
    if dpkg -l | grep -q $DEB_APP_NAME; then
      IS_DEB="true"
    else
      IS_DEB="false"
    fi
  else
    IS_DEB="true"
  fi
fi

install_cortex $CHANNEL $VERSION $IS_DEB
create_uninstall_script $IS_DEB

echo "Installation complete. Run cortex-uninstall.sh to uninstall."
