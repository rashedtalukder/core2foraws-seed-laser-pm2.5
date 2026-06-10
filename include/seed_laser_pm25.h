/*!
 * @brief Driver for the Seeed Studio Grove - Laser PM2.5 Sensor (HM3301)
 * on the Core2 for AWS IoT Kit, connected to expansion Port A (I2C).
 *
 * @copyright Copyright (c) 2026 by Rashed Talukder[https://rashedtalukder.com]
 *
 * @license SPDX-License-Identifier: Apache 2.0
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @Links [HM3301](https://wiki.seeedstudio.com/Grove-Laser_PM2.5_Sensor-HM3301/)
 *
 * @version  V0.0.1
 * @date  2026-06-10
 */

#ifndef _SEED_LASER_PM25_H_
#define _SEED_LASER_PM25_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <esp_err.h>
#include <stdint.h>

/** @brief 7-bit I2C address of the HM3301, from Kconfig (default 0x40). */
#define SEED_LASER_PM25_ADDR    CONFIG_SEED_LASER_PM25_ADDRESS

/** @brief Length of the raw measurement frame returned by the sensor. */
#define SEED_LASER_PM25_FRAME_LEN 29

/** @brief Time the fan needs to stabilize after power-on for accurate data. */
#define SEED_LASER_PM25_WARMUP_MS 30000

  /**
   * @brief Parsed measurement data from one HM3301 frame.
   *
   * The HM3301 reports particulate matter mass concentration on three
   * channels (PM1.0, PM2.5, PM10) using two calibrations: the "CF=1 /
   * standard particulate" set and the "atmospheric environment" set. For
   * ambient air-quality use cases prefer the atmospheric values.
   *
   * The particle-count fields are part of the shared HM-330x frame but are
   * not part of the HM3301's specified output; treat them as best-effort.
   */
  typedef struct
  {
    uint16_t sensor_number;       /*!< Module number reported by the sensor */

    uint16_t pm1_0_std;           /*!< PM1.0 (CF=1, standard), µg/m³ */
    uint16_t pm2_5_std;           /*!< PM2.5 (CF=1, standard), µg/m³ */
    uint16_t pm10_std;            /*!< PM10  (CF=1, standard), µg/m³ */

    uint16_t pm1_0_atm;           /*!< PM1.0 (atmospheric), µg/m³ */
    uint16_t pm2_5_atm;           /*!< PM2.5 (atmospheric), µg/m³ */
    uint16_t pm10_atm;            /*!< PM10  (atmospheric), µg/m³ */

    uint16_t particles_0_3um;     /*!< Particles ≥ 0.3 µm per 0.1 L (best-effort) */
    uint16_t particles_0_5um;     /*!< Particles ≥ 0.5 µm per 0.1 L (best-effort) */
    uint16_t particles_1_0um;     /*!< Particles ≥ 1.0 µm per 0.1 L (best-effort) */
    uint16_t particles_2_5um;     /*!< Particles ≥ 2.5 µm per 0.1 L (best-effort) */
    uint16_t particles_5_0um;     /*!< Particles ≥ 5.0 µm per 0.1 L (best-effort) */
    uint16_t particles_10um;      /*!< Particles ≥ 10  µm per 0.1 L (best-effort) */
  } seed_laser_pm25_data_t;

  /**
   * @brief Initialize the Grove - Laser PM2.5 Sensor (HM3301) on Port A.
   *
   * Registers the sensor on the Core2 for AWS external I2C bus (Port A) at
   * 100 kHz, then sends the select command (0x88) once to disable the
   * sensor's default UART auto-upload and switch it into I2C mode. Creates
   * the internal mutex used to protect concurrent access.
   *
   * @note This starts the external Port A I2C bus itself (via
   *       @ref core2foraws_expports_i2c_begin), so it only requires that
   *       @ref core2foraws_init has already run.
   *
   * @note When @c CONFIG_SEED_LASER_PM25_USE_PAHUB is enabled, the sensor is
   *       accessed through an M5Stack PaHUB I2C multiplexer on the channel
   *       set by @c CONFIG_SEED_LASER_PM25_PAHUB_CHANNEL. The hub is
   *       initialized and the channel selected automatically.
   *
   * @note The sensor fan needs ≥ 30 seconds (@ref SEED_LASER_PM25_WARMUP_MS)
   *       after power-on before readings are accurate.
   *
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK    : Success
   *  - ESP_FAIL  : Failed to create the mutex
   *  - Others    : Forwarded from the I2C device add / select command
   */
  esp_err_t seed_laser_pm25_init( void );

  /**
   * @brief Read and parse one measurement frame from the sensor.
   *
   * Reads the 29-byte frame, validates the checksum, and parses the
   * big-endian 16-bit fields into @p data. The sensor refreshes its data
   * once per second, so calling faster than 1 Hz returns repeated values.
   *
   * @param[out] data Destination for the parsed values. Must not be NULL.
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK                : Success
   *  - ESP_ERR_INVALID_ARG   : data is NULL
   *  - ESP_ERR_INVALID_STATE : seed_laser_pm25_init() has not been called
   *  - ESP_ERR_INVALID_CRC   : Frame checksum mismatch (data discarded)
   *  - ESP_ERR_TIMEOUT       : Failed to acquire the mutex or I2C timeout
   *  - Others                : Forwarded from the I2C read
   */
  esp_err_t seed_laser_pm25_read( seed_laser_pm25_data_t *data );

  /**
   * @brief Release the resources used by the driver.
   *
   * Removes the I2C device handle and deletes the internal mutex. After this
   * call @ref seed_laser_pm25_init must be called again before reading.
   *
   * @return
   * [esp_err_t](https://docs.espressif.com/projects/esp-idf/en/release-v4.3/esp32/api-reference/system/esp_err.html#macros).
   *  - ESP_OK : Success
   */
  esp_err_t seed_laser_pm25_deinit( void );

#ifdef __cplusplus
}
#endif

#endif /* _SEED_LASER_PM25_H_ */
