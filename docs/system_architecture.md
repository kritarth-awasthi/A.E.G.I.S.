# A.E.G.I.S. — System Architecture

## Hardware Wiring

```
ESP32-WROOM-32         MPU6050
GPIO 22 (SDA) ──────── SDA  (Yellow wire)
GPIO 21 (SCL) ──────── SCL  (Pink/Green wire)
3.3V          ──────── VCC
GND           ──────── GND
NOTE: Sensor mounted FLAT — Z-axis = 1G at rest (CNN baseline)

ESP32-WROOM-32         TP4056 PMIC
VIN           ──────── OUT+ (via SPDT switch)
GND           ──────── OUT-

TP4056                 18650 Cell
B+            ──────── Battery +
B-            ──────── Battery -

ESP32-C3 Super Mini    USB-C Port
GPIO 18 (D-)  ──────── D- (direct wire)
GPIO 19 (D+)  ──────── D+ (direct wire)
```

## FreeRTOS Core Isolation

```
Core 0 — imuTask (priority 2)
  └── vTaskDelayUntil 10ms → MPU6050 I2C read → circular buffer write.
  └── NEVER runs inference, NEVER sends ESP-NOW.

Core 1 — inferenceTask (priority 1)
  └── reads circular buffer → flat float32 array.
  └── runs CNN → softmax → confidence check.
  └── if confidence > 92% AND gesture != REST → set gesture_ready flag.
  └── main loop picks up flag → ESP-NOW send.
```

## ESP-NOW Packet

```cpp
struct AEGISPacket {
  uint8_t  gesture_id;      // 0-4
  float    confidence;      // 0.0-1.0
  uint32_t timestamp_ms;    // for latency measurement
};
// Total: 9 bytes — minimal overhead for sub-2ms transmission
```

## CNN Input Shape

```
150 frames × 6 axes = 900 float32 values.
Axes: ax, ay, az (accelerometer g) + gx, gy, gz (gyroscope deg/s).
Window: 1.5 seconds at 100Hz.
Sliding: new inference every 100ms (not every frame).
```

## ESP32-C3 Safe Pins Reference

```
NEVER use for inputs/switches:
  GPIO 2, 8, 9  → strapping pins (control boot mode).
  GPIO 18, 19   → native USB D-/D+ (HID output).

Safe for future expansion:
  GPIO 0, 1, 3, 4, 5, 6, 7, 10.
```

## Power Subsystem

```
18650 Li-Ion (3.7V, 2000mAh)
  → TP4056 PMIC (over-discharge protection + USB-C charging)
  → SPDT Slide Switch (in OUT+ line ONLY)
  → ESP32-WROOM VIN

Switch placement: between TP4056 OUT+ and ESP32 VIN,
This allows battery charging while logic board is OFF,
DO NOT place switch on B+ line — bypasses TP4056 protection

Runtime: 18+ hours continuous operation
```
