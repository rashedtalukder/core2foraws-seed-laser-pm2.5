# Grove - Laser PM2.5 Sensor (HM3301) Datasheet

> Consolidated reference for the **Seeed Studio Grove - Laser PM2.5 Sensor** built around the
> **HM-3301** laser dust sensor. Compiled from the HM-3300/3600 Dust Sensor Data Sheet (V2.1,
> July 2018) and the [Seeed Studio product wiki](https://wiki.seeedstudio.com/Grove-Laser_PM2.5_Sensor-HM3301/).
>
> This document is intended as the single source of truth for writing an I2C driver for the
> HM3301 and for answering integration questions. The Grove module exposes **I2C only**; the
> UART portions of the original HM-3300/3600 datasheet are included only for context and are
> **not wired out** on the Grove board.

---

## 1. Overview

The HM-3301 is a laser-scattering dust detection sensor used for continuous, real-time
detection of dust (particulate matter) in the air. Unlike pumping-type sensors, it uses an
internal fan to drive air through a sealed detection chamber, where dust of various particle
sizes is measured in real time.

The Grove - Laser PM2.5 Sensor packages the HM-3301 behind a Grove 4-pin connector and uses
the **I2C interface** for all communication.

**Typical applications:** dust detectors, air purifiers, air conditioners, ventilation fans,
air quality testing, haze meters, environmental monitoring, multichannel particle counters.

---

## 2. Key Facts for Driver Development

| Property | Value |
| --- | --- |
| Sensor IC | HM-3301 |
| Channels | 3 (PM1.0, PM2.5, PM10) |
| Interface (Grove module) | I2C only |
| I2C clock | 100 kHz (Standard mode). Sensor IC supports 100–400 kHz |
| I2C address (7-bit) | `0x40` |
| I2C write address (8-bit) | `0x80` |
| I2C read address (8-bit) | `0x81` |
| Select / mode command | Write byte `0x88` to switch the module into I2C mode (disables UART auto-upload) |
| Data frame length | 29 bytes (`Data[0]` … `Data[28]`) |
| Checksum | `Data[28]` = sum of `Data[0]` … `Data[27]` (low 8 bits) |
| Operating voltage | 3.3 V / 5 V (Grove module accepts both) |
| Logic level | 3.3 V on data/control pins of the bare sensor |
| Warm-up / stability time | 30 seconds after power-on (fan must spin up) |
| Data refresh rate | Once per second (1 s) |

> **Important:** The bare HM-3301 powers up in **UART mode** and starts auto-uploading data.
> When using I2C you must first send the select command (`0x88`) so the module turns off UART;
> otherwise the I2C data will be wrong. See [Section 7](#7-i2c-communication-protocol).

---

## 3. Features

- High sensitivity on dust particles of **0.3 µm or greater**.
- Real-time and continuous detection of dust concentration in the air.
- Based on laser light scattering (Mie scattering) technology — accurate, stable, consistent.
- Directly outputs **PM2.5** and **PM10** mass concentration in **µg/m³**.
- With humidity compensation; scalable for temperature/humidity sensor (model dependent).
- Ultra-low power consumption (< 150 µA sleep, < 75 mA operating).
- Low noise (< 45 dB measured 1 m away).
- Follows ISO 21501-4, ISO 14644-1 and FS209E standards.

---

## 4. Specifications

| Item | Value |
| --- | --- |
| Sensor technology | Laser light scattering, electron cutting, particle counting |
| Range (PM2.5 standard) | 1–500 µg/m³ (effective), 1000 µg/m³ (maximum) |
| Particle size channels | 3 channels: 1.0 µm, 2.5 µm, 10 µm |
| Output values | PM1.0, PM2.5, PM10 mass concentration (µg/m³) |
| Resolution | Concentration: 1 µg/m³; Counting: 1 s / 0.1 L |
| Consistency | ±10 µg/m³ @ (0–100) µg/m³; ±10% @ (100–500) µg/m³, 25 °C, 50% RH |
| Stability time | 30 seconds after power-on |
| Sensitivity / refresh | Refresh data once every 1 second |
| Supply voltage | 3.3 V / 5 V (Grove). Bare sensor: DC 5 V ±3% |
| Operating current | Average < 75 mA, peak < 120 mA |
| Sleep current | < 150 µA |
| Interface | I2C (Grove module) |
| I2C address | 0x40 (7-bit) |
| Operating temperature | -10 ~ 60 °C |
| Operating humidity | 10% ~ 90% RH (non-condensing) |
| Dimensions (sensor) | 40 (L) × 38 (W) × 15 (H) mm |
| Mounting | Two Ø2 mm positioning holes, M2.5 screws |
| Life | ≥ 2 years (indoor use) |

### Temperature vs. accuracy

The absolute consistency deviation is lowest (≈ ±6–10%) between roughly **10 °C and 55 °C** and
increases outside that band (up to ~25% typical / ~38% max near the temperature limits).
For best accuracy keep the sensor within ~10–55 °C.

---

## 5. Working Principle

The HM-3301 is based on Mie scattering theory. When light passes through particles whose size
is comparable to or larger than the wavelength of the light, it scatters. The scattered light
is concentrated onto a highly sensitive photodiode, amplified, and analyzed. Using a specific
mathematical model and algorithm, the count concentration and mass concentration of the dust
particles are derived.

Main components: fan, infrared laser source, condensing mirror, photosensitive tube (photodiode),
signal amplifying circuit, and signal sorting circuit.

```
Air containing dust
        │ (air inlet)
        ▼
   ┌──────────────┐      ┌──────────────┐      ┌───────────────────┐
   │ Sealed       │ ───▶ │ Light        │ ───▶ │ Filter amplifier  │
   │ detection    │      │ detector     │      │ → Multi-channel   │
   │ chamber      │      │ (photodiode) │      │   acquisition     │
   └──────────────┘      └──────────────┘      │ → Microprocessor  │
        ▲                                       │   digital proc.   │
   ┌────┴─────┐                                 └───────────────────┘
   │  Laser   │
   └──────────┘
        │ (air outlet)
```

### Standard particulate vs. atmospheric environment

The sensor reports **two sets** of PM values:

- **CF=1, Standard particulate matter** — mass concentration calibrated against industrial
  metallic equivalent particles. Suitable for industrial production workshops.
- **Atmospheric environment** — mass concentration calibrated against the density of typical
  atmospheric pollutants. Suitable for ordinary indoor/outdoor environments.

For most ambient air-quality use cases, prefer the **atmospheric environment** values.

---

## 6. Hardware / Pin-out

### Bare HM-3301 sensor connector (1.25T-8P)

| Pin | Name | Description |
| --- | --- | --- |
| 1 | VCC | Power supply, DC 5 V |
| 2 | GND | Power ground |
| 3 | SET | TTL @3.3 V. High/floating = normal operation; Low = sleep. Internal pull-up. |
| 4 | RXD | UART receive (TTL @3.3 V) |
| 5 | TXD | UART transmit (TTL @3.3 V) |
| 6 | RESET | Module reset (TTL @3.3 V), active low. Internal pull-up. |
| 7 | IIC_SDA | I2C data (@3.3 V) |
| 8 | IIC_SCL | I2C clock (@3.3 V) |

> Notes:
> - The sensor is powered by 5 V, but communication/control pins are 3.3 V logic.
> - SET and RESET have internal pull-ups; leave floating if unused.
> - **Do not use UART and I2C at the same time.**

### Grove module connector (4-pin Grove → I2C)

| Grove pin | Wire color | Signal |
| --- | --- | --- |
| 1 | Yellow | SCL |
| 2 | White | SDA |
| 3 | Red | VCC (3.3 V / 5 V) |
| 4 | Black | GND |

Direct wiring to an MCU (no Grove Base Shield):

| MCU | Grove cable | Sensor |
| --- | --- | --- |
| GND | Black | GND |
| 5 V or 3.3 V | Red | VCC |
| SDA | White | SDA |
| SCL | Yellow | SCL |

---

## 7. I2C Communication Protocol

In I2C Standard mode (100 kHz) the sensor is an I2C **slave** at 7-bit address `0x40`
(8-bit write `0x80`, read `0x81`).

### 7.1 Select / mode command (required at startup)

The module defaults to UART at power-up and actively uploads data. To use I2C you must first
send the select command to disable UART, otherwise the read data is invalid.

Control instruction sequence:

| Start | Address (write) | ACK | Command | ACK | Stop |
| --- | --- | --- | --- | --- | --- |
| S | `0x80` | ACK | `0x88` | ACK | P |

Driver pseudocode:

```
i2c_start();
i2c_write(0x80);   // write address (7-bit 0x40 << 1)
i2c_write(0x88);   // select / enable I2C, disable UART auto-upload
i2c_stop();
```

> Call this **once** after power-up (and after any reset) before reading.

### 7.2 Reading measurement data (29 bytes)

| Start | Address (read) | ACK | Data1 | ACK | … | Data n | ACK/NACK | Stop |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| S | `0x81` | ACK | byte | ACK | … | byte | NACK | P |

Driver pseudocode:

```
i2c_start();
i2c_write(0x81);             // read address
read 29 bytes into buf[0..28];  // ACK first 28, NACK the last
i2c_stop();
```

### 7.3 Data frame format (29 bytes, big-endian / high byte first)

Each 16-bit value is `(high_byte << 8) | low_byte`.

| Byte index | Field | Description |
| --- | --- | --- |
| Data[0]–Data[1] | Reserved | — |
| Data[2]–Data[3] | Sensor number | Module number |
| Data[4]–Data[5] | PM1.0 (CF=1, standard) | µg/m³ |
| Data[6]–Data[7] | PM2.5 (CF=1, standard) | µg/m³ |
| Data[8]–Data[9] | PM10 (CF=1, standard) | µg/m³ |
| Data[10]–Data[11] | PM1.0 (atmospheric) | µg/m³ |
| Data[12]–Data[13] | PM2.5 (atmospheric) | µg/m³ |
| Data[14]–Data[15] | PM10 (atmospheric) | µg/m³ |
| Data[16]–Data[17] | Particles ≥ 0.3 µm | count per 1 L air |
| Data[18]–Data[19] | Particles ≥ 0.5 µm | count per 1 L air |
| Data[20]–Data[21] | Particles ≥ 1.0 µm | count per 1 L air |
| Data[22]–Data[23] | Particles ≥ 2.5 µm | count per 1 L air |
| Data[24]–Data[25] | Particles ≥ 5.0 µm | count per 1 L air |
| Data[26]–Data[27] | Particles ≥ 10 µm | count per 1 L air |
| Data[28] | Checksum | Sum of Data[0]…Data[27], low 8 bits |

> The HM3301 is a **3-channel** variant: the primary, specified outputs are **PM1.0, PM2.5,
> PM10** mass concentration. The particle-count fields (Data[16]–Data[27]) are present in the
> shared HM-330x frame but are not part of the HM3301's specified 3-channel output; treat them
> as best-effort/unspecified.

### 7.4 Checksum validation

```
uint8_t sum = 0;
for (int i = 0; i < 28; i++) sum += buf[i];
if (sum != buf[28]) {
    // checksum error — discard frame
}
```

### 7.5 Recommended driver flow

1. Power on; wait **≥ 30 seconds** for the fan to stabilize for accurate readings.
2. Send the select command (`0x80` → `0x88`) once to enable I2C / disable UART.
3. Every ≥ 1 second: read 29 bytes from address `0x81`.
4. Verify checksum (`Section 7.4`).
5. Parse the 16-bit big-endian fields you need (typically PM1.0/PM2.5/PM10, atmospheric set).

---

## 8. Reference Driver Notes (from Seeed Arduino library)

The Seeed `Seeed_HM330X` library (`Seeed_HM330X.h`) confirms the I2C flow:

- `init()` sends the select command (`0x88`) to the device at I2C address `0x40`.
- `read_sensor_value(buf, 29)` reads the 29-byte frame.
- Parsing iterates `for (i = 1; i < 8; i++)` over 16-bit values starting at `buf[2]`, mapping:
  - sensor number, PM1.0/PM2.5/PM10 (CF=1 standard), PM1.0/PM2.5/PM10 (atmospheric).

Example serial output for a healthy read:

```
sensor num: 0
PM1.0 concentration(CF=1,Standard particulate matter,unit:ug/m3): 45
PM2.5 concentration(CF=1,Standard particulate matter,unit:ug/m3): 63
PM10 concentration(CF=1,Standard particulate matter,unit:ug/m3): 69
PM1.0 concentration(Atmospheric environment,unit:ug/m3): 34
PM2.5 concentration(Atmospheric environment,unit:ug/m3): 50
PM10 concentration(Atmospheric environment,unit:ug/m3): 59
```

---

## 9. Installation & Precautions

- The sensor metal case is electrically connected to internal power ground — do not short it to
  other boards or covers.
- Mount with air inlet/outlet near the product's air holes; keep the air outlet unobstructed.
  Provide structure between inlet and outlet to isolate airflow.
- Position the sensor **≥ 20 cm above the ground** to avoid large dust/flocs fouling the fan.
- For outdoor use, the host product must protect against dust storms, rain/snow and floc.
- Do not disassemble (precision device).
- Fix using the two Ø2 mm holes with M2.5 screws.
- After waking from sleep, allow **≥ 30 seconds** before trusting measurements.

---

## 10. HM-330x Model Reference (context)

The HM3301 belongs to the HM-330x family. For reference:

| Model | Channels | Output | Range | Temp/Humidity | Comms |
| --- | --- | --- | --- | --- | --- |
| **HM-3301** | 3 | Weighting (mass) | 0–500 µg/m³ | — | UART/IIC |
| HM-3302 | 3 | Counting + weighing | 0–500 µg/m³ | Optional | UART/IIC |
| HM-3601 | 8 | Weighting (mass) | 0–1000 µg/m³ | — | UART/IIC |
| HM-3602 | 6 | Counting + weighing | 0–1000 µg/m³ | Optional | UART/IIC |

The Grove module uses the **HM-3301** and communicates over **I2C**.

---

## 11. Resources

- Product wiki: <https://wiki.seeedstudio.com/Grove-Laser_PM2.5_Sensor-HM3301/>
- Arduino library: <https://github.com/Seeed-Studio/Seeed_PM2_5_sensor_HM3301>
- Original datasheet PDF: [HM-3300&3600_V2.1.pdf](./HM-3300&3600_V2.1.pdf)
