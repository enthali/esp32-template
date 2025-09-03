@echo off
REM ESP32 USB Attachment Helper - Double-click to run
REM This batch file runs the PowerShell script with Administrator privileges

echo ESP32 Dev Container USB Attachment
echo ===================================
echo.
echo This tool will attach your ESP32 to the dev container for flashing.
echo You will be prompted for Administrator privileges.
echo.

REM Run PowerShell script as Administrator with PowerShell 7 if available
where pwsh >nul 2>nul
if %ERRORLEVEL% == 0 (
    echo Using PowerShell 7...
    pwsh -Command "Start-Process pwsh -ArgumentList '-ExecutionPolicy Bypass -File ""%~dp0attach-esp32.ps1""' -Verb RunAs"
) else (
    echo PowerShell 7 not found, using Windows PowerShell...
    powershell -Command "Start-Process PowerShell -ArgumentList '-ExecutionPolicy Bypass -File ""%~dp0attach-esp32.ps1""' -Verb RunAs"
)
