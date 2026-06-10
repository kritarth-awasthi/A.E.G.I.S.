/*
 * espnow_tx.h — ESP-NOW Transmitter Handler
 *
 * Bare-metal peer-to-peer protocol targeting receiver MAC directly.
 * Bypasses TCP/IP stack entirely — sub-2ms transmission latency.
 * DO NOT replace with Bluetooth — handshake overhead is 20-45ms.
 */

#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include "config.h"

// Packet sent to receiver — keep minimal for low latency
struct AEGISPacket {
  uint8_t gesture_id;       // 0-4 gesture class index
  float   confidence;       // Softmax confidence 0.0-1.0
  uint32_t timestamp_ms;    // millis() at trigger time
};

volatile bool tx_success = false;

void onDataSent(const uint8_t* mac, esp_now_send_status_t status) {
  tx_success = (status == ESP_NOW_SEND_SUCCESS);
}

class ESPNowTransmitter {
public:
  bool begin() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) {
      Serial.println(F("[ESP-NOW] Init failed"));
      return false;
    }

    esp_now_register_send_cb(onDataSent);

    // Register receiver peer
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, RECEIVER_MAC, 6);
    peer.channel = 0;
    peer.encrypt  = false;

    if (esp_now_add_peer(&peer) != ESP_OK) {
      Serial.println(F("[ESP-NOW] Peer registration failed — check RECEIVER_MAC"));
      return false;
    }

    Serial.println(F("[ESP-NOW] Transmitter ready"));
    return true;
  }

  bool sendGesture(uint8_t gesture_id, float confidence) {
    AEGISPacket packet;
    packet.gesture_id    = gesture_id;
    packet.confidence    = confidence;
    packet.timestamp_ms  = millis();

    esp_err_t result = esp_now_send(
      RECEIVER_MAC,
      (uint8_t*)&packet,
      sizeof(packet)
    );

    if (result == ESP_OK) {
      Serial.printf("[TX] Gesture %d (%.1f%%) → receiver\n",
                    gesture_id, confidence * 100);
      return true;
    }
    Serial.println(F("[TX] Send failed"));
    return false;
  }
};
