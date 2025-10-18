# Known Limitations

Welcome, brave explorer! Every project has its dragons—here are ours. If you run into these, don’t panic: you’re not alone.

## 📱 Captive Portal: Not-So-Captive

- **What happens?** When you connect to the ESP32’s WiFi, your phone/tablet might not be automatically redirected to the configuration page. Instead, you’ll need to open your browser and type `192.168.4.1`.
- **Why?** Modern devices are picky about captive portals, and our ESP32's captive portal may not trigger an automatic redirect on all devices.
- **Will it be fixed?** Probably not. It’s a common IoT quirk, and most users figure it out quickly.

## 🦄 Other Oddities

- If you find a new quirk, let us know—we love a good bug hunt.

---

> Remember: If everything worked perfectly, it wouldn’t be called a “project.”
