@echo off
echo Testing cryptography library in ESP-IDF environment...
cd /D C:\workspace\ESP32_Projects\distance
call C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat
python tools\test_crypto.py
pause