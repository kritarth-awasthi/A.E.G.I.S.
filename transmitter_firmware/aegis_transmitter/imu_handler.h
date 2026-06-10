/*
 * imu_handler.h — MPU6050 IMU Handler
 * Runs exclusively on Core 0 at exactly 100Hz.
 *
 * Flat-mount directive: MPU6050 mounted flush against perfboard so
 * Z-axis absorbs exactly 1G at rest — creates clean CNN baseline.
 *
 * Custom I2C: Wire.begin(22, 21) — DO NOT revert to defaults.
 */

#pragma once
#include <Wire.h>
#include <MPU6050.h>
#include "config.h"

// Shared circular buffer — written by Core 0, read by Core 1
struct IMUFrame {
  float ax, ay, az;   // Accelerometer X/Y/Z (g)
  float gx, gy, gz;   // Gyroscope X/Y/Z (deg/s)
};

volatile IMUFrame imu_buffer[WINDOW_SIZE];
volatile int      buffer_head     = 0;
volatile bool     buffer_ready    = false;
portMUX_TYPE      buffer_mux      = portMUX_INITIALIZER_UNLOCKED;

class IMUHandler {
public:
  IMUHandler() : _mpu() {}

  bool begin() {
    // Custom I2C pins — critical, do not change
    Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN);
    Wire.setClock(400000);  // 400kHz fast mode

    _mpu.initialize();
    if (!_mpu.testConnection()) {
      Serial.println(F("[IMU] MPU6050 connection failed — check wiring"));
      return false;
    }

    // Configure ranges
    _mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);   // ±4g
    _mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_500);   // ±500 deg/s
    _mpu.setDLPFMode(MPU6050_DLPF_BW_20);              // Low-pass: 20Hz cutoff

    Serial.println(F("[IMU] MPU6050 ready @ 100Hz"));
    return true;
  }

  // Call from Core 0 task every SAMPLE_INTERVAL_MS
  void sampleAndStore() {
    int16_t ax, ay, az, gx, gy, gz;
    _mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    IMUFrame frame;
    frame.ax = ax / 8192.0f;    // Convert to g (±4g range)
    frame.ay = ay / 8192.0f;
    frame.az = az / 8192.0f;
    frame.gx = gx / 65.5f;     // Convert to deg/s (±500 range)
    frame.gy = gy / 65.5f;
    frame.gz = gz / 65.5f;

    portENTER_CRITICAL(&buffer_mux);
    imu_buffer[buffer_head] = frame;
    buffer_head = (buffer_head + 1) % WINDOW_SIZE;
    if (buffer_head == 0) buffer_ready = true;  // Full window available
    portEXIT_CRITICAL(&buffer_mux);
  }

  // Copy current window into a flat float array for CNN input
  // Input shape: [WINDOW_SIZE * 6] — 150 frames × 6 axes
  void getWindowFlat(float* output) {
    portENTER_CRITICAL(&buffer_mux);
    for (int i = 0; i < WINDOW_SIZE; i++) {
      int idx = (buffer_head + i) % WINDOW_SIZE;
      output[i * 6 + 0] = imu_buffer[idx].ax;
      output[i * 6 + 1] = imu_buffer[idx].ay;
      output[i * 6 + 2] = imu_buffer[idx].az;
      output[i * 6 + 3] = imu_buffer[idx].gx;
      output[i * 6 + 4] = imu_buffer[idx].gy;
      output[i * 6 + 5] = imu_buffer[idx].gz;
    }
    portEXIT_CRITICAL(&buffer_mux);
  }

  bool isWindowReady() { return buffer_ready; }

private:
  MPU6050 _mpu;
};
