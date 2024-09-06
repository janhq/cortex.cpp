#!/bin/bash

# required root privileges
if [ "$EUID" -ne 0 ]
  then echo "Please run as root with sudo"
  exit
fi

rm /usr/local/bin/cortex

echo "Do you want to delete the 'cortex' data folder for all users? (yes/no)"
read -r answer

case "$answer" in
    [yY][eE][sS]|[yY])
        echo "Deleting 'cortex' data folders..."
        for userdir in /Users/*; do
            if [ -d "$userdir/cortex" ]; then
                echo "Removing $userdir/cortex"
                rm -rf "$userdir/cortex" > /dev/null 2>&1
            fi
        done
        ;;
    [nN][oO]|[nN])
        echo "Keeping the 'cortex' data folders."
        ;;
    *)
        echo "Invalid response. Please type 'yes' or 'no'."
        ;;
esac

rm /usr/local/bin/cortex-uninstall.sh