# core2foraws-seed-laser-pm2.5

ESP-IDF component driver for the [Seeed Studio Grove - Laser PM2.5 Sensor](https://wiki.seeedstudio.com/Grove-Laser_PM2.5_Sensor-HM3301/) (built around the **HM3301** laser dust sensor) for use on the Core2 for AWS IoT Kit. It uses the abstractions built in to the [BSP for the Core2 for AWS](https://github.com/m5stack/Core2-for-AWS-IoT-Kit/tree/BSP-dev).

The sensor connects to the external **Port A** (I2C) on the Core2. The driver registers the device on the external I2C bus, switches it into I2C mode, reads the 29-byte measurement frame, validates the checksum, and parses the PM1.0/PM2.5/PM10 mass concentrations (both the CF=1 "standard particulate" and the "atmospheric environment" calibrations).

## Wiring

| Grove cable | Core2 Port A |
| --- | --- |
| Yellow (SCL) | SCL (GPIO 33) |
| White (SDA) | SDA (GPIO 32) |
| Red (VCC) | VCC |
| Black (GND) | GND |

## PaHUB / I2C multiplexing

The sensor can also be accessed through an [M5Stack PaHUB/PaHUB2](https://docs.m5stack.com/en/unit/pahub) I2C multiplexer instead of a direct Port A connection. Enable it in `menuconfig` under **Seeed Laser PM2.5 (HM3301) Configuration**:

- `SEED_LASER_PM25_USE_PAHUB` — route all sensor I2C through the PaHUB.
- `SEED_LASER_PM25_PAHUB_CHANNEL` — the hub channel (0-5) the sensor is wired to.

When enabled, `seed_laser_pm25_init()` initializes the hub and selects the channel automatically, and the [`core2foraws-unit-pahub`](https://github.com/rashedtalukder/core2foraws-unit-pahub) component is pulled in as a dependency. The PaHUB itself connects to Port A. No API or wiring changes are needed beyond plugging the sensor into a hub channel.

## Notes

- The HM3301 powers up in UART auto-upload mode. `seed_laser_pm25_init()` sends the I2C select command (`0x88`) once so I2C reads return valid data.
- `seed_laser_pm25_init()` starts the external Port A I2C bus itself, so you only need to call `core2foraws_init()` beforehand.
- Allow **≥ 30 seconds** (`SEED_LASER_PM25_WARMUP_MS`) after power-on for the fan to stabilize before trusting readings.
- The sensor refreshes its data once per second, so reading faster than 1 Hz returns repeated values.
- For ambient air-quality use cases, prefer the atmospheric values (`pm2_5_atm`, etc.).
- The particle-count fields (`particles_*`) are part of the shared HM-330x frame but are not specified for the 3-channel HM3301 and read `0` on this module; treat them as best-effort.

## Usage

```c
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "core2foraws.h"
#include "seed_laser_pm25.h"

static const char *TAG = "PM25_DEMO";

void app_main( void )
{
    core2foraws_init();

    if ( seed_laser_pm25_init() != ESP_OK )
    {
        ESP_LOGE( TAG, "Failed to init PM2.5 sensor" );
        return;
    }

    // Wait for the fan to stabilize before trusting readings.
    vTaskDelay( pdMS_TO_TICKS( SEED_LASER_PM25_WARMUP_MS ) );

    seed_laser_pm25_data_t data;
    for ( ;; )
    {
        if ( seed_laser_pm25_read( &data ) == ESP_OK )
        {
            ESP_LOGI( TAG, "PM1.0: %u, PM2.5: %u, PM10: %u (ug/m3, atmospheric)",
                      data.pm1_0_atm, data.pm2_5_atm, data.pm10_atm );
        }
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}
```

## API

| Function | Description |
| --- | --- |
| `seed_laser_pm25_init()` | Register the sensor on Port A and switch it into I2C mode. |
| `seed_laser_pm25_read( seed_laser_pm25_data_t * )` | Read, checksum-validate, and parse one measurement frame. |
| `seed_laser_pm25_deinit()` | Release the I2C device handle and mutex. |