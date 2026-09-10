## Custom Firmware Plan & Hardware Peripheral Mapping (`STM32F103CBT6`)

Hi everyone! I am working on developing custom firmware (VESC-like / Open-FOC approach) for the stock motor controller while keeping the **hardware 100% stock** (no PCB trace modifications, no pin remapping).

The controller uses an **STM32F103CBT6** in LQFP48 paired with an ESP32 bridge (communicating via native UART1 without remap).

I would appreciate any feedback, validation, or collaboration from the community on this pinout map and diagnostic approach.

---

### 1. Identified Hardware Pinout (Stock Layout)

#### Power Stage (TIM1 - Center-Aligned PWM @ 18kHz)
* **High-Side MOSFETs:**
  * Phase A: `PA8` (`TIM1_CH1`)
  * Phase B: `PA9` (`TIM1_CH2`)
  * Phase C: `PA10` (`TIM1_CH3`)
* **Low-Side MOSFETs:**
  * Phase A: `PB13` (`TIM1_CH1N`)
  * Phase B: `PB14` (`TIM1_CH2N`)
  * Phase C: `PB15` (`TIM1_CH3N`)
* *Dead-Time setting required in `BDTR`: ~1.5 µs.*

#### Current Sensing & Fast ADC (Injected ADC triggered by `TIM1_CC4`)
* `PA4` -> `ADC12_IN4` (Phase Shunt A / Current OpAmp)
* `PA5` -> `ADC12_IN5` (Phase Shunt B / Current OpAmp)
* `PA6` -> `ADC12_IN6` (Phase Shunt C / Bus Current OpAmp)

#### Regular Group ADC & Slow Peripherals
* `PA2` -> `ADC1_IN2` (Throttle signal, ~0.8V – 4.2V)
* `PA3` -> `ADC1_IN3` (E-ABS / Low Brake analog level)
* `PA7` -> `ADC2_IN7` (Vbus / Battery Voltage Divider)
* `PB0` -> `ADC1_IN8` (Heatsink NTC Temp Sensor)
* `PB1` -> `ADC1_IN9` (Auxiliary Volts / 5V Rail control)

#### System & Communication (NATIVE - NO REMAP)
* **UART1 (to ESP32):** `PA9` (TX), `PA10` (RX) — *Native pins, AFIO remap disabled.*
* **SWD Debug:** `PA13` (SWDIO), `PA14` (SWCLK)
* **Emergency / Brake:** `PB12` (Digital In / `TIM1_BKIN`), `PA15` (High Cutoff Brake)

#### Unused / Floating Candidates
* `PC13`, `PC14`, `PC15` (No external PCB traces)
* `PB3`, `PB4` (Freed by disabling JTAG via `AFIO_MAPR_SWJ_CFG_JTAGDISABLE`, keeping SWD active)
* `PB8` (No top layer trace)

---

### 2. Diagnostic Diagnostic Firmware Plan (Safe Parameter Mapping)

Before driving the MOSFETs, a diagnostic firmware is flashed to stream raw ADC and GPIO states via UART to ESP32 (and over BLE via Nordic UART Service). All PWM outputs remain disabled.

#### Objectives:
1. **Shunt Zero-Offsets:** Capture rest voltages on `PA4`, `PA5`, `PA6` (expected ADC ~2048 / 1.65V).
2. **Hall Sensors / Position:** Identify exact pins for Hall signals (`PB3`, `PB4`, `PB5` or others) by rotating the wheel manually and logging state changes.
3. **Throttle & Brake Calibration:** Verify exact ADC ranges for `PA2` and digital pin transitions.
4. **Vbus Divider Scaling:** Map `PA7` ADC value against physical multimeter battery voltage.

#### Diagnostic Payload Format (JSON over UART @ 115200):
```json
{
  "ADC": {
    "PA2": 812, "PA3": 0, "PA4": 2045, "PA5": 2048, 
    "PA6": 2042, "PA7": 2450, "PB0": 1200, "PB1": 3100
  },
  "GPIO": {
    "PA15": 1, "PB3": 1, "PB4": 0, "PB5": 1, 
    "PB6": 1, "PB7": 1, "PB8": 1, "PB12": 1
  }
}
