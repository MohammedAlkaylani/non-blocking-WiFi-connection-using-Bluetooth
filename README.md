# non-blocking-WiFi-connection-using-Bluetooth
ESP32 WiFi Connection Manager with Bluetooth Provisioning This project implements a robust WiFi connection manager for ESP32 devices, featuring Bluetooth provisioning for initial setup and persistent storage of credentials. It handles connection timeouts, automatic reconnection, and includes a reset button to clear stored credentials.

# ESP32 WiFi Connection Manager with Bluetooth Provisioning

![ESP32 WiFi-Bluetooth Manager](https://img.shields.io/badge/Platform-ESP32-blue) ![License-MIT](https://img.shields.io/badge/License-MIT-green)

A robust WiFi connection manager for ESP32 that supports:
- **Bluetooth provisioning** for initial WiFi setup.
- **Persistent storage** of credentials using Preferences.
- **Auto-reconnection** and timeout handling.
- **Hardware reset button** to clear stored credentials.

---

## 📌 Features
- **Bluetooth LE Provisioning**: Configure WiFi via a mobile app (send SSID/password as JSON).
- **Persistent Storage**: Saves credentials to survive reboots.
- **Connection Resilience**: Handles timeouts, retries, and reconnection logic.
- **Debounced Hardware Reset**: Clear credentials with a button press.

---

## 🛠 Hardware Setup
1. **ESP32 Board** (Tested on ESP32-DevKitC).
2. **Tactile Button** connected to `GPIO 16` (pull-up enabled).

---

## 📝 Code Overview
### Key Components:
- **`Preferences`**: Stores/retrieves WiFi credentials.
- **`BluetoothSerial`**: Receives new credentials via Bluetooth.
- **Finite State Machine (FSM)**: Manages WiFi connection states.

### States:
- `START_LOGIN`: Loads saved credentials or starts Bluetooth.
- `OPEN_BLUETOOTH`: Waits for new credentials via BT.
- `NEW/OLD_CONNECTING`: Handles connection attempts.
- `LOOP`: Monitors active connection.

---

## 🔧 Usage
1. **First Boot**: If no credentials exist, ESP32 enters Bluetooth mode (`MY ESP`).
2. **Provisioning**: Send a JSON string via BT:
   ```json
   {"ssid":"YourSSID", "password":"YourPassword"}
3. Reset: Press the button to clear credentials and restart provisioning.

📄 Dependencies
 . ArduinoJSON (v6+)
 . ESP32 Arduino Core

⚡ Output Examples
 . Successful Connection:

    WiFi Connected!
    IP Address: 192.168.1.100
 . Bluetooth Mode:

    No WiFi credentials found.
    Bluetooth started.
