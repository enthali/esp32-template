#!bash
# on-start.sh

# VNC Startup Script for ESP32 Template
# This script ensures necessary permissions for VNC operation in the development container
sudo chmod 1777 /tmp

date > .on-start-ran
