# A.E.G.I.S. V2
### Articulated Electronic Gesture Inference System

> Wearable edge-AI HCI controller. float32 CNN on ESP32 via FreeRTOS
> dual-core isolation. ESP-NOW <2ms. USB HID driverless output. 98.2% accuracy.

**Developer:** Kritarth Awasthi | BIT Mesra, Jaipur
**Status:** Active development

---

## System Architecture

```
┌─────────────────────────────┐     ┌──────────────────────────┐
│      HAND NODE               │     │     FOREARM NODE          │
│   ESP32-WROOM-32             │     │     (Power Supply)        │
│                              │     │                           │
│  Core 0: MPU6050 @ 100Hz    │     │  18650 Li-Ion 2000mAh    │
│  SDA→GPIO22 | SCL→GPIO21    │     │  TP4056 PMIC (USB-C)     │
│                              │     │  SPDT Switch (OUT+ only) │
│  Core 1: float32 CNN        │     │                           │
│  150-frame window            │     └────────────┬─────────────┘
│  92% confidence threshold    │                  │ VIN (flexible tether)
│                              │◄─────────────────┘
└──────────────┬───────────────┘
               │ ESP-NOW (bare-metal, no TCP/IP)
               │ Peer-to-peer MAC address
               │ < 2ms latency
               ▼
┌──────────────────────────────┐
│   PC RECEIVER NODE           │
│   ESP32-C3 Super Mini        │
│   GPIO18(D-) GPIO19(D+)→USB │
│   Native USB HID             │
│   Zero drivers · Any OS     │
└──────────────────────────────┘
```

## Gesture → Keyboard Mapping

| Gesture | HID Output | Future |
|---|---|---|
| Swipe Left | ← Arrow Key | Mouse left |
| Swipe Right | → Arrow Key | Mouse right |
| Swipe Up | ↑ Arrow Key | Scroll up |
| Swipe Down | ↓ Arrow Key | Scroll down |
| Rest | No action | — |

## Performance

| Metric | Value |
|---|---|
| Validation Accuracy | 98.2% |
| Gesture Classes | 5 |
| Inference Time | 2ms |
| Peak RAM Usage | 1.6KB |
| ESP-NOW Latency | <2ms |
| Total Pipeline | ~14ms |
| Battery Life | 18+ hours |

## Why float32, Not INT8

INT8 quantisation rounds 12.2° and 12.8° wrist tilt to both 12.
For a gesture controller mapping micro-movements to precise inputs,
that aliasing causes false positive triggers. float32 preserves
decimal precision. Memory cost offset by FreeRTOS core isolation —
Core 0 handles sensor polling exclusively, Core 1 handles CNN
inference exclusively. Neither ever blocks the other.

## Repository Structure

```
A.E.G.I.S./
├── transmitter_firmware/    ESP32-WROOM hand node
│   └── aegis_transmitter/
│       ├── aegis_transmitter.ino
│       ├── config.h
│       ├── imu_handler.h
│       └── espnow_tx.h
├── receiver_firmware/       ESP32-C3 USB dongle
│   └── aegis_receiver/
│       ├── aegis_receiver.ino
│       ├── hid_mapper.h
│       └── config_receiver.h
├── ml_pipeline/             Data collection + training guide
│   ├── collect_data.py
│   └── README.md
└── docs/
    └── system_architecture.md
```

> **Note:** The trained float32 CNN model (`aegis_model.h`) is not
> included in this public repository. See `ml_pipeline/README.md`
> to train your own model via Edge Impulse.

## Setup

**Transmitter:**
1. Install: `MPU6050 by Electronic Cats`, `ESP32 Arduino`
2. Edit `config.h` → set `RECEIVER_MAC[]` from receiver Serial output
3. Board: `ESP32 Dev Module` → Flash

**Receiver:**
1. Open receiver sketch → Flash → note MAC address from Serial
2. Paste MAC into transmitter `config.h`
3. Board: `ESP32C3 Dev Module` with `USB CDC On Boot: Enabled`

## Roadmap

- [x] FreeRTOS dual-core architecture
- [x] float32 CNN inference pipeline
- [x] ESP-NOW wireless bridge
- [x] USB HID keyboard output
- [ ] V2.1 — XIAO ESP32S3 miniaturisation
- [ ] Mouse movement mode (pitch/roll → cursor)
- [ ] Haptic feedback via LRAs
- [ ] Flex sensor finger articulation
