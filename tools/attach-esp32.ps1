# ESP32 USB Device Attachment Script for Dev Container
# Run this script as Administrator when you need to connect ESP32 to the dev container

# Set error action to continue so we can handle errors gracefully
$ErrorActionPreference = "Continue"

$logFile = "$PSScriptRoot/attach-esp32.log"
Start-Transcript -Path $logFile -Append -Force

try {
    Write-Host "ESP32 Debug Tool (Attach Script)" -ForegroundColor Green
    Write-Host "==============================" -ForegroundColor Green
    Write-Host ""

    # Check admin privileges
    $isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")
    Write-Host "Administrator privileges: $isAdmin" -ForegroundColor $(if($isAdmin){"Green"}else{"Red"})

    # Check usbipd
    try {
        $usbipd = Get-Command usbipd -ErrorAction Stop
        Write-Host "usbipd found at: $($usbipd.Source)" -ForegroundColor Green
    } catch {
        Write-Host "usbipd NOT found" -ForegroundColor Red
    }

    # Try to run usbipd list
    Write-Host ""
    Write-Host "Running 'usbipd list'..." -ForegroundColor Cyan
    try {
        $output = usbipd list 2>&1
        Write-Host "Success! Output:" -ForegroundColor Green
        Write-Host $output
    } catch {
        Write-Host "Failed to run usbipd list:" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
    }

    # Look for ESP32 patterns
    Write-Host ""
    Write-Host "Looking for ESP32 patterns in output..." -ForegroundColor Cyan
    $esp32Patterns = @("1a86:7523", "10c4:ea60", "0403:6001", "1a86:55d4")
    foreach ($pattern in $esp32Patterns) {
        if ($output -match $pattern) {
            Write-Host "Found pattern $pattern in output!" -ForegroundColor Green
        } else {
            Write-Host "Pattern $pattern not found" -ForegroundColor Yellow
        }
    }

    # Extend the script to share and attach the identified ESP32 USB device
    Write-Host ""
    Write-Host "Processing ESP32 device for WSL2 attachment..." -ForegroundColor Cyan

    # Extract the Bus ID of the first matching device
    $busId = ($output -split "`r`n|`r|`n") | Where-Object { $_ -match "1a86:7523" } | ForEach-Object {
        if ($_ -match "^(\d+-\d+)\s+1a86:7523") {
            $matches[1]
        }
    } | Select-Object -First 1

    if (-not $busId) {
        Write-Host "ERROR: Failed to extract Bus ID for the ESP32 device." -ForegroundColor Red
        Write-Host "Press any key to exit..."
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        exit 1
    }

    Write-Host "Using device with Bus ID: $busId" -ForegroundColor Cyan

    # Check if the device is already shared
    if ($output -match "^$busId\s+1a86:7523.+?Shared") {
        Write-Host "✓ Device is already shared" -ForegroundColor Green
    } else {
        # Share device if not already shared
        Write-Host "Sharing device..." -ForegroundColor Cyan
        try {
            usbipd bind --busid $busId
            Write-Host "✓ Device shared successfully" -ForegroundColor Green
        } catch {
            Write-Host "ERROR: Failed to share device" -ForegroundColor Red
            Write-Host $_.Exception.Message -ForegroundColor Red
            Write-Host "Press any key to exit..."
            $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
            exit 1
        }
    }

    # Attach device to WSL2
    Write-Host "Attaching device to WSL2..." -ForegroundColor Cyan
    try {
        usbipd attach --wsl --busid $busId
        Write-Host "✓ Device attached to WSL2 successfully!" -ForegroundColor Green
    } catch {
        Write-Host "ERROR: Failed to attach device to WSL2" -ForegroundColor Red
        Write-Host $_.Exception.Message -ForegroundColor Red
        Write-Host "Make sure WSL2 is running and Docker Desktop is started." -ForegroundColor Yellow
        Write-Host "Press any key to exit..."
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        exit 1
    }

    Write-Host ""
    Write-Host "🎉 ESP32 is now available in the dev container!" -ForegroundColor Green
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

    # Stop transcript logging
    Stop-Transcript
} catch {
    Write-Host "" 
    Write-Host "FATAL ERROR occurred:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host "" 
    Write-Host "Stack trace:" -ForegroundColor Yellow
    Write-Host $_.ScriptStackTrace -ForegroundColor Yellow
    Write-Host "" 
    Write-Host "Error details have been logged to $logFile" -ForegroundColor Cyan
    Write-Host "Press any key to exit..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    Stop-Transcript
    exit 1
}
