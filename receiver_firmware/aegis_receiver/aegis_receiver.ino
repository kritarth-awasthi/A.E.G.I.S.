/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║               A . E . G . I . S .   V 2                     ║
 * ║                  Receiver — PC Dongle Node                   ║
 * ║                                                              ║
 * ║  Developer : Kritarth Awasthi | BIT Mesra, Jaipur           ║
 * ║  Hardware  : ESP32-C3 Super Mini (RISC-V)                   ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  ROLE: Wireless USB dongle. Plugs into PC USB-C port.
 *
 *  FLOW:
 *  [Glove ESP32-WROOM] → ESP-NOW <2ms → [This C3 dongle]
 *                                        → USB HID keystroke
 *                                        → Host PC (any OS)
 *
 *  NATIVE USB PINS (DO NOT USE FOR SENSORS):
 *  GPIO 18 (D-) and GPIO 19 (D+) → wired to USB-C connector
 *  These emit the actual HID data to the PC.
 *
 *  The host OS sees this as a standard keyboard — no drivers,
 *  no software, no pairing. Plug and play on Windows/macOS/Linux.
 *
 *  TO FIND YOUR RECEIVER MAC ADDRESS:
 *  Flash this sketch → open Serial Monitor → read the printed MAC.
 *  Paste that MAC into config.h RECEIVER_MAC[] on the transmitter.
 */

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <USB.h>
#include <USBHIDKeyboard.h>
#include "config_receiver.h"
#include "hid_mapper.h"

HIDMapper hid;

// Packet structure — must match transmitter espnow_tx.h
struct AEGISPacket {
  uint8_t  gesture_id;
  float    confidence;
  uint32_t timestamp_ms;
};

// ── ESP-NOW Receive Callback ──────────────────────────────────────────────────
void onDataReceived(const esp_now_recv_info_t* info,
                    const uint8_t* data, int len) {
  if (len != sizeof(AEGISPacket)) return;

  AEGISPacket packet;
  memcpy(&packet, data, sizeof(packet));

  Serial.printf("[RX] Gesture: %d | Confidence: %.1f%% | Latency from TX: ~%lums\n",
                packet.gesture_id,
                packet.confidence * 100,
                millis() - packet.timestamp_ms);

  // Flash LED on receive
  digitalWrite(LED_PIN, HIGH);
  hid.fireGesture(packet.gesture_id, packet.confidence);
  digitalWrite(LED_PIN, LOW);
}

// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println(F("\n[A.E.G.I.S.] Receiver booting..."));

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Print MAC address for transmitter pairing
  WiFi.mode(WIFI_STA);
  Serial.print(F("[INFO] Receiver MAC Address: "));
  Serial.println(WiFi.macAddress());
  Serial.println(F("       Paste this into RECEIVER_MAC[] in transmitter config.h"));

  // Initialise ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("[FATAL] ESP-NOW init failed"));
    while(1) delay(1000);
  }

  esp_now_register_recv_cb(onDataReceived);
  Serial.println(F("[ESP-NOW] Receiver listening..."));

  // Initialise USB HID
  hid.begin();

  // Boot LED flash — signals ready to PC
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH); delay(100);
    digitalWrite(LED_PIN, LOW);  delay(100);
  }

  Serial.println(F("[A.E.G.I.S.] Receiver ready — waiting for gestures"));
}

// =============================================================================
void loop() {
  delay(10);  // ESP-NOW callback handles all work
}
