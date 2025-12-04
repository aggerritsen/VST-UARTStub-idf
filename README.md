# XIAO ESP32-S3 UART Stub

This repository contains a minimal **UART communication test firmware** for the **Seeed Studio XIAO ESP32-S3**.

The firmware can operate in two modes:

- **Sender Mode** — periodically transmits `PING <n>` messages.  
- **Receiver Mode** — waits for UART data and responds with `ACK:<payload>`.

Mode selection is controlled at compile time via the flag:

```cpp
#define XIAO_UART_IS_SENDER  1   // 1 = sender, 0 = receiver
```

---

## 🔁 Sender / Receiver Behavior

### **Sender Mode (`XIAO_UART_IS_SENDER 1`)**
- Sends `PING <counter>` every 500 ms.
- Listens briefly for any UART replies.
- Useful when verifying downstream UART devices or link integrity.

### **Receiver Mode (`XIAO_UART_IS_SENDER 0`)**
- Listens continuously for incoming data.
- Sends back `ACK:<message>` upon receiving a payload.
- Useful when simulating a peripheral or testing upstream communication.

USB serial output remains active in both modes for debugging.

---

## 🔌 Pinout

Place the XIAO ESP32-S3 pinout image in:

```
docs/Seeed-XIAO-ESP32-S3-Pinout.jpg
```

Reference in README:

![XIAO ESP32-S3 Pinout](docs/Seeed-XIAO-ESP32-S3-Pinout.jpg)

This README reflects the **hardware and system behavior** of the Seeed Studio **XIAO ESP32-S3**, including the critical fact that:

### **D6/D7 (GPIO43/44) are physically exposed but *not* free for UART use by default**,  
because they are occupied by the **USB-CDC / Serial Console** of the ESP32-S3.

This clarifies the electrical design and prevents conflicts when integrating modules such as Groove Vision AI.

---

## 🧩 Bus Overview (Correct & Safe)

| Bus | Pins | Status | Notes |
|------|------|--------|--------|
| **USB-CDC Serial Console** | GPIO43 (D6), GPIO44 (D7) | **IN USE** | Default UART0 signals mapped to USB-CDC. Cannot be used for external UART unless console is moved or disabled. |
| **I²C Bus** | D4 (GPIO5) SDA, D5 (GPIO6) SCL | **Free** | Used by Groove Vision AI or other I²C modules. |
| **UART1 (Suggested App UART)** | D0 (GPIO1) TX, D1 (GPIO2) RX | **Free** | Safe UART pins already used in development stub. |
| **USB-C Native** | — | **Console, flashing** | Independent of UART0 pins. |

---

## ⚠️ Critical Clarification About D6/D7 (GPIO43/44)

### ✔ These pins *are physically exposed*  
### ✖ But **they are not available as general-purpose UART pins** in the default XIAO ESP32-S3 firmware

This is because:

- ESP-IDF config for the board maps the console to **UART0 → GPIO43/44**
- USB-CDC also uses UART0 signals internally
- Therefore **connecting another UART device to D6/D7 will conflict with console operation**

### You may use D6/D7 for UART *only if*:

1. You disable UART0 console entirely, **and**
2. You switch the console to **USB_SERIAL_JTAG** or **None**, **and**
3. You reassign UART0 or UART1 manually to GPIO43/44

This must be intentional and documented because it affects:

- Flashing
- Bootloader output
- Runtime logs
- Debugging access

---

## ✅ Safe & Recommended UART for Application Logic

For all custom firmware, use:

| Pin | GPIO | Function | Status |
|------|-------|-----------|---------|
| **D0** | GPIO1 | UART1 TX | Available |
| **D1** | GPIO2 | UART1 RX | Available |

These pins:

- Do **not** interfere with I²C
- Do **not** conflict with USB-CDC
- Are electrically stable for external devices
- Are already used in your stub firmware

---

## 🔌 I²C Pins (Groove Vision AI)

These are dedicated and conflict-free:

| XIAO Pin | GPIO | Function |
|----------|--------|----------|
| **D4**   | GPIO5 | I²C SDA |
| **D5**   | GPIO6 | I²C SCL |

Groove Vision AI uses these pins natively.

---

## 📘 Recommended Design Strategy

| Bus | Recommended Pins | Why |
|------|------------------|------|
| **I²C → Vision AI** | D4/D5 | Standard, conflict-free |
| **UART → SIM or Vision AI** | D0/D1 | Safe, supported, no console conflict |
| **Console** | USB-CDC | Leaves GPIO pins untouched |

If you later require UART on D6/D7, I can generate the exact ESP-IDF configuration changes needed to free those pins.

---

## 🚀 Features

- Simple UART sender/receiver testbench
- Compile-time mode switching
- Clean framing (`PING`, `ACK:<payload>`)
- Safe RX handling with timeouts
- USB serial console always active

---

## 📦 Folder Structure

```
/main
  ├── main.cpp
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

### Receiver Mode

```
XIAO_UART: RX (7 bytes): 'PING 1'
XIAO_UART: TX ACK (12 bytes)
```

### Sender Mode

```
XIAO_UART: >> Sent (8 bytes): 'PING 4'
XIAO_UART: << Received (12 bytes): 'ACK:PING 4'
```

---

## 📄 License

GNU General Public License  
© Vespa Smart Trap
