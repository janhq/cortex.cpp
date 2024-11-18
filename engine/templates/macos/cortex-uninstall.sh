#!/usr/bin/env sh
DESTINATION_BINARY_NAME=cortex
DATA_FOLDER_NAME=.cortex
CONFIGURATION_FILE_NAME=.cortexrc
UNINSTALLER_FILE_NAME=cortex-uninstall.sh
DESTINATION_BINARY_SERVER_NAME=cortex-server

# required root privileges
if [ "$EUID" -ne 0 ]
  then echo "Please run as root with sudo"
  exit
fi

USER_TO_RUN_AS=${SUDO_USER:-$(whoami)}

# Use /root if user is root, otherwise /Users/<username>
if [ "$USER_TO_RUN_AS" = "root" ]; then
    USER_HOME="/root"
else
    USER_HOME="/Users/$USER_TO_RUN_AS"
fi

sudo -u "$USER_TO_RUN_AS" /usr/local/bin/$DESTINATION_BINARY_NAME stop > /dev/null 2>&1
rm /usr/local/bin/$DESTINATION_BINARY_NAME
rm /usr/local/bin/$DESTINATION_BINARY_SERVER_NAME

echo "Do you want to delete the '$USER_HOME/$DATA_FOLDER_NAME' data folder and file '$USER_HOME/$CONFIGURATION_FILE_NAME'? (y/n) [default: n]"
read -r answer

# Default to 'no' if no input is provided
while true; do
    case "$answer" in
        [yY][eE][sS]|[yY])
            echo "Deleting cortex data folders..."
            if [ -d "$USER_HOME/$DATA_FOLDER_NAME" ]; then
                echo "Removing $USER_HOME/$DATA_FOLDER_NAME"
                rm -rf "$USER_HOME/$DATA_FOLDER_NAME" > /dev/null 2>&1
            fi
            if [ -f "$USER_HOME/$CONFIGURATION_FILE_NAME" ]; then
                echo "Removing $USER_HOME/$CONFIGURATION_FILE_NAME"
                rm -f "$USER_HOME/$CONFIGURATION_FILE_NAME" > /dev/null 2>&1
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

rm /usr/local/bin/$UNINSTALLER_FILE_NAME
