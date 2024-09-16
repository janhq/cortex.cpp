#!/usr/bin/env sh
DESTINATION_BINARY_NAME=cortex
DATA_FOLDER_NAME=.cortex
CONFIGURATION_FILE_NAME=.cortexrc
UNINSTALLER_FILE_NAME=cortex-uninstall.sh

# required root privileges
if [ "$EUID" -ne 0 ]
  then echo "Please run as root with sudo"
  exit
fi

USER_TO_RUN_AS=${SUDO_USER:-$(whoami)}

rm /usr/local/bin/$DESTINATION_BINARY_NAME

echo "Do you want to delete the '~/$DATA_FOLDER_NAME' data folder and file '~/$CONFIGURATION_FILE_NAME'? (yes/no)"
read -r answer

case "$answer" in
    [yY][eE][sS]|[yY])
        echo "Deleting cortex data folders..."
        if [ -d "/Users/$USER_TO_RUN_AS/$DATA_FOLDER_NAME" ]; then
            echo "Removing /Users/$USER_TO_RUN_AS/$DATA_FOLDER_NAME"
            rm -rf "/Users/$USER_TO_RUN_AS/$DATA_FOLDER_NAME" > /dev/null 2>&1
        fi
        if [ -f "/Users/$USER_TO_RUN_AS/$CONFIGURATION_FILE_NAME" ]; then
            echo "Removing /Users/$USER_TO_RUN_AS/$CONFIGURATION_FILE_NAME"
            rm -f "/Users/$USER_TO_RUN_AS/$CONFIGURATION_FILE_NAME" > /dev/null 2>&1
        fi
        ;;
    [nN][oO]|[nN])
        echo "Keeping the 'cortex' data folders."
        ;;
    *)
        echo "Invalid response. Please type 'yes' or 'no'."
        ;;
esac

rm /usr/local/bin/$UNINSTALLER_FILE_NAME