#!/bin/bash
# on-start.sh

# This script ensures necessary permissions for user:esp operation in the development container
sudo chmod 1777 /tmp

# Start network stack (TUN bridge and HTTP proxy) for QEMU
echo ""
echo "Setting up network stack for QEMU..."
cd /workspaces/esp32-template
bash ./tools/network/ensure-network-stack.sh

# Start code-server ONLY if NOT in GitHub Codespaces (for local development)
if [ -z "$CODESPACES" ]; then
    echo ""
    echo "Starting code-server for browser-based access..."
    echo "Access VS Code in browser at: http://localhost:8088"
    nohup code-server \
        --bind-addr 0.0.0.0:8088 \
        --auth none \
        --disable-telemetry \
        /workspaces/esp32-template \
        > /tmp/code-server.log 2>&1 &
    echo "code-server started (PID: $!)"
else
    echo ""
    echo "Running in GitHub Codespaces - code-server not needed"
fi

# Write timestamp to workspace root
echo "on-start.sh executed at $(date)" > .on-start-ran
