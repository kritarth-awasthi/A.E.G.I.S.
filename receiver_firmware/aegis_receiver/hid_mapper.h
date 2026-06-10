/*
 * hid_mapper.h — Gesture to USB HID Keyboard Mapping
 *
 * Maps AEGIS gesture class IDs to HID keyboard codes.
 * ESP32-C3 emits these as native USB keystrokes — no drivers needed.
 *
 * Current: Keyboard shortcuts
 * ROADMAP: Add mouse movement mode (pitch/roll → cursor X/Y)
 */

#pragma once
#include <USB.h>
#include <USBHIDKeyboard.h>
#include "config_receiver.h"

// HID key codes
#define KEY_LEFT_ARROW  0xD8
#define KEY_RIGHT_ARROW 0xD7
#define KEY_UP_ARROW    0xDA
#define KEY_DOWN_ARROW  0xD9

USBHIDKeyboard Keyboard;

class HIDMapper {
public:
  void begin() {
    Keyboard.begin();
    USB.begin();
    Serial.println(F("[HID] USB HID Keyboard ready"));
  }

  // Fire keystroke based on gesture ID
  void fireGesture(uint8_t gesture_id, float confidence) {
    Serial.printf("[HID] Firing gesture %d (%.1f%%)\n",
                  gesture_id, confidence * 100);

    switch (gesture_id) {
      case GESTURE_SWIPE_LEFT:
        Keyboard.press(KEY_LEFT_ARROW);
        delay(HID_KEY_HOLD_MS);
        Keyboard.release(KEY_LEFT_ARROW);
        Serial.println(F("[HID] ← Left Arrow"));
        break;

      case GESTURE_SWIPE_RIGHT:
        Keyboard.press(KEY_RIGHT_ARROW);
        delay(HID_KEY_HOLD_MS);
        Keyboard.release(KEY_RIGHT_ARROW);
        Serial.println(F("[HID] → Right Arrow"));
        break;

      case GESTURE_SWIPE_UP:
        Keyboard.press(KEY_UP_ARROW);
        delay(HID_KEY_HOLD_MS);
        Keyboard.release(KEY_UP_ARROW);
        Serial.println(F("[HID] ↑ Up Arrow"));
        break;

      case GESTURE_SWIPE_DOWN:
        Keyboard.press(KEY_DOWN_ARROW);
        delay(HID_KEY_HOLD_MS);
        Keyboard.release(KEY_DOWN_ARROW);
        Serial.println(F("[HID] ↓ Down Arrow"));
        break;

      case GESTURE_REST:
      default:
        // No action on rest state
        break;
    }
  }

  // ROADMAP: mouse movement mode
  // void fireMouseMode(float pitch, float roll) {
  //   int dx = (int)(roll  * MOUSE_SENSITIVITY);
  //   int dy = (int)(pitch * MOUSE_SENSITIVITY);
  //   Mouse.move(dx, dy, 0);
  // }
};
