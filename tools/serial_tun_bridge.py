#!/usr/bin/env python3
"""
Serial-TUN Bridge for ESP32 QEMU IP Tunnel

Bridges IP packets between QEMU UART1 (TCP port 5556) and a Linux TUN device.
This provides full TCP/IP connectivity to the simulated ESP32.

Architecture:
- Reads IP frames from TCP socket (QEMU UART1)
- Writes frames to TUN device (Linux network stack)
- Bidirectional: TUN → Serial also works

Framing Protocol:
- [LENGTH:2 bytes big-endian][DATA:N bytes]
- Maximum frame size: 1500 bytes (MTU)

Usage:
    sudo ./serial_tun_bridge.py

Requirements:
    - Root privileges (for TUN device creation)
    - pytun or python-pytun package

Author: ESP32 Distance Project
Date: 2025
"""

import socket
import struct
import sys
import os
import select
import signal
import logging

# Setup logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Configuration
SERIAL_HOST = 'localhost'
SERIAL_PORT = 5556
TUN_NAME = 'tun0'
TUN_IP = '192.168.100.1'
TUN_NETMASK = '255.255.255.0'
ESP32_IP = '192.168.100.2'
MAX_FRAME_SIZE = 1500

# TUN device handling
try:
    from pytun import TunTapDevice, IFF_TUN, IFF_NO_PI
    HAVE_PYTUN = True
except ImportError:
    HAVE_PYTUN = False
    logger.warning("pytun not available, using manual TUN device creation")

class SerialTunBridge:
    def __init__(self):
        self.serial_sock = None
        self.tun = None
        self.running = False

    def create_tun_device_manual(self):
        """Create TUN device manually using system commands"""
        import subprocess
        
        logger.info(f"Creating TUN device {TUN_NAME} manually...")
        
        # Create TUN device
        ret = subprocess.call(['ip', 'tuntap', 'add', 'dev', TUN_NAME, 'mode', 'tun'])
        if ret != 0:
            logger.error(f"Failed to create TUN device (exit code {ret})")
            return None
        
        # Set IP address
        ret = subprocess.call(['ip', 'addr', 'add', f'{TUN_IP}/24', 'dev', TUN_NAME])
        if ret != 0:
            logger.warning(f"Failed to set IP address (may already exist)")
        
        # Bring interface up
        ret = subprocess.call(['ip', 'link', 'set', TUN_NAME, 'up'])
        if ret != 0:
            logger.error(f"Failed to bring up TUN device")
            subprocess.call(['ip', 'tuntap', 'del', 'dev', TUN_NAME, 'mode', 'tun'])
            return None
        
        logger.info(f"TUN device {TUN_NAME} created: {TUN_IP}/24")
        
        # Open TUN device for reading/writing
        try:
            tun_fd = os.open(f'/dev/net/tun', os.O_RDWR)
            return tun_fd
        except Exception as e:
            logger.error(f"Failed to open TUN device: {e}")
            subprocess.call(['ip', 'tuntap', 'del', 'dev', TUN_NAME, 'mode', 'tun'])
            return None

    def create_tun_device(self):
        """Create and configure TUN device"""
        if HAVE_PYTUN:
            try:
                self.tun = TunTapDevice(name=TUN_NAME, flags=IFF_TUN | IFF_NO_PI)
                self.tun.addr = TUN_IP
                self.tun.netmask = TUN_NETMASK
                self.tun.mtu = MAX_FRAME_SIZE
                self.tun.up()
                logger.info(f"TUN device {TUN_NAME} created: {TUN_IP}/{TUN_NETMASK}")
                return self.tun
            except Exception as e:
                logger.error(f"Failed to create TUN device with pytun: {e}")
                return None
        else:
            # Manual TUN creation
            tun_fd = self.create_tun_device_manual()
            if tun_fd is None:
                return None
            
            # Wrap in file object
            class TunDevice:
                def __init__(self, fd):
                    self.fd = fd
                    self.file = os.fdopen(fd, 'r+b', buffering=0)
                
                def read(self, size):
                    return os.read(self.fd, size)
                
                def write(self, data):
                    return os.write(self.fd, data)
                
                def fileno(self):
                    return self.fd
                
                def close(self):
                    os.close(self.fd)
            
            self.tun = TunDevice(tun_fd)
            return self.tun

    def connect_serial(self):
        """Connect to QEMU UART TCP socket"""
        logger.info(f"Connecting to QEMU UART at {SERIAL_HOST}:{SERIAL_PORT}...")
        
        max_retries = 10
        for attempt in range(max_retries):
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect((SERIAL_HOST, SERIAL_PORT))
                logger.info(f"Connected to QEMU UART")
                return sock
            except ConnectionRefusedError:
                if attempt < max_retries - 1:
                    logger.warning(f"Connection refused, retrying ({attempt+1}/{max_retries})...")
                    import time
                    time.sleep(1)
                else:
                    logger.error(f"Failed to connect after {max_retries} attempts")
                    return None
            except Exception as e:
                logger.error(f"Failed to connect to serial: {e}")
                return None

    def serial_to_tun(self):
        """Read frame from serial and write to TUN"""
        try:
            # Read frame length (2 bytes, big-endian)
            len_data = self.serial_sock.recv(2)
            if len(len_data) != 2:
                return False
            
            frame_len = struct.unpack('>H', len_data)[0]
            
            if frame_len == 0 or frame_len > MAX_FRAME_SIZE:
                logger.warning(f"Invalid frame length: {frame_len}")
                return True
            
            # Read frame data
            frame_data = b''
            while len(frame_data) < frame_len:
                chunk = self.serial_sock.recv(frame_len - len(frame_data))
                if not chunk:
                    return False
                frame_data += chunk
            
            logger.debug(f"Serial→TUN: {frame_len} bytes")
            
            # Write to TUN
            self.tun.write(frame_data)
            
            return True
            
        except Exception as e:
            logger.error(f"Error in serial_to_tun: {e}")
            return False

    def tun_to_serial(self):
        """Read frame from TUN and write to serial"""
        try:
            # Read from TUN (blocking)
            frame_data = self.tun.read(MAX_FRAME_SIZE)
            
            if not frame_data:
                return True
            
            frame_len = len(frame_data)
            logger.debug(f"TUN→Serial: {frame_len} bytes")
            
            # Send length header (big-endian)
            len_data = struct.pack('>H', frame_len)
            self.serial_sock.sendall(len_data)
            
            # Send frame data
            self.serial_sock.sendall(frame_data)
            
            return True
            
        except Exception as e:
            logger.error(f"Error in tun_to_serial: {e}")
            return False

    def cleanup(self):
        """Cleanup resources"""
        logger.info("Cleaning up...")
        
        self.running = False
        
        if self.serial_sock:
            self.serial_sock.close()
            self.serial_sock = None
        
        if self.tun:
            if HAVE_PYTUN:
                self.tun.down()
                self.tun.close()
            else:
                self.tun.close()
                import subprocess
                subprocess.call(['ip', 'tuntap', 'del', 'dev', TUN_NAME, 'mode', 'tun'])
            self.tun = None

    def run(self):
        """Main bridge loop"""
        # Setup signal handlers
        signal.signal(signal.SIGINT, lambda s, f: self.cleanup())
        signal.signal(signal.SIGTERM, lambda s, f: self.cleanup())
        
        # Create TUN device
        if not self.create_tun_device():
            logger.error("Failed to create TUN device")
            return 1
        
        # Connect to serial
        self.serial_sock = self.connect_serial()
        if not self.serial_sock:
            self.cleanup()
            return 1
        
        logger.info("Bridge active. Press Ctrl+C to stop.")
        logger.info(f"ESP32 should be accessible at {ESP32_IP}")
        
        self.running = True
        
        # Main loop
        while self.running:
            # Wait for data on either serial or TUN
            rlist = [self.serial_sock, self.tun]
            readable, _, _ = select.select(rlist, [], [], 1.0)
            
            for r in readable:
                if r == self.serial_sock:
                    if not self.serial_to_tun():
                        logger.error("Serial connection lost")
                        self.running = False
                        break
                
                elif r == self.tun:
                    if not self.tun_to_serial():
                        logger.error("TUN device error")
                        self.running = False
                        break
        
        self.cleanup()
        return 0

def main():
    # Check for root
    if os.geteuid() != 0:
        logger.error("This script must be run as root (for TUN device)")
        logger.info("Try: sudo ./serial_tun_bridge.py")
        return 1
    
    bridge = SerialTunBridge()
    return bridge.run()

if __name__ == '__main__':
    sys.exit(main())
