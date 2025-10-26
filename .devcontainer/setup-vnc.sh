#!/bin/bash
# VNC Setup Script for ESP32 Template
# This script configures VNC for GUI access in the development container

echo "Setting up VNC configuration..."

# Create VNC directory
mkdir -p ~/.vnc

# Create optimized xstartup script with XFCE4
cat > ~/.vnc/xstartup << 'EOF'
#!/bin/bash
# VNC Startup Script with XFCE4 Desktop Environment
unset SESSION_MANAGER
unset DBUS_SESSION_BUS_ADDRESS

# Start dbus session
if which dbus-launch >/dev/null 2>&1; then
    eval `dbus-launch --sh-syntax --exit-with-session`
fi

# Set German keyboard layout for X11 session
setxkbmap de

# Start XFCE4 desktop environment
exec startxfce4
EOF

# Make startup script executable
chmod +x ~/.vnc/xstartup

echo "VNC configuration completed successfully!"
echo "Keyboard layout: German (DE) - can be changed with 'setxkbmap [layout]'"
echo "To start VNC server: vncserver :1 -geometry 1024x768 -depth 24"
echo "To access GUI: Use NoVNC on port 6080 or set DISPLAY=:1"