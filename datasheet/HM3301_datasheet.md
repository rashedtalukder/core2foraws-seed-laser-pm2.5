# Grove - Laser PM2.5 Sensor (HM3301) Datasheet

> Consolidated reference for the **Seeed Studio Grove - Laser PM2.5 Sensor** built around the
> **HM-3301** laser dust sensor. Compiled from:
>
> - [HM-3300&3600_V2.1.pdf](./HM-3300&3600_V2.1.pdf): HM-3300/3600 Dust Sensor Data Sheet V2.1,
>   released 2018-07-20. Byte-identical to the copy linked from the Seeed wiki.
> - [Seeed wiki capture](./Grove%20-%20Laser%20PM2.5%20Sensor%20(HM3301)%20%7C%20Seeed%20Studio%20Wiki.pdf)
>   of <https://wiki.seeedstudio.com/Grove-Laser_PM2.5_Sensor-HM3301/> (2026-09-26).
> - Seeed Eagle schematic for the Grove board (v1.0, 2018-09-19).
> - Seeed Arduino library `Seeed_PM2_5_sensor_HM3301` at commit `e3ffb82`.
>
> Machine-readable facts, hashes, and source conflicts are in [schema.yml](./schema.yml).
>
> The Grove module exposes **I2C only**. The sensor's UART, SET and RESET pins are not on the
> Grove connector. UART details are in [Section 8](#8-uart-protocol-bare-sensor-only) for context.

---

## 1. Overview

The HM-3301 is a laser-scattering dust detection sensor used for continuous, real-time
detection of dust (particulate matter) in the air. Unlike pumping-type sensors, it uses an
internal fan to drive air through a sealed detection chamber, where dust of various particle
sizes is measured in real time.

The Grove - Laser PM2.5 Sensor packages the HM-3301 behind a Grove 4-pin connector with an
on-board 5 V supply and an I2C level shifter, and uses the **I2C interface** for all
communication.

**Typical applications:** air purifiers and air conditioners, air quality testing equipment,
industrial PM analysis, dust and smoke detection, real-time PM2.5/PM10/TSP detectors,
multichannel particle counters, environmental testing equipment, haze meters, ventilation fans,
clean-room evaluation and filter-media testing.

---

## 2. Key Facts for Driver Development

| Property | Value |
| --- | --- |
| Sensor | HM-3301 (3-channel, weighting/mass mode) |
| Size channels | 3: 1.0 µm, 2.5 µm, 10 µm (PM1.0, PM2.5, PM10) |
| Interface (Grove module) | I2C only |
| I2C clock | 100 kHz Standard mode per Appendix 2; design notes allow 100–400 kHz |
| I2C address (7-bit) | `0x40` (silkscreen "I2C 0x40") |
| I2C write / read address (8-bit) | `0x80` / `0x81` |
| Register addressing | None. Plain 1-byte command write; plain 29-byte read |
| Select command | Write `0x88` to switch to I2C (turns off UART auto-upload) |
| Data frame length | 29 bytes (`Data[0]` … `Data[28]`, 0-indexed) |
| Checksum | `Data[28]` = low 8 bits of sum of `Data[0]` … `Data[27]` |
| Byte order | 16-bit words, high byte first |
| Grove supply | 3.3 V or 5 V. The board makes its own 5 V for the sensor |
| Host-side I2C pull-ups | 4.7 kΩ to Grove VCC (on board) |
| Stability time | 30 s after power-on or wake (fan spin-up) |
| Data refresh rate | Once per second |

> **Important:** The bare HM-3301 powers up in **UART mode** and starts auto-uploading data.
> When using I2C you must first send the select command (`0x88`) so the module turns off UART;
> otherwise the I2C data will be wrong. See [Section 7](#7-i2c-communication-protocol).

---

## 3. Features

- High sensitivity on dust particles of **0.3 µm or greater**.
- Real-time and continuous detection of dust concentration in the air.
- Based on laser light scattering (Mie scattering) technology: accurate, stable, consistent.
- Directly outputs **PM2.5** and **PM10** mass concentration in **µg/m³** (the frame also
  carries PM1.0).
- Ultra-low power consumption (< 150 µA sleep, < 75 mA operating).
- Low noise (< 45 dB measured 1 m away).
- Follows ISO 21501-4, ISO 14644-1 and FS209E standards.
- UART and I2C interfaces on the bare sensor.

Family-level claims that do **not** apply to the HM-3301:

- Six size channels (0.3, 0.5, 1.0, 2.5, 5, 10 µm). The HM-3301 is a 3-channel part.
- Humidity compensation and optional temperature/humidity sensor. The model table lists T/RH
  as "-" for the HM-3301 (optional only on HM-3302/HM-3602).

---

## 4. Specifications

### Grove module (Seeed wiki)

| Item | Value |
| --- | --- |
| Operating voltage | 3.3 V / 5 V |
| Operating temperature | -10 ~ 60 °C |
| Operating humidity | 10% ~ 90% RH (non-condensing) |
| Particle size | 3 channels: 1.0 µm, 2.5 µm, 10 µm |
| Range (PM2.5 standard value) | 1–500 µg/m³ effective, 1000 µg/m³ maximum |
| Resolution | Concentration 1 µg/m³; counting concentration 1 s / 0.1 L |
| Stability time | 30 seconds after power-on |
| Interface | I2C |
| I2C address | 0x40 |

Seeed does not specify the Grove-side supply current. Because of the on-board buck and boost
stages ([Section 6](#6-hardware--pin-out)), input current differs from the bare sensor figures below.

### Bare HM-3301 sensor (manufacturer datasheet)

| Item | Value |
| --- | --- |
| Sensor technology | Laser light scattering, electron cutting, particle counting |
| Range (PM2.5 standard) | 1–500 µg/m³ effective, 1000 µg/m³ maximum. Model table: 0–500 µg/m³ |
| Output values | PM2.5, PM10, TSP mass concentration (µg/m³); particle counts. No TSP field exists in either frame |
| Resolution | Concentration 1 µg/m³; counting concentration 1 s / 0.1 L |
| Consistency | ±10 µg/m³ @ 0–100 µg/m³; ±10% @ 100–500 µg/m³ (25 °C, 50% RH) |
| Stability time | 30 seconds after power-on |
| Sensitivity / refresh | Data refreshed once every 1 second |
| Supply voltage | DC 5 V ±3% (4.85–5.15 V) |
| Logic level | 3.3 V TTL on SET, RXD, TXD, RESET, SDA, SCL |
| Operating current | Average < 75 mA, peak < 120 mA |
| Sleep current | < 150 µA (SET low) |
| Interface | UART or I2C (not both at once) |
| Conditions of use | -10 ~ 60 °C, 10% ~ 90% RH (non-condensing) |
| Dimensions | 40 (L) × 38 (W) × 15 (H) mm |
| Mounting | Two bottom positioning holes. Drawing marks Ø2 mm; text specifies M2.5 screws |
| Connector | 1.25 mm pitch, 8-pin (1.25T-8P) |
| Life | ≥ 2 years (indoor use) |
| Standards | ISO 14644-1, FS209E (feature list adds ISO 21501-4) |

### Temperature vs. accuracy

From the "absolute tolerance (%)" plot (p.10). Values are read from the graph; the x-axis spans
-40 to 125 °C, which is wider than the -10 to 60 °C operating range.

| Temperature | Typical tolerance | Maximal tolerance |
| --- | --- | --- |
| -40 °C | ≈ 26% | ≈ 38% |
| -10 °C (operating min) | ≈ 14% | ≈ 20% |
| 10–55 °C (flat band) | ≈ 6% | ≈ 8% |
| 60 °C (operating max) | ≈ 7% | ≈ 10% |
| 125 °C | ≈ 24% | ≈ 36% |

For best accuracy keep the sensor within 10–55 °C. The consistency test plot itself was taken
at 20 °C, 50% RH.

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
                Light sensor                              Signal acquisition and processing
              Air containing dust
                     │ air inlet
                     ▼
┌───────┐     ┌──────────────┐     ┌──────────┐     ┌──────────────────┐
│ Laser │ ──▶ │   Sealed     │ ──▶ │  Light   │ ──▶ │ Filter amplifier │
└───────┘     │  detection   │     │ detector │     └────────┬─────────┘
              │   chamber    │     └──────────┘              ▼
              └──────────────┘                      ┌──────────────────┐
                     │ air outlet                   │  Multi-channel   │
                     ▼                              │   acquisition    │
                                                    └────────┬─────────┘
                                                             ▼
                                                    ┌──────────────────┐
                                                    │  Microprocessor  │
                                                    │ digital process. │
                                                    └──────────────────┘
```

### Standard particulate vs. atmospheric environment

The sensor reports **two sets** of PM values (Seeed wiki):

- **CF=1, Standard particulate matter**: mass concentration obtained by density conversion using
  industrial metal particles as equivalent particles. Suitable for industrial production workshops.
- **Atmospheric environment**: mass concentration converted using the density of the main air
  pollutants as equivalent particles. Suitable for ordinary indoor/outdoor environments.

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
> - The reference circuits show 10 kΩ pull-ups to 3.3 V on SET and RESET, and no I2C pull-ups.
> - In sleep (SET low) the fan stops; allow ≥ 30 s after wake before trusting data.

### Grove module connector (J2, 4-pin Grove → I2C)

| Grove pin | Wire color | Signal |
| --- | --- | --- |
| 1 | Yellow | SCL |
| 2 | White | SDA |
| 3 | Red | VCC (3.3 V / 5 V) |
| 4 | Black | GND |

### Grove board circuit (Seeed schematic v1.0)

| Block | Implementation |
| --- | --- |
| Input protection | F1, 1 A fuse on Grove VCC |
| Sensor 5 V | U3 ETA3410 buck → `3V0` rail → U1 ETA1038 boost → `5V0` rail → sensor pin 1 |
| 3.3 V rail | U4 XC6206P332MR LDO from Grove VCC; biases the level-shifter gates |
| I2C level shifter | Q1 (SDA) and Q2 (SCL) 2N7002, gates on 3.3 V, source on sensor side |
| Host-side pull-ups | R3, R4: 4.7 kΩ to Grove VCC |
| Sensor-side pull-ups | R1, R2: not fitted |
| SET / RESET | Routed to J3 pads (header not fitted); pull-ups R13/R12 not fitted |
| RXD / TXD | Not connected |

Integration consequences:

- The sensor always runs from the on-board 5 V boost, whichever Grove VCC is used.
- The board's 4.7 kΩ SDA/SCL pull-ups go to Grove VCC. With 5 V on Grove VCC (for example
  Core2 Port A), and the schematic shows a MOSFET level shifter. Direct connection to Core2
  Port A has been tested and works with this sensor. The schematic alone does not establish the
  voltage seen at every point on the bus or its electrical margin; re-check bus levels if
  changing the board, cable, or host setup.
- The module is specified for 3.3 V or 5 V input and generates the sensor's 5 V on-board.
- SET and RESET float on the sensor's internal pull-ups, so the sensor stays in normal mode.
  Sleep and hardware reset are only reachable by soldering to the SET/RESET pads.

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
(8-bit write `0x80`, read `0x81`). The datasheet quotes the address as `0x80`, which is the
8-bit write form.

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

> Call this **once** after power-up (and after any reset) before reading. The Seeed library sends
> it with no preceding delay.

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

No register/pointer byte is written before the read.

### 7.3 Data frame format (29 bytes, big-endian / high byte first)

Each 16-bit value is `(high_byte << 8) | low_byte`. The datasheet table numbers bytes
`Data1`…`Data29` (1-indexed) but describes the checksum as "Data0~Data28"; the offsets below
are 0-indexed and match the Seeed library.

| Byte index | Field | Description |
| --- | --- | --- |
| Data[0]–Data[1] | Reserved | Read `0x00 0xFF` in Seeed's example |
| Data[2]–Data[3] | Sensor number | Module number (0 in Seeed's example) |
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

> The HM3301 is a **3-channel weighting-mode** variant: its specified outputs are **PM1.0,
> PM2.5, PM10** mass concentration. The particle-count fields (Data[16]–Data[27]) are part of
> the shared HM-330x frame; counting mode is listed only for the HM-3302/HM-3602, and all count
> words are zero in Seeed's HM3301 example output. Treat them as unspecified.
>
> Count units conflict in the datasheet: the spec table says per 0.1 L, the frame tables say
> per 1 L.

### 7.4 Checksum validation

```
uint8_t sum = 0;
for (int i = 0; i < 28; i++) sum += buf[i];
if (sum != buf[28]) {
    // checksum error — discard frame
}
```

### 7.5 Recommended driver flow

1. Power on. Readings are not accurate until **≥ 30 seconds** after power-on (fan stabilization).
2. Send the select command (`0x80` → `0x88`) once to enable I2C / disable UART.
3. Every ≥ 1 second: read 29 bytes from address `0x81`.
4. Verify checksum ([Section 7.4](#74-checksum-validation)).
5. Parse the 16-bit big-endian fields you need (typically PM1.0/PM2.5/PM10, atmospheric set).
6. After any sensor power cycle, repeat step 2; the module defaults back to UART.

---

## 8. UART Protocol (bare sensor only)

Not usable on the Grove board: RXD/TXD are not connected. Included for completeness.

- 9600 baud, 8N1.
- Checksum: 16-bit sum of all preceding bytes, sent high byte first.

### 8.1 Module number

The module number is the UART address; modules on a shared line need unique numbers.

| Operation | Frame |
| --- | --- |
| Read (TX) | `45 40 AA AA AA AA 03 2D` |
| Read (RX) | `45 40 08 <reserved:2> <number:2> 00 00 00 00 <sum:2>` |
| Write (TX) | `45 60 00 00 <old:2> 00 00 <new:2> 00 00 00 00 <sum:2>` |

Write example (1 → 2): `45 60 00 00 00 01 00 00 00 02 00 00 00 00 00 A8`.
The datasheet's read-response example `45 40 08 00 00 00 02 00 00 00 00 00 90` ends in `00 90`,
but the sum rule gives `00 8F`.

### 8.2 Mode commands

Frame: `42 4D CMD DATAH DATAL LRCH LRCL`.

| Function | CMD | DATA | Frame (LRC derived) |
| --- | --- | --- | --- |
| Passive read | `E2` | `00 00` | `42 4D E2 00 00 01 71` |
| Set passive | `E1` | `00 00` | `42 4D E1 00 00 01 70` |
| Set active | `E1` | `00 01` | `42 4D E1 00 01 01 71` |
| Standby | `E4` | `00 00` | `42 4D E4 00 00 01 73` |
| Normal | `E4` | `00 01` | `42 4D E4 00 01 01 74` |

The datasheet does not define LRCH/LRCL; the values above apply the appendix-wide sum rule.

### 8.3 Upload frame (32 bytes)

| Offset | Field |
| --- | --- |
| 0 | `0x42` |
| 1 | `0x4D` active mode, `0x4E` polling mode |
| 2–3 | Frame length = 2 × 13 + 2 = 28 |
| 4–15 | PM1.0, PM2.5, PM10 (CF=1), then PM1.0, PM2.5, PM10 (atmospheric), µg/m³ |
| 16–27 | Counts ≥ 0.3, 0.5, 1.0, 2.5, 5, 10 µm per 1 L |
| 28–29 | Module number |
| 30–31 | Checksum = sum of bytes 0–29 |

---

## 9. Reference Driver Notes (Seeed Arduino library)

The Seeed `Seeed_PM2_5_sensor_HM3301` library (commit `e3ffb82`) confirms the I2C flow:

- `init()` calls `Wire.begin()` then writes the single byte `0x88` to address `0x40`. No delay.
- `read_sensor_value(buf, len)` always calls `Wire.requestFrom(0x40, 29)` with no register byte,
  then waits up to 10 × 1 ms for `len` bytes.
- The checksum check sums `data[0..27]` and compares with `data[28]`.
- Parsing iterates `for (i = 1; i < 8; i++)` over `data[i*2]<<8 | data[i*2+1]`, i.e. bytes
  2–15: sensor number, PM1.0/PM2.5/PM10 (CF=1), PM1.0/PM2.5/PM10 (atmospheric). Counts are not
  parsed.
- The example reads every 5 s.

Example frame bytes 0–27 (regrouped from the wiki's hex dump) and decoded output:

```
00 FF 00 00 00 2D 00 3F 00 45 00 22 00 32 00 3B 00 00 00 00 00 00 00 00 00 00 00 00

sensor num: 0
PM1.0 concentration(CF=1,Standard particulate matter,unit:ug/m3): 45
PM2.5 concentration(CF=1,Standard particulate matter,unit:ug/m3): 63
PM10 concentration(CF=1,Standard particulate matter,unit:ug/m3): 69
PM1.0 concentration(Atmospheric environment,unit:ug/m3): 34
PM2.5 concentration(Atmospheric environment,unit:ug/m3): 50
PM10 concentration(Atmospheric environment,unit:ug/m3): 59
```

---

## 10. Installation & Precautions

- The sensor metal case is electrically connected to internal power ground. Do not short it to
  other boards or product covers.
- Keep the air inlet and outlet close to the product's vents. If that is not possible, keep the
  outlet unobstructed. Provide a structure between inlet and outlet to isolate airflow.
- The product vent for the air inlet must not be smaller than the sensor's inlet.
- In purifiers, avoid placing the sensor in the purifier's own air duct; if unavoidable, isolate
  it in a separate space.
- In purifiers or fixed equipment, mount **> 20 cm above the ground** so large dust or floc does
  not wrap the fan.
- For outdoor use, the host product must protect against dust storms, rain, snow and willow floc.
- Do not disassemble (precision device).
- Fix with the two bottom positioning holes and M2.5 screws (the drawing marks the holes Ø2 mm).
- After waking from sleep, allow **≥ 30 seconds** before trusting measurements.

---

## 11. Reliability Tests (manufacturer)

All tests reported zero failures (C = 0).

| Test | Method | Result | N |
| --- | --- | --- | --- |
| Drop | 1 m onto wooden floor, 3 times | No cracking, normal function | 6 |
| Vibration | Sine + random, X/Y/Z, 10–50 Hz, 1 h | Stable | 6 |
| High-temperature operation | 45 °C, 800 h | Within spec (below) | 10 |
| Low-temperature operation | -5 °C, 600 h | Within spec | 10 |
| High-temperature/humidity storage | 70 °C, 95% RH, 720 h | Within spec | 10 |
| Low-temperature storage | -20 °C, 720 h | Within spec | 10 |
| Power fluctuation | 4.5–5.5 V at 0.1 V/min | Within spec | 2 |
| On/off | 5 V, 20 °C, 50% RH, toggled every minute | Within spec | 3 |
| Sleep current | SET low | < 150 µA | 2 |
| Noise | Quiet room, sound level meter at 1 m | < 45 dB | 1 |
| Salt spray | 5% NaCl 24 h, then 48 h rest | No obvious rust on shield | 3 |
| ESD | IEC 61000-4-2, 150 pF / 330 Ω, air discharge ±2/±4/±8 kV | Normal work | 1 |

"Within spec": 12-point average over 0–500 µg/m³ vs. a reference instrument deviates by
≤ 10 µg/m³ below 100 µg/m³ and ≤ 10% at or above 100 µg/m³.

---

## 12. HM-330x/360x Model Reference (context)

Model code `HM-3XXX`: the first `3` is the version, the next digit is the channel class, and the
last digit is the function (counting, weighing, integrated T/RH, etc.).

| Model | Channels | Output | Range | Temp/Humidity | Comms |
| --- | --- | --- | --- | --- | --- |
| **HM-3301** | 3 | Weighting (mass) | 0–500 µg/m³ | — | UART/IIC |
| HM-3302 | 3 | Counting + weighing | 0–500 µg/m³ | Optional | UART/IIC |
| HM-3601 | 8 | Weighting (mass) | 0–1000 µg/m³ | — | UART/IIC |
| HM-3602 | 6 | Counting + weighing | 0–1000 µg/m³ | Optional | UART/IIC |

Consistency for all models: ±10 µg/m³ @ 0–100 µg/m³, ±10% @ 100–500 µg/m³.

The Grove module uses the **HM-3301** and communicates over **I2C**.

---

## 13. Source Conflicts

| Topic | Conflict | Used here |
| --- | --- | --- |
| I2C address | PDF: `0x80`; Seeed: `0x40` | 7-bit `0x40` (`0x80` is the 8-bit write form) |
| I2C speed | Appendix 2: standard mode; design note: 100–400 kHz | 100 kHz default |
| Frame indexing | Table `Data1`…`Data29`; checksum "Data0~Data28" | 0-indexed, checksum over bytes 0–27 at byte 28 |
| Count units | Spec: per 0.1 L; frame tables: per 1 L | Unspecified for HM-3301 |
| Range | Spec: 1–500 / 1000 max; model table: 0–500 | Both recorded |
| Channels | Features: 6; HM-3301: 3 | 3 |
| TSP | Listed as output; no frame field | Not available |
| Humidity compensation | Features: yes; HM-3301 T/RH: "-" | Not available on HM-3301 |
| Mounting | Drawing Ø2 mm; text M2.5 screws | Both recorded |
| UART read example | Checksum `00 90`; sum rule gives `00 8F` | Sum rule |
| Consistency conditions | Spec at 25 °C; plot at 20 °C | Both recorded |

---

## 14. Resources

- Product wiki: <https://wiki.seeedstudio.com/Grove-Laser_PM2.5_Sensor-HM3301/>
- Wiki capture: [Grove - Laser PM2.5 Sensor (HM3301) | Seeed Studio Wiki.pdf](./Grove%20-%20Laser%20PM2.5%20Sensor%20(HM3301)%20%7C%20Seeed%20Studio%20Wiki.pdf)
- Eagle files (schematic): <https://files.seeedstudio.com/wiki/Grove-Laser_PM2.5_Sensor-HM3301/res/Grove%20-%20Laser%20PM2.5%20Sensor%20(HM3301).zip>
- Arduino library: <https://github.com/Seeed-Studio/Seeed_PM2_5_sensor_HM3301>
- Original datasheet PDF: [HM-3300&3600_V2.1.pdf](./HM-3300&3600_V2.1.pdf)
- Machine-readable facts: [schema.yml](./schema.yml)
