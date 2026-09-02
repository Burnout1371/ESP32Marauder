# Hardware Pinout & Wiring Reference

## Quick Pin Reference Card

### ESP32-S3-N16R8 Pinout

```
                    ┌─────────────────────┐
                    │   ESP32-S3-N16R8    │
                    │    (Top View)       │
                    └─────────────────────┘

    GND  3V3  IO46  IO45  IO44  IO43  IO42  IO41  IO40  IO39  IO38  IO37  IO36  IO35  IO34  IO33  IO32  IO31  IO30  IO29  IO28  IO27

    GND  IO1   IO2   IO3   IO4   IO5   IO6   IO7   IO8   IO9   IO10  IO11  IO12  IO13  IO14  IO15  IO16  IO17  IO18  IO19  IO20  IO21

    GND  3V3   5V    GND   GND

╔════════════════════════════════════════════════════════════════╗
║                     SPI SLAVE (FB2 Link)                       ║
╠════════════════════════════════════════════════════════════════╣
║  GPIO 20  │ MOSI (MO from FB2)  │ Expansion Board Pin (orange) ║
║  GPIO 19  │ MISO (MI to FB2)    │ Expansion Board Pin (yellow) ║
║  GPIO 21  │ SCK (Clock)         │ Expansion Board Pin (green)  ║
║  GPIO 47  │ CS (Chip Select)    │ Expansion Board Pin (blue)   ║
║  GND      │ Ground              │ Expansion Board Pin (black)  ║
╚════════════════════════════════════════════════════════════════╝

╔════════════════════════════════════════════════════════════════╗
║                  NRF24L01+ (FSPI Bus)                          ║
╠════════════════════════════════════════════════════════════════╣
║  GPIO 12  │ SCK (Clock)         │ NRF Pin 5 (yellow)           ║
║  GPIO 13  │ MISO (Data In)      │ NRF Pin 7 (blue)             ║
║  GPIO 11  │ MOSI (Data Out)     │ NRF Pin 6 (green)            ║
║  GPIO 10  │ CSN (Chip Select)   │ NRF Pin 4 (orange)           ║
║  GPIO 9   │ CE (Chip Enable)    │ NRF Pin 3 (brown)            ║
║  3V3      │ VCC                 │ NRF Pin 2 (red)              ║
║  GND      │ Ground              │ NRF Pin 1 (black)            ║
╚════════════════════════════════════════════════════════════════╝
```

---

## FireBeetle 2 (DFR1237) Pinout

```
                    ┌─────────────────────┐
                    │  FireBeetle 2       │
                    │   (Top View)        │
                    └─────────────────────┘

    GND  IO13  IO12  IO14  IO27  IO26  IO25  IO32  IO33  IO5   IO17  IO16  IO4   IO0   IO2   IO15  IO21  IO20  3V3   3V3

    GND  IO19  IO18  IO23  IO22  IO11  IO10  IO9   IO8   IO7   IO6   IO1   IO3   5V    GND

╔════════════════════════════════════════════════════════════════╗
║                  SPI MASTER (S3 Link)                          ║
╠════════════════════════════════════════════════════════════════╣
║  GPIO 29  │ MO (MOSI to S3)     │ To S3 GPIO 20 (MOSI)        ║
║  GPIO 30  │ MI (MISO from S3)   │ To S3 GPIO 19 (MISO)        ║
║  GPIO 28  │ SCK (Clock)         │ To S3 GPIO 21 (SCK)         ║
║  GPIO 27  │ CS (Chip Select)    │ To S3 GPIO 47 (CS)          ║
║  GND      │ Ground              │ To S3 GND                    ║
╚════════════════════════════════════════════════════════════════╝

╔════════════════════════════════════════════════════════════════╗
║                  Display (Touchscreen)                         ║
╠════════════════════════════════════════════════════════════════╣
║  GPIO 9   │ DC (Data/Command)                                  ║
║  GPIO 10  │ CS (Chip Select)                                   ║
║  GPIO 8   │ RST (Reset)                                        ║
║  GPIO 11  │ BL (Backlight)                                     ║
║  GPIO 20  │ SDA (I2C Data - Touch)                             ║
║  GPIO 21  │ SCL (I2C Clock - Touch)                            ║
║  5V       │ VCC (Power)                                        ║
║  GND      │ Ground                                             ║
╚════════════════════════════════════════════════════════════════╝
```

---

## Wiring Diagram (Text Version)

### SPI Connection (S3 ↔ FB2)

```
ESP32-S3-N16R8              FireBeetle 2 (DFR1237)
──────────────              ───────────────────────

GPIO 20 (MOSI) ──────────── GPIO 29 (MO)
GPIO 19 (MISO) ──────────── GPIO 30 (MI)
GPIO 21 (SCK)  ──────────── GPIO 28 (SCK)
GPIO 47 (CS)   ──────────── GPIO 27 (CS)
GND            ──────────── GND
```

**Cable Requirements:**
- 5× 20-30cm jumper wires (or short ribbon cable)
- Wire gauge: 22-24 AWG recommended
- Color coding (recommended):
  - Orange: MOSI
  - Yellow: MISO
  - Green: SCK
  - Blue: CS
  - Black: GND

---

### NRF24L01+ Module Wiring

```
NRF24L01+          ESP32-S3-N16R8
─────────          ──────────────

Pin 1 (GND) ────── GND
Pin 2 (VCC) ────── 3V3
Pin 3 (CE)  ────── GPIO 9
Pin 4 (CSN) ────── GPIO 10
Pin 5 (SCK) ────── GPIO 12
Pin 6 (MOSI) ───── GPIO 11
Pin 7 (MISO) ───── GPIO 13
Pin 8 (IRQ) ────── (Optional, not used)
```

**Recommended Component List:**
```
- NRF24L01+ module
- 1× 10µF capacitor (between VCC and GND on NRF)
- 1× 100nF capacitor (between VCC and GND on NRF)
- 6× 20cm jumper wires for NRF connection
```

---

## Complete Wiring Checklist

### Before Assembly
- [ ] ESP32-S3 board and FireBeetle 2 board available
- [ ] NRF24L01+ module with antenna
- [ ] 11× jumper wires (5 for S3↔FB2 SPI, 6 for NRF24)
- [ ] Capacitors for NRF24 (1× 10µF, 1× 100nF)
- [ ] USB cables for programming both boards

### Assembly Steps

**Step 1: Prepare NRF24L01+**
1. Solder capacitors to NRF module:
   - 10µF between pins 2(VCC) and 1(GND)
   - 100nF across the same pins
2. Allow solder to cool

**Step 2: Connect NRF24L01+ to S3**
```
Wire Color  │ NRF Pin  │ NRF Function │ S3 GPIO │ S3 Function
────────────┼──────────┼──────────────┼─────────┼─────────────
Black       │ 1        │ GND          │ GND     │ Ground
Red         │ 2        │ VCC          │ 3V3     │ 3.3V Power
Brown       │ 3        │ CE           │ 9       │ Chip Enable
Orange      │ 4        │ CSN          │ 10      │ Chip Select
Yellow      │ 5        │ SCK          │ 12      │ SPI Clock
Green       │ 6        │ MOSI         │ 11      │ SPI Data Out
Blue        │ 7        │ MISO         │ 13      │ SPI Data In
```

**Step 3: Connect FB2 to S3 (SPI Slave Link)**
```
Wire Color  │ FB2 GPIO │ FB2 Function │ S3 GPIO │ S3 Function
────────────┼──────────┼──────────────┼─────────┼─────────────
Black       │ GND      │ Ground       │ GND     │ Ground
Orange      │ 29       │ MO (MOSI)    │ 20      │ MOSI (In)
Yellow      │ 30       │ MI (MISO)    │ 19      │ MISO (Out)
Green       │ 28       │ SCK          │ 21      │ SCK
Blue        │ 27       │ CS           │ 47      │ Chip Select
```

**Step 4: Verify Connections**
```
Using a multimeter in continuity mode:

NRF24 → S3:
  ✓ NRF GND (pin 1) ↔ S3 GND
  ✓ NRF CSN (pin 4) ↔ S3 GPIO 10
  ✓ NRF CE (pin 3) ↔ S3 GPIO 9
  ✓ NRF SCK (pin 5) ↔ S3 GPIO 12
  ✓ NRF MOSI (pin 6) ↔ S3 GPIO 11
  ✓ NRF MISO (pin 7) ↔ S3 GPIO 13

FB2 → S3:
  ✓ FB2 GND ↔ S3 GND
  ✓ FB2 GPIO 29 ↔ S3 GPIO 20
  ✓ FB2 GPIO 30 ↔ S3 GPIO 19
  ✓ FB2 GPIO 28 ↔ S3 GPIO 21
  ✓ FB2 GPIO 27 ↔ S3 GPIO 47
```

---

## Signal Integrity Tips

### Cable Management
- Keep SPI lines together (under 10cm total)
- Route SPI wires away from power lines
- Use twisted pairs or ribbon cable if possible
- Add ferrite beads on NRF power line if noise issues occur

### Decoupling
```
NRF24L01+ (with expansion board):

    ┌─────────────────────┐
    │    NRF24L01+        │
    │    Pin 2 (VCC)      │
    └───────┬─────────────┘
            │
            ├──┤├── 100nF (close to module)
            │  │
            ├──┤├── 10µF (close to module)
            │  │
            └──┴───── GND
            
S3 Power (near GPIO):
    ┌───────────────┐
    │  S3 GPIO 20   │
    │  S3 GPIO 21   │
    └───────────────┘
            │
            ├──┤├── 100nF
            │  │
            └──┴───── GND
```

### Noise Reduction
1. **Capacitors:** 100nF near every IC power pin
2. **Ground plane:** Solder all GND wires to common point
3. **Shielding:** Wrap SPI lines in foil if EMI suspected
4. **Ferrite beads:** On NRF power line (recommended)

---

## Expansion Board Connections (Optional)

If using DFRobot expansion board with FB2:

```
Expansion Board Pin Map:
┌──────────────────────────────────────┐
│   FireBeetle 2 Expansion Board       │
├──────────────────────────────────────┤
│  Orange (MO/MOSI) ──→ FB2 GPIO 29   │
│  Yellow (MI/MISO) ──→ FB2 GPIO 30   │
│  Green (SCK)      ──→ FB2 GPIO 28   │
│  Blue (CS)        ──→ FB2 GPIO 27   │
│  Black (GND)      ──→ GND            │
└──────────────────────────────────────┘
```

---

## Troubleshooting Connection Issues

### Visual Inspection
```
Check for:
  ✓ Loose solder joints on NRF module
  ✓ Bent pins on connectors
  ✓ Broken jumper wire strands
  ✓ Cold solder connections (dull appearance)
  ✓ Shorts between adjacent pins
```

### Continuity Testing
```
Device A → Device B

Using Multimeter (Continuity mode):
1. Touch probe to pin on Device A
2. Touch probe to pin on Device B
3. Continuity beeper should sound
4. If no beep, check connection

Common issues:
  ✗ Wire not fully inserted
  ✗ Bent pin not making contact
  ✗ Solder bridge somewhere else
  ✗ Wrong GPIO pin selected
```

### Power Verification
```
Using Multimeter (Voltage mode):

NRF24L01+ Power:
  ✓ Between VCC (pin 2) and GND (pin 1): 3.0-3.6V
  
S3 Power:
  ✓ Between 3V3 and GND: 3.0-3.6V
  
FB2 Power:
  ✓ Between 5V and GND: 4.8-5.2V
```

---

## Quick Reference Card (Print This)

```
╔════════════════════════════════════════════════════════════╗
║          ESP32-S3 ↔ FireBeetle 2 Quick Wiring             ║
╠════════════════════════════════════════════════════════════╣
║  S3 GPIO 20 (MOSI) ←→ FB2 GPIO 29 (MO)     │ Orange       ║
║  S3 GPIO 19 (MISO) ←→ FB2 GPIO 30 (MI)     │ Yellow       ║
║  S3 GPIO 21 (SCK)  ←→ FB2 GPIO 28 (SCK)    │ Green        ║
║  S3 GPIO 47 (CS)   ←→ FB2 GPIO 27 (CS)     │ Blue         ║
║  S3 GND            ←→ FB2 GND               │ Black        ║
╠════════════════════════════════════════════════════════════╣
║          ESP32-S3 ↔ NRF24L01+ Quick Wiring                ║
╠════════════════════════════════════════════════════════════╣
║  S3 GPIO 12 (SCK)   → NRF Pin 5  │ Yellow                  ║
║  S3 GPIO 13 (MISO)  → NRF Pin 7  │ Blue                    ║
║  S3 GPIO 11 (MOSI)  → NRF Pin 6  │ Green                   ║
║  S3 GPIO 10 (CSN)   → NRF Pin 4  │ Orange                  ║
║  S3 GPIO 9 (CE)     → NRF Pin 3  │ Brown                   ║
║  S3 3V3              → NRF Pin 2  │ Red                     ║
║  S3 GND              → NRF Pin 1  │ Black                   ║
╚════════════════════════════════════════════════════════════╝

SPI Clock Speed:
  S3 Slave: 1 MHz (adjustable in spi_fb2_bridge.h)
  NRF24: 10 MHz (standard)

Recommended Wire Gauge: 22-24 AWG
Recommended Length: 10-30 cm per wire
```

---

## Component Sources

### Recommended Suppliers
- **NRF24L01+ modules:** AliExpress, SparkFun, Adafruit
- **FireBeetle 2:** DFRobot official store, AliExpress
- **ESP32-S3:** Espressif official distributors, AliExpress
- **Jumper wires:** Any electronics supplier
- **Capacitors:** Any electronics supplier (common values)

### Bill of Materials (BoM)
```
Item                          Qty   Approx Cost
─────────────────────────────────────────────────
ESP32-S3-N16R8                1     $15-20
FireBeetle 2 (DFR1237)        1     $25-35
NRF24L01+ module              1     $2-5
Jumper wires (pack)           1     $3-8
Capacitor 10µF                2     $0.50
Capacitor 100nF               3     $0.50
Ferrite beads                 2     $1
───────────────────────────────────────────────
Total (approximate):                $47-70
```

---

**Last Updated:** September 2, 2026  
**Version:** 1.0
