#!/bin/bash
# on-attach.sh

# This script runs every time you attach to the container

# Write timestamp to workspace root
cd /workspaces/esp32-template
echo "on-attach.sh executed at $(date)" > .on-attach-ran