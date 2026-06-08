# A.E.G.I.S.
### Articulated Electronic Gesture Inference System

> Wearable edge-AI human-computer interface controller.  
> float32 CNN inference on ESP32 · FreeRTOS dual-core · ESP-NOW · USB HID  
> **Status: Active Development**

---

## System Architecture
``` 
╔══════════════════════════════════════════════════════════════╗
║                    A.E.G.I.S. V2 ARCHITECTURE                ║
╚══════════════════════════════════════════════════════════════╝

┌─────────────────────────────┐      ┌──────────────────────────┐
│      HAND NODE              │      │     FOREARM NODE          │
│   (Logic & Sensing)         │      │     (Power Supply)        │
│                             │      │                           │
│  ┌─────────────────────┐    │      │  ┌────────────────────┐   │
│  │   ESP32-WROOM-32    │    │      │  │  18650 Li-Ion Cell │   │
│  │   Dual-Core 240MHz  │    │      │  │     2000 mAh       │   │
│  │                     │    │      │  └────────┬───────────┘   │
│  │  Core 0 ──────────► │    │      │           │               │
│  │  MPU6050 I2C @100Hz │    │      │  ┌────────▼───────────┐   │
│  │  SDA → GPIO 22      │    │      │  │   TP4056 PMIC      │   │
│  │  SCL → GPIO 21      │    │      │  │   USB-C Charging   │   │
│  │                     │    │      │  └────────┬───────────┘   │
│  │  Core 1 ──────────► │    │      │           │               │
│  │  float32 CNN        │    │      │  ┌────────▼───────────┐   │
│  │  150-frame window   │    │      │  │   SPDT Slide Switch│   │
│  │  98.2% accuracy     │    │      │  │   (OUT+ line only) │   │
│  └────────┬────────────┘    │      │  └────────┬───────────┘   │
│           │                 │      │           │               │
│  ┌────────▼────────────┐    │      │           │               │
│  │  MPU6050 6-DOF IMU  │    │      └───────────┼───────────────┘
│  │  Flat-mount (Z=1G)  │    │                  │
│  └─────────────────────┘    │                  │ VIN
└──────────┬──────────────────┘                  │
           │                          ◄───────────┘
           │ ESP-NOW (bare-metal)      Flexible wire tether
           │ Peer-to-peer MAC target   (Split Architecture)
           │ < 2ms latency
           │ No TCP/IP stack
           ▼
┌──────────────────────────────┐
│      PC RECEIVER NODE        │
│    ESP32-C3 Super Mini       │
│    RISC-V Single Core        │
│                              │
│  Internal Wi-Fi Antenna      │
│  ← receives ESP-NOW packet   │
│                              │
│  GPIO 18 (D-) ──────────┐   │
│  GPIO 19 (D+) ──────────┼──► USB-C → Host PC
│                          │   │
│  Native USB HID output   │   │  Windows / macOS / Linux
│  Keyboard + Mouse emul.  │   │  Zero drivers required
└──────────────────────────┘   │  Plug-and-play
                               │
                    ┌──────────▼────────────┐
                    │      HOST PC          │
                    │                       │
                    │  Sees AEGIS as a      │
                    │  standard HID device  │
                    │  14ms total latency   │
                    └───────────────────────┘

──────────────────────────────────────────────────────────────
 DATA FLOW
──────────────────────────────────────────────────────────────

 Hand movement
     │
     ▼
 MPU6050 captures X,Y,Z accel + gyro @ 100Hz (Core 0)
     │
     ▼
 150-frame sliding window buffer (1.5 seconds of data)
     │
     ▼
 DSP low-pass filter → removes tremor/noise
     │
     ▼
 float32 1D CNN inference (Core 1)
     │
     ▼
 Softmax confidence > 92% threshold
     │
     ▼
 ESP-NOW micro-packet → MAC address of C3 receiver
     │                   < 2ms
     ▼
 ESP32-C3 receives packet → injects USB HID interrupt
     │
     ▼
 Host PC registers keystroke/mouse click
     │
     Total: ~14ms end-to-end
``` 
## Why float32 instead of INT8

Standard TinyML uses INT8 quantisation to save SRAM. For this project, INT8 was deliberately rejected.

When the MPU6050 reads a 12.2° tilt and a 12.8° tilt, INT8 rounds both to 12. For gesture recognition where subtle wrist micro-movements map to precise inputs, that precision loss causes false positive triggers.

float32 is 4x heavier on SRAM — offset entirely by FreeRTOS dual-core isolation. Core 0 and Core 1 never share compute, giving Core 1 100% headroom for float32 matrix multiplication.

## Performance Metrics

| Metric | Value |
|---|---|
| Validation Accuracy | 98.2% |
| Gesture Classes | 5 |
| Inference Time | 2ms |
| Peak RAM Usage | 1.6KB |
| ESP-NOW Latency | <2ms |
| Total Pipeline Latency | 14ms |
| Battery Life | 18+ hours |

## Hardware

| Component | Role |
|---|---|
| ESP32-WROOM-32 | Transmitter — sensor polling + CNN inference |
| MPU6050 6-DOF IMU | Kinetic telemetry (I2C: SDA→GPIO22, SCL→GPIO21) |
| ESP32-C3 Super Mini | Receiver — USB HID emulation |
| 18650 Li-Ion + TP4056 | Power — compression-fit harness, USB-C charging |

## Roadmap

- [x] FreeRTOS dual-core architecture
- [x] float32 CNN training on Edge Impulse
- [x] ESP-NOW wireless bridge
- [x] USB HID receiver
- [ ] V2.1 miniaturisation — XIAO ESP32S3
- [ ] Haptic feedback via LRAs
- [ ] Flex sensor finger articulation

---
*Lead Developer: Kritarth Awasthi | BIT Mesra, Jaipur*


     
