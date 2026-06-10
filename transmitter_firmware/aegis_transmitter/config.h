/*
 * config.h — A.E.G.I.S. V2 Transmitter Configuration
 * Hand Node: ESP32-WROOM-32 DevKit V1
 */

#pragma once

// ── I2C Pins (custom routing for orthogonal perfboard layout) ─────────────────
// DO NOT change to defaults — physical wiring is fixed to these pins
#define IMU_SDA_PIN   22   // Yellow wire
#define IMU_SCL_PIN   21   // Pink/Green wire

// ── IMU Sampling ──────────────────────────────────────────────────────────────
#define SAMPLE_RATE_HZ        100     // 10ms intervals
#define SAMPLE_INTERVAL_MS    10      // 1000ms / 100Hz
#define WINDOW_SIZE           150     // 1.5 seconds of data at 100Hz

// ── CNN Inference ─────────────────────────────────────────────────────────────
#define CONFIDENCE_THRESHOLD  0.92f   // 92% minimum to fire gesture trigger
#define NUM_GESTURE_CLASSES   5

// Gesture class indices — must match Edge Impulse training label order
#define GESTURE_REST          0
#define GESTURE_SWIPE_LEFT    1
#define GESTURE_SWIPE_RIGHT   2
#define GESTURE_SWIPE_UP      3
#define GESTURE_SWIPE_DOWN    4

// ── ESP-NOW ───────────────────────────────────────────────────────────────────
// MAC address of ESP32-C3 receiver dongle
// Replace with your actual receiver MAC — read via receiver Serial monitor
uint8_t RECEIVER_MAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ── FreeRTOS Task Config ──────────────────────────────────────────────────────
#define TASK_IMU_CORE         0       // Core 0 — sensor polling only
#define TASK_INFER_CORE       1       // Core 1 — CNN inference only
#define TASK_IMU_PRIORITY     2
#define TASK_INFER_PRIORITY   1
#define TASK_STACK_SIZE       8192

// ── Status LED ────────────────────────────────────────────────────────────────
#define LED_PIN               2       // Onboard LED — blinks on gesture fire
