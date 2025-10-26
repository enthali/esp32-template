#!/bin/bash
# VNC Setup Script for ESP32 Template
# This script configures VNC for GUI access in the development container

echo "Setting up VNC configuration..."

# Create VNC directory
mkdir -p ~/.vnc

# Create optimized xstartup script
cat > ~/.vnc/xstartup << 'EOF'
#!/bin/bash
# VNC Startup Script for ESP32 Development Environment
unset SESSION_MANAGER
unset DBUS_SESSION_BUS_ADDRESS
export XKL_XMODMAP_DISABLE=1

# Start dbus session if available
if which dbus-launch >/dev/null 2>&1; then
    eval `dbus-launch --sh-syntax --exit-with-session`
fi

# Start window manager
exec fluxbox
EOF

# Make startup script executable
chmod +x ~/.vnc/xstartup

echo "VNC configuration completed successfully!"
echo "To start VNC server: vncserver :1 -geometry 1024x768 -depth 24"
echo "To access GUI: Use NoVNC on port 6080 or set DISPLAY=:1"