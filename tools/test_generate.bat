@echo off
echo Testing certificate generation in ESP-IDF environment...
cd /D C:\workspace\ESP32_Projects\distance
call C:\workspace\ESP32_Projects\esp\v5.4.1\esp-idf\export.bat
python tools\generate_cert.py
echo.
echo Checking generated files:
if exist server_cert.pem (
    echo ✅ server_cert.pem created
    echo Certificate info:
    type server_cert.pem | findstr "BEGIN\|END"
) else (
    echo ❌ server_cert.pem not found
)
if exist server_key.pem (
    echo ✅ server_key.pem created
    echo Key info:
    type server_key.pem | findstr "BEGIN\|END"
) else (
    echo ❌ server_key.pem not found
)
pause