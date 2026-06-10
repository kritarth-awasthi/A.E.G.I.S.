/*
 * config_receiver.h — ESP32-C3 Receiver Configuration
 *
 * CRITICAL PIN RULES FOR ESP32-C3 SUPER MINI:
 *
 * Native USB pins — NEVER connect external sensors here:
 *   GPIO 18 (D-) and GPIO 19 (D+) → wired directly to USB-C port
 *
 * Strapping pins — NEVER use for switches or inputs:
 *   GPIO 2, 8, 9 → control boot mode
 *   GPIO 9 pulled LOW → forces Download Mode (boot loop)
 *
 * Safe GPIO for future expansion: 0, 1, 3, 4, 5, 6, 7, 10
 */

#pragma once

// Gesture class indices — must match transmitter config.h
#define GESTURE_REST          0
#define GESTURE_SWIPE_LEFT    1
#define GESTURE_SWIPE_RIGHT   2
#define GESTURE_SWIPE_UP      3
#define GESTURE_SWIPE_DOWN    4

// HID keystroke hold duration
#define HID_KEY_HOLD_MS       80    // ms — long enough to register, short enough to feel responsive

// Status LED (onboard on C3 Super Mini)
#define LED_PIN               8     // Safe to use as output on C3
