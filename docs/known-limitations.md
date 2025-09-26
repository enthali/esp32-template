# Known Limitations & Quirks 🚧

Welcome, brave explorer! Every project has its dragons—here are ours. If you run into these, don’t panic: you’re not alone, and sometimes it’s just how the (micro)chips fall.

## 📱 Captive Portal: Not-So-Captive
- **What happens?** When you connect to the ESP32’s WiFi, your phone/tablet might not be magically whisked away to the configuration page. Instead, you’ll need to open your browser and type `192.168.4.1`.
- **Why?** Modern devices are picky about captive portals, and our ESP32 is doing its best. Some phones just won’t auto-redirect—blame the phone, not the project!
- **Will it be fixed?** Probably not. It’s a common IoT quirk, and most users figure it out quickly.

## 🖥️ Web UI in Emulator: No Show
- **What happens?** If you’re running the project in an emulator, the web interface might be unreachable.
- **Why?** Networking in emulators is tricky, and not all features are supported.
- **Will it be fixed?** Maybe! We might open a GitHub issue for this. If you have ideas, PRs are welcome.

## 🦄 Other Oddities
- If you find a new quirk, let us know! We love a good bug hunt (and a good laugh).

---

*Remember: If everything worked perfectly, it wouldn’t be called a “project.”*
