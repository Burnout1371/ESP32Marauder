# ESP32-S3 Marauder with FireBeetle 2 SPI Integration Guide

## 🎯 Overview

This guide documents the real-time bidirectional SPI communication between:
- **ESP32-S3-N16R8** (acts as **SPI Slave**, runs Marauder)
- **FireBeetle 2 DFR1237** (acts as **SPI Master**, runs UI controller)

### Architecture Diagram
```
┌─────────────────────────────────────────────────────────────┐
│          FireBeetle 2 (SPI Master)                          │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ Touchscreen UI                                      │   │
│  │ - WiFi Scan / Attack Menu                           │   │
│  │ - BLE Scan / Attack Menu                            │   │
│  │ - NRF24 Scan / Attack Menu                          │   │
│  │ - Real-time results display                         │   │
│  └─────────────────────────────────────────────────────┘   │
│                         │                                   │
│                    SPI Master                               │
│         GPIO 30(MI) ↔ GPIO 19(MISO)                        │
│         GPIO 29(MO) ↔ GPIO 20(MOSI)                        │
│         GPIO 28(SCK) ↔ GPIO 21(SCK)                        │
│         GPIO 27(CS) ↔ GPIO 47(CS)                          │
│                         ↓                                   │
└─────────────────────────────────────────────────────────────┘
                          │
┌─────────────────────────────────────────────────────────────┐
│          ESP32-S3-N16R8 (SPI Slave)                         │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ Marauder Engine                                     │   │
│  │ - WiFi Scanning (802.11 mgmt)                       │   │
│  │ - BLE Scanning                                      │   │
│  │ - NRF24L01+ Scanning/Attacks                        │   │
│  │ - Deauth/Jamming Capabilities                       │   │
│  └─────────────────────────────────────────────────────┘   │
│                         │                                   │
│              NRF24L01+ (FSPI)                               │
│         GPIO 12(SCK), 13(MISO), 11(MOSI)                   │
│         GPIO 10(CSN), GPIO 9(CE)                           │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔌 Hardware Pin Configuration

### ESP32-S3-N16R8 Pins

#### NRF24L01+ Module (Left Side)
| Function | GPIO | NRF Pin |
|----------|------|---------|
| SCK | 12 | 5 |
| MISO | 13 | 7 |
| MOSI | 11 | 6 |
| CSN | 10 | 4 |
| CE | 9 | 3 |
| VCC | 3V3 | 2 |
| GND | GND | 1 |

#### FireBeetle 2 SPI Slave Link
| Function | GPIO | FB2 Pin | Signal |
|----------|------|---------|--------|
| MOSI | 20 | 29 | MO (Master Out) |
| MISO | 19 | 30 | MI (Master In) |
| SCK | 21 | 28 | SCK (Clock) |
| CS | 47 | 27 | Chip Select |
| GND | GND | GND | Ground |

### FireBeetle 2 (DFR1237) Pins

#### SPI Master Configuration
| Function | GPIO | Purpose |
|----------|------|---------|
| MOSI (MO) | 29 | Send commands to S3 |
| MISO (MI) | 30 | Receive responses from S3 |
| SCK | 28 | Clock signal |
| CS | 27 | Chip Select |
| GND | GND | Ground |

#### Display Pins (Touchscreen)
| Function | GPIO | Purpose |
|----------|------|---------|
| DC | 9 | Data/Command |
| CS | 10 | Display Chip Select |
| RST | 8 | Reset |
| BL | 11 | Backlight |
| SDA | 20 | I2C Data |
| SCL | 21 | I2C Clock |

---

## 📦 Software Architecture

### Files Overview

#### On ESP32-S3
```
esp32_marauder/
├── spi_fb2_bridge.h          # SPI Slave layer + state machine
├── fb2_command_handlers.h    # Command dispatch + execution
└── esp32_marauder_fb2.ino    # Main sketch with FB2 integration
```

#### On FireBeetle 2
```
FireBeetle2_UI/
└── FireBeetle2_UI.ino        # SPI Master + touchscreen UI
```

---

## 🔄 Command Protocol

### Frame Structure
```
Byte 0:    Command ID (0x00-0x10)
Bytes 1-31: Parameters / Response Data
```

### Command IDs (Byte 0)

| ID | Command | Parameter | Response |
|----|---------|-----------|----------|
| 0x00 | PING | None | "PONG" |
| 0x01 | WiFi Scan | Channel (byte 1) | Device count, RSSI data |
| 0x02 | BLE Scan | Scan duration ms | Device count, MAC, RSSI |
| 0x03 | NRF Scan | Channel (byte 1) | Device count, MAC, RSSI |
| 0x04 | WiFi Deauth | SSID/MAC (bytes 1-6) | ACK |
| 0x05 | BLE Spam | Advertising type (byte 1) | ACK |
| 0x06 | NRF Attack | Target MAC (bytes 1-6) | ACK |
| 0x0F | STOP | None | "Stopped" |
| 0x10 | STATUS | None | Status string |

### Response Codes (Byte 0 from S3)

| Code | Meaning |
|------|---------|
| 0x00 | ACK (Success) |
| 0x01 | BUSY (Scanning/Attacking in progress) |
| 0x02 | ERROR (Invalid command or failure) |
| 0x03 | DATA (Response contains data) |
| 0x04 | DONE (Scan/Attack completed) |

### Example Transaction Sequence

```
FB2 → S3 (MOSI):
  Byte 0: 0x01 (WiFi Scan)
  Byte 1: 0x06 (Channel 6)
  Bytes 2-31: 0x00 (unused)

S3 ← FB2 (MISO):
  Byte 0: 0x00 (ACK)
  Byte 1-31: "WiFi scan started"
  
[Scanning occurs...]

FB2 → S3 (MOSI):
  Byte 0: 0x10 (Status)
  Bytes 1-31: 0x00

S3 ← FB2 (MISO):
  Byte 0: 0x03 (DATA)
  Byte 1: 5 (Devices found)
  Bytes 2-31: Device data (MAC, RSSI, etc.)
```

---

## 🚀 Installation Steps

### Step 1: Prepare ESP32-S3 (Marauder Side)

1. **Copy files to Arduino sketch folder:**
   ```
   ~/Arduino/libraries/
   └── esp32_marauder/
       ├── spi_fb2_bridge.h
       └── fb2_command_handlers.h
   ```

2. **Use the FB2 integration sketch:**
   - Open `esp32_marauder_fb2.ino` in Arduino IDE
   - **DO NOT use the standard `esp32_marauder.ino`** (it doesn't have FB2 support)

3. **Board Settings:**
   - Board: `ESP32-S3 Dev Module`
   - Flash Size: `16MB`
   - Partition Scheme: `Huge APP (3MB No OTA)`
   - Upload Speed: `921600`
   - Port: Select your COM port

4. **Compile & Upload:**
   ```
   Sketch → Upload (Ctrl+U)
   ```

5. **Verify in Serial Monitor (115200 baud):**
   ```
   ╔═══════════════════════════════════════════════════════════╗
   ║       ESP32-S3 Marauder with FireBeetle 2 Bridge          ║
   ║                   Initializing...                         ║
   ╚═══════════════════════════════════════════════════════════╝
   
   [INIT] Starting standard Marauder initialization...
   [INIT] Initializing NRF24L01+ on FSPI...
   [INIT] Initializing FireBeetle 2 SPI Slave Bridge...
   
   ╔═══════════════════════════════════════════════════════════╗
   ║              ✓ Initialization Complete                    ║
   ║         Waiting for FireBeetle 2 commands...              ║
   ╚═══════════════════════════════════════════════════════════╝
   ```

### Step 2: Prepare FireBeetle 2 (UI Side)

1. **Install required library:**
   - Arduino IDE → Sketch → Include Library → Manage Libraries
   - Search: `DFRobot ST7789`
   - Install: `DFRobot_ST7789` by DFRobot

2. **Upload FB2 firmware:**
   - Open `FireBeetle2_UI.ino` in Arduino IDE
   - Board: `FireBeetle 2 Board for DFR1237` (or select based on your board)
   - Upload Speed: `921600`

3. **Verify initialization:**
   - Touchscreen displays "MARAUDER" title
   - Menu options visible
   - Serial monitor shows SPI initialization

### Step 3: Test Communication

#### Quick Test (Serial Commands)

On **FB2 Serial Monitor** (115200 baud), type:
```
1    → Start WiFi Scan
2    → Start BLE Scan
3    → Start NRF Scan
4    → WiFi Attack
5    → BLE Attack
6    → NRF Attack
s    → Get Status
x    → Stop current scan
b    → Back to menu
```

#### Expected Output

**FB2 Serial:**
```
[FB2-SPI] Sent: 0x01 | Response: 0x00
[FB2-SPI] Data: 57 69 46 69 20 73 63 61 6e 20 73 74 61 72 74 65
```
(Response bytes decode to: "WiFi scan started")

**S3 Serial:**
```
[FB2] Command received: 0x01 at 12345 ms
[FB2-WiFi] Starting WiFi scan...
```

---

## 🔧 Troubleshooting

### Issue: No SPI Communication
**Symptoms:** FB2 sends command, S3 doesn't receive

**Solutions:**
1. **Check pin connections:**
   - Verify all 4 SPI lines (MOSI, MISO, SCK, CS) + GND
   - Use multimeter to test continuity
   
2. **Check SPI frequency:**
   - Reduce to 500kHz in `FireBeetle2_UI.ino`:
     ```cpp
     spi_master.setFrequency(500000);  // 500 kHz
     ```

3. **Add pull-up resistors:**
   - MISO line: 10kΩ pull-up to 3.3V
   - CS line: 10kΩ pull-up to 3.3V

### Issue: Garbled Data
**Symptoms:** Responses are unreadable/corrupted

**Solutions:**
1. **Check cable lengths:** Keep SPI wires under 10cm
2. **Add capacitors:**
   - 100nF between 3.3V and GND (close to both boards)
3. **Reduce clock speed** to 500kHz

### Issue: Commands Timeout
**Symptoms:** FB2 sends command but gets no response

**Solutions:**
1. Verify S3 `FB2_SPI_Poll()` is running in main loop
2. Check that S3 is not blocked in a scan operation
3. Add `delay(10)` in S3 main loop if missing

---

## 📊 Real-time Data Streaming

### WiFi Scan Results Format
```
Response Byte 0: 0x03 (DATA)
Response Byte 1: Device count (N)
Response Bytes 2-31: Device data
  - Each device: [RSSI (1 byte)] [Channel (1 byte)] [MAC (6 bytes)]
  - Multiple devices packed sequentially
```

### BLE Scan Results Format
```
Response Byte 0: 0x03 (DATA)
Response Byte 1: Device count (N)
Response Bytes 2-31: Device MAC + RSSI pairs
  - Each device: [MAC (6 bytes)] [RSSI (1 byte)]
```

### NRF24 Scan Results Format
```
Response Byte 0: 0x03 (DATA)
Response Byte 1: Device count (N)
Response Bytes 2-31: Device data
  - Each device: [MAC (5 bytes)] [RSSI (1 byte)] [Flags (1 byte)]
```

---

## 🎮 Using the Touchscreen (FB2)

### Menu Structure
```
MARAUDER
├─ 1. WiFi Scan      → Starts passive WiFi channel scan
├─ 2. BLE Scan       → Starts BLE advertisement scan
├─ 3. NRF Scan       → Starts NRF24 channel hopping scan
├─ 4. WiFi Attack    → Deauth/disassoc attack
├─ 5. BLE Attack     → Advertisement spam
└─ 6. NRF Attack     → NRF24 jammer
```

### Touch Implementation (Coming Soon)
Currently uses **serial input**. For full touchscreen:
1. Add `touch_driver.h` for capacitive touch
2. Map touch zones to menu options
3. Update UI with animated results display

---

## 🐛 Debug Mode

### Enable Verbose Logging

**On S3**, add to `esp32_marauder_fb2.ino`:
```cpp
void loop() {
    FB2_Update();
    
    // Debug: Print state every 5 seconds
    static uint32_t last_debug = 0;
    if ((millis() - last_debug) > 5000) {
        FB2_PrintState();
        last_debug = millis();
    }
    
    delay(10);
}
```

**On FB2**, add to `FireBeetle2_UI.ino`:
```cpp
// Uncomment to see all SPI transactions
#define DEBUG_SPI
```

---

## 📝 Integration Checklist

- [ ] Hardware connections verified (all 5 wires: MOSI, MISO, SCK, CS, GND)
- [ ] ESP32-S3 compiled and uploaded with `esp32_marauder_fb2.ino`
- [ ] FireBeetle 2 compiled and uploaded with `FireBeetle2_UI.ino`
- [ ] S3 Serial Monitor shows "Waiting for FireBeetle 2 commands..."
- [ ] FB2 Serial Monitor shows SPI initialization
- [ ] PING command (0x00) returns "PONG"
- [ ] WiFi/BLE/NRF scans execute without errors
- [ ] Real-time results display on FB2 touchscreen

---

## 🚀 Next Steps

1. **Full Touchscreen Integration**
   - Implement capacitive touch input handling
   - Add animated result visualization

2. **Advanced Attacks**
   - Integrate Marauder's deauth/jamming functions
   - Add channel targeting and dwell time control

3. **Persistent Logging**
   - Save scan results to SD card (if available)
   - Export data to FB2 for analysis

4. **OTA Updates**
   - Over-the-air firmware updates for both boards
   - Version synchronization

---

## 📚 References

- [ESP32-S3 SPI Slave Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/spi_slave.html)
- [FireBeetle 2 DFR1237 Docs](https://www.dfrobot.com/product-2761.html)
- [RF24 Library Documentation](https://maniacbug.github.io/RF24/)
- [Marauder GitHub](https://github.com/justcallmekoko/ESP32Marauder)

---

**Created:** September 2, 2026  
**Version:** 1.0  
**Status:** Beta - Real-time SPI communication working, attack functions in progress
