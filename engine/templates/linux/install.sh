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

# Determine the home directory based on the user
USER_TO_RUN_AS=${SUDO_USER:-$(whoami)}
if [ "$USER_TO_RUN_AS" = "root" ]; then
    USER_HOME="/root"
else
    USER_HOME="/home/$USER_TO_RUN_AS"
fi

# Default values
CHANNEL="stable"
VERSION=""
IS_UPDATE="false"

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
    UPDATER_SCRIPT="/usr/bin/cortex-updater.sh"
    CONFIGURATION_FILE="$USER_HOME/.cortexrc"
    ;;
  beta)
    CLI_BINARY_NAME="cortex-beta"
    SERVER_BINARY_NAME="cortex-server-beta"
    DATA_DIR="$USER_HOME/cortexcpp-beta"
    UNINSTALL_SCRIPT="/usr/bin/cortex-beta-uninstall.sh"
    UPDATER_SCRIPT="/usr/bin/cortex-beta-updater.sh"
    CONFIGURATION_FILE="$USER_HOME/.cortexrc-beta"
    ;;
  nightly)
    CLI_BINARY_NAME="cortex-nightly"
    SERVER_BINARY_NAME="cortex-server-nightly"
    DATA_DIR="$USER_HOME/cortexcpp-nightly"
    UNINSTALL_SCRIPT="/usr/bin/cortex-nightly-uninstall.sh"
    UPDATER_SCRIPT="/usr/bin/cortex-nightly-updater.sh"
    CONFIGURATION_FILE="$USER_HOME/.cortexrc-nightly"
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
  local url=""

  case $channel in
    stable)
      url="https://github.com/janhq/cortex.cpp/releases/download/v${version}/cortex-${version}-linux-amd64.tar.gz"
      ;;
    beta)
      url="https://github.com/janhq/cortex.cpp/releases/download/v${version}/cortex-${version}-linux-amd64.tar.gz"
      ;;
    nightly)
      url="https://delta.jan.ai/cortex/v${version}/linux-amd64/cortex-nightly.tar.gz"
      ;;
  esac
  echo "Downloading cortex $channel version $version from $url"
  curl -L $url -o /tmp/cortex.tar.gz
  tar -xzvf /tmp/cortex.tar.gz -C /tmp
  chmod +x /tmp/cortex/*
  cp /tmp/cortex/* /usr/bin/
  # Check is update or not
  if [ "$IS_UPDATE" = "false" ]; then
    su -c "$INSTALL_DIR/$CLI_BINARY_NAME engines install llama-cpp" $USER_TO_RUN_AS
    su -c "$INSTALL_DIR/$CLI_BINARY_NAME stop > /dev/null 2>&1" $USER_TO_RUN_AS
  fi
}

# Function to create uninstall script
create_uninstall_script() {
  cat << EOF > $UNINSTALL_SCRIPT
#!/bin/bash
# Check for root privileges
if [ "\$(id -u)" != "0" ]; then
    echo "This script must be run as root. Please run again with sudo."
    exit 1
fi

echo "Stopping cortex..."
su -c "$INSTALL_DIR/$CLI_BINARY_NAME stop > /dev/null 2>&1" $USER_TO_RUN_AS
rm -rf $INSTALL_DIR/$CLI_BINARY_NAME
rm -rf $INSTALL_DIR/$SERVER_BINARY_NAME
rm -f $UNINSTALL_SCRIPT
rm -f $UPDATER_SCRIPT

echo "Do you want to delete the ~/$DATA_DIR data folder and file ~/$CONFIGURATION_FILE? (yes/no) [default: no]"
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
  chmod +x $UNINSTALL_SCRIPT
  echo "Uninstall script created at $UNINSTALL_SCRIPT"
}

# Function to create updater script
create_updater_script() {
  cat << EOF > $UPDATER_SCRIPT
#!/bin/bash
# Check for root privileges
if [ "\$(id -u)" != "0" ]; then
    echo "This script must be run as root. Please run again with sudo."
    exit 1
fi
echo "Stopping cortex for update..."
su -c "$INSTALL_DIR/$CLI_BINARY_NAME stop > /dev/null 2>&1" $USER_TO_RUN_AS
curl -s https://raw.githubusercontent.com/janhq/cortex/feat/linux-bash-install-script/engine/templates/linux/install.sh | bash -s -- --channel $CHANNEL --is_update
EOF
  chmod +x $UPDATER_SCRIPT
  echo "Updater script created at $UPDATER_SCRIPT"
}

# Run installation
check_install_jq_tar
install_cortex $CHANNEL $VERSION
create_uninstall_script
create_updater_script

echo "Installation complete. Run cortex-uninstall.sh to uninstall and cortex-updater.sh to update."
