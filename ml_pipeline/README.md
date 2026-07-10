# A.E.G.I.S. — ML Training Pipeline

## Training Workflow

1. Flash transmitter firmware with data-collection Serial output enabled
2. Run `collect_data.py` for each gesture (50+ samples each)
3. Upload CSVs to Edge Impulse project
4. Configure: Spectral Features DSP + 1D Conv Neural Network
5. **Select float32 (Unoptimized) — NOT int8** (see below)
6. Train and validate — target >95% accuracy
7. Deploy → Arduino Library → extract model header
8. Include `aegis_model.h` in transmitter firmware

## Why float32, NOT int8

INT8 quantisation rounds 12.2° and 12.8° wrist tilt to both 12.
For subtle gesture micro-movements this causes false positives.
float32 preserves decimal precision. Memory cost is offset by
FreeRTOS Core 1 having 100% headroom for float32 matrix math.

## Gesture Classes (in this order for Edge Impulse labels)

0. rest
1. swipe_left
2. swipe_right
3. swipe_up
4. swipe_down

## Edge Impulse Project Settings

- Input: 1500ms window, 10ms stride, 100Hz.
- DSP: Spectral Analysis block (6 axes).
- ML: 1D Convolutional Neural Network.
- Output: float32 (unoptimized).
