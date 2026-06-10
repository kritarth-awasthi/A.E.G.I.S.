"""
collect_data.py — IMU Data Collection for AEGIS Gesture Training

Reads live sensor data from ESP32 via Serial and saves labelled
CSV datasets for Edge Impulse upload.

Usage:
  python collect_data.py --gesture swipe_left --samples 50 --port COM3

Each sample = 150 frames × 6 axes (ax,ay,az,gx,gy,gz) at 100Hz = 1.5s window
"""

import serial
import csv
import argparse
import time
import os

WINDOW_SIZE   = 150   # Must match firmware WINDOW_SIZE
NUM_AXES      = 6     # ax, ay, az, gx, gy, gz
SAMPLE_RATE   = 100   # Hz

GESTURE_LABELS = ["rest", "swipe_left", "swipe_right", "swipe_up", "swipe_down"]


def collect_samples(port: str, baud: int, gesture: str,
                    num_samples: int, output_dir: str):

    if gesture not in GESTURE_LABELS:
        print(f"[ERROR] Unknown gesture '{gesture}'")
        print(f"        Valid options: {GESTURE_LABELS}")
        return

    os.makedirs(output_dir, exist_ok=True)
    filename = os.path.join(output_dir, f"{gesture}_{int(time.time())}.csv")

    print(f"[COLLECT] Gesture: {gesture}")
    print(f"[COLLECT] Samples: {num_samples}")
    print(f"[COLLECT] Output:  {filename}")
    print(f"[COLLECT] Connecting to {port} @ {baud} baud...")

    try:
        ser = serial.Serial(port, baud, timeout=2)
        time.sleep(2)  # Let ESP32 reset
        print("[COLLECT] Connected. Starting collection...\n")
    except serial.SerialException as e:
        print(f"[ERROR] Cannot open port: {e}")
        return

    headers = ["label"] + [
        f"{axis}_{i}"
        for i in range(WINDOW_SIZE)
        for axis in ["ax", "ay", "az", "gx", "gy", "gz"]
    ]

    collected = 0

    with open(filename, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(headers)

        while collected < num_samples:
            input(f"  Press ENTER to capture sample {collected + 1}/{num_samples} "
                  f"— perform '{gesture}' immediately after...")

            print(f"  → Recording 1.5s window...")
            window_data = []

            for frame in range(WINDOW_SIZE):
                line = ser.readline().decode("utf-8", errors="ignore").strip()
                try:
                    values = [float(v) for v in line.split(",")]
                    if len(values) == NUM_AXES:
                        window_data.extend(values)
                except ValueError:
                    frame -= 1  # Retry on bad parse
                    continue

            if len(window_data) == WINDOW_SIZE * NUM_AXES:
                writer.writerow([gesture] + window_data)
                collected += 1
                print(f"  ✓ Sample {collected}/{num_samples} saved")
            else:
                print(f"  ✗ Incomplete frame ({len(window_data)} values) — retrying")

    ser.close()
    print(f"\n[COLLECT] Done — {collected} samples saved to {filename}")
    print(f"[COLLECT] Upload {filename} to Edge Impulse → Data Acquisition → Upload")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="AEGIS Data Collector")
    parser.add_argument("--gesture",  required=True,         help="Gesture label")
    parser.add_argument("--samples",  type=int, default=50,  help="Number of samples")
    parser.add_argument("--port",     default="COM3",        help="ESP32 serial port")
    parser.add_argument("--baud",     type=int, default=115200)
    parser.add_argument("--output",   default="dataset/")
    args = parser.parse_args()

    collect_samples(args.port, args.baud, args.gesture,
                    args.samples, args.output)
