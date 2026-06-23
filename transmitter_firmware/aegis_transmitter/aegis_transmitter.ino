/*
 * ╔══════════════════════════════════════════════════════════════╗
 * ║               A . E . G . I . S .                            ║
 * ║        Articulated Electronic Gesture Inference System       ║
 * ║                 Transmitter — Hand Node                      ║
 * ║                                                              ║
 * ║  Developer : Kritarth Awasthi                                ║
 * ║  Hardware  : ESP32-WROOM-32 DevKit V1 (30-pin)               ║
 * ╚══════════════════════════════════════════════════════════════╝
 *
 *  CORE ARCHITECTURE:
 *  ┌──────────────────────────────────────────────────────┐
 *  │  Core 0 (imuTask)                                    │
 *  │  MPU6050 → I2C @ 100Hz → circular buffer             │
 *  │  SDA: GPIO22 | SCL: GPIO21 (custom routing)          │
 *  │                                                      │
 *  │  Core 1 (inferenceTask)                              │
 *  │  circular buffer → float32 CNN → confidence check    │
 *  │  → ESP-NOW packet → ESP32-C3 receiver dongle         │
 *  └──────────────────────────────────────────────────────┘
 *
 *  WHY float32 (NOT int8):
 *  INT8 quantisation rounds 12.2° and 12.8° wrist tilt to
 *  both 12 — aliased data causes false positives on subtle
 *  micro-movements. float32 preserves decimal precision.
 *  Memory cost offset entirely by FreeRTOS core isolation.
 *
 *  WHY ESP-NOW (NOT Bluetooth):
 *  BLE handshake overhead = 20-45ms. ESP-NOW bypasses the
 *  TCP/IP stack, targeting receiver MAC directly: <2ms.
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "config.h"
#include "imu_handler.h"
#include "espnow_tx.h"

// ── NOTE ON CNN MODEL 
// The trained float32 TFLite model is NOT included in this public repository.
// The model is compiled into a C++ header (aegis_model.h) via:
//   Edge Impulse → Deployment → Arduino Library → float32 version
// To train your own model: see ml_pipeline/collect_data.py + train_model.py
// Include aegis_model.h and update gesture_engine.h inference calls accordingly

// ── Global Objects 
IMUHandler       imu;
ESPNowTransmitter espnow;

// ── Shared inference trigger flags 
volatile uint8_t  last_gesture    = GESTURE_REST;
volatile float    last_confidence = 0.0f;
volatile bool     gesture_ready   = false;
portMUX_TYPE      gesture_mux     = portMUX_INITIALIZER_UNLOCKED;

// ── CNN Input Buffer (flat float32 array) 
// Shape: [WINDOW_SIZE × 6 axes] = [150 × 6] = 900 floats
static float cnn_input[WINDOW_SIZE * 6];

// CORE 0 — IMU TASK
// Polls MPU6050 at exactly 100Hz and fills circular buffer.
// No inference, no ESP-NOW — just sensor polling.
void imuTask(void* pvParameters) {
  Serial.println(F("[Core 0] IMU task started"));
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;) {
    imu.sampleAndStore();
    // Precise 10ms delay — vTaskDelayUntil guarantees timing accuracy
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
  }
}

// CORE 1 — INFERENCE TASK
// Reads 150-frame window, runs float32 CNN, fires ESP-NOW on confidence pass.
// No sensor I/O — 100% headroom for float32 matrix math.

void inferenceTask(void* pvParameters) {
  Serial.println(F("[Core 1] Inference task started"));

  // Brief delay to let IMU buffer fill first window
  vTaskDelay(pdMS_TO_TICKS(1600));

  for (;;) {
    if (imu.isWindowReady()) {
      // Copy current window into flat input array
      imu.getWindowFlat(cnn_input);

      // ── CNN INFERENCE 
      // Replace this block with your Edge Impulse model inference calls.
      // Pattern:
      //   signal_t signal;
      //   numpy::signal_from_buffer(cnn_input, WINDOW_SIZE * 6, &signal);
      //   ei_impulse_result_t result;
      //   run_classifier(&signal, &result, false);
      //   uint8_t  top_class = argmax(result.classification);
      //   float    top_conf  = result.classification[top_class].value;

      // Placeholder — replace with actual model output
      uint8_t top_class = GESTURE_REST;
      float   top_conf  = 0.0f;

      // Only fire if confidence exceeds threshold AND not resting
      if (top_conf >= CONFIDENCE_THRESHOLD && top_class != GESTURE_REST) {
        portENTER_CRITICAL(&gesture_mux);
        last_gesture    = top_class;
        last_confidence = top_conf;
        gesture_ready   = true;
        portEXIT_CRITICAL(&gesture_mux);

        // Visual feedback
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(50));
        digitalWrite(LED_PIN, LOW);
      }
    }
    // Inference runs every 100ms — no need to run every sample
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n[A.E.G.I.S.] Transmitter booting..."));

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // IMU — custom I2C pins enforced in imu_handler.h
  if (!imu.begin()) {
    Serial.println(F("[FATAL] IMU init failed — halting"));
    while(1) { delay(1000); }
  }

  // ESP-NOW
  if (!espnow.begin()) {
    Serial.println(F("[FATAL] ESP-NOW init failed — halting"));
    while(1) { delay(1000); }
  }

  // Pin IMU task to Core 0 exclusively
  xTaskCreatePinnedToCore(
    imuTask,
    "IMU_Task",
    TASK_STACK_SIZE,
    NULL,
    TASK_IMU_PRIORITY,
    NULL,
    TASK_IMU_CORE
  );

  // Pin inference task to Core 1 exclusively
  xTaskCreatePinnedToCore(
    inferenceTask,
    "Inference_Task",
    TASK_STACK_SIZE,
    NULL,
    TASK_INFER_PRIORITY,
    NULL,
    TASK_INFER_CORE
  );

  Serial.println(F("[A.E.G.I.S.] Boot complete — both cores active"));
}

void loop() {
  // Main loop handles ESP-NOW dispatch only
  // Heavy work is on FreeRTOS tasks pinned to cores

  if (gesture_ready) {
    uint8_t g; float c;
    portENTER_CRITICAL(&gesture_mux);
    g = last_gesture;
    c = last_confidence;
    gesture_ready = false;
    portEXIT_CRITICAL(&gesture_mux);

    espnow.sendGesture(g, c);
  }
  vTaskDelay(pdMS_TO_TICKS(10));
}
