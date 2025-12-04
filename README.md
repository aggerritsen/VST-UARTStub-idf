# XIAO ESP32-S3 UART Stub

This repository contains a minimal **UART stub firmware** for the **Seeed Studio XIAO ESP32-S3**, intended for testing UART communication with a T-SIM7080G-S3 or similar master device.  
The stub receives bytes over UART and returns a structured ACK response, making it ideal for protocol and hardware validation.

---

## 🔌 Pinout

Below is the pinout reference for the **Seeed XIAO ESP32-S3**.  
Place the following file in the `docs/` folder of your repository:

```
docs/Seeed-XIAO-ESP32-S3-Pinout.jpg
```

And embed it in this README like this:

![XIAO ESP32-S3 Pinout](docs/Seeed-XIAO-ESP32-S3-Pinout.jpg)

---

## 🛠 UART Wiring (Stub)

| XIAO Pin | Function | Notes |
|----------|----------|--------|
| **GPIO1 (D0)** | UART TX | Sends ACK responses |
| **GPIO2 (D1)** | UART RX | Receives master messages |
| **GND** | Ground | Must be shared with master device |

No VCC is exchanged — both boards should be powered from USB independently.

---

## 🚀 Features

- Configurable UART at **115200 baud, 8N1**
- Echoes back received frames in structured format:
  - Hex dump
  - ASCII-safe representation
- Flush-safe RX handling
- USB serial console remains active for debugging
- Suitable for replacement later with VisionAI → SIM UART pathway

---

## 📦 Folder Structure

```
/main
  ├── main.cpp        # UART stub logic
/docs
  ├── Seeed-XIAO-ESP32-S3-Pinout.jpg
README.md
```

---

## ▶️ Build & Flash

```bash
idf.py build flash monitor
```

---

## 📝 Example Output

On the XIAO USB console:

```
XIAO_STUB: RX (7): 'PING 1'
XIAO_STUB: TX ACK (14 bytes)
```

---

## 📄 License

GNU General Public License  
© Vespa Smart Trap

