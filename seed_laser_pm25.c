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

#include "seed_laser_pm25.h"
#include "core2foraws.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifdef CONFIG_SEED_LASER_PM25_USE_PAHUB
#include "unit_pahub.h"
#endif

static const char *_TAG = "SEED_LASER_PM25";

// HM3301 communicates at I2C Standard mode (100 kHz).
#define SEED_LASER_PM25_I2C_SPEED_HZ 100000

// Command that switches the module out of its default UART auto-upload mode
// into I2C mode. Must be sent once after power-up (datasheet section 7.1).
#define SEED_LASER_PM25_CMD_SELECT 0x88

// How long to wait for the internal mutex before giving up.
#define SEED_LASER_PM25_MUTEX_TIMEOUT_MS 1000

static SemaphoreHandle_t _pm25_mutex = NULL;
static i2c_master_dev_handle_t _pm25_dev = NULL;

// Combine two big-endian frame bytes (high byte first) into a 16-bit value.
static inline uint16_t _be16( const uint8_t *p )
{
  return (uint16_t)( ( (uint16_t)p[ 0 ] << 8 ) | p[ 1 ] );
}

// Register-less I2C write to the sensor. When CONFIG_SEED_LASER_PM25_USE_PAHUB
// is enabled the transfer is routed through the PaHUB on the configured
// channel; otherwise it goes directly out on Port A.
static esp_err_t _pm25_i2c_write( const uint8_t *data, uint16_t length )
{
#ifdef CONFIG_SEED_LASER_PM25_USE_PAHUB
  return unit_pahub_i2c_write( CONFIG_SEED_LASER_PM25_PAHUB_CHANNEL, _pm25_dev,
                               CORE2FORAWS_I2C_NO_REG, data, length );
#else
  return core2foraws_expports_i2c_write( _pm25_dev, CORE2FORAWS_I2C_NO_REG,
                                         data, length );
#endif
}

// Register-less I2C read from the sensor. Routed through the PaHUB when
// CONFIG_SEED_LASER_PM25_USE_PAHUB is enabled, else directly on Port A.
static esp_err_t _pm25_i2c_read( uint8_t *data, uint16_t length )
{
#ifdef CONFIG_SEED_LASER_PM25_USE_PAHUB
  return unit_pahub_i2c_read( CONFIG_SEED_LASER_PM25_PAHUB_CHANNEL, _pm25_dev,
                              CORE2FORAWS_I2C_NO_REG, data, length );
#else
  return core2foraws_expports_i2c_read( _pm25_dev, CORE2FORAWS_I2C_NO_REG,
                                        data, length );
#endif
}

esp_err_t seed_laser_pm25_init( void )
{
  if( _pm25_mutex != NULL )
  {
    // Already initialized.
    return ESP_OK;
  }

  _pm25_mutex = xSemaphoreCreateMutex();
  if( _pm25_mutex == NULL )
  {
    ESP_LOGE( _TAG, "Failed to create PM2.5 sensor mutex" );
    return ESP_FAIL;
  }

  // Bring up the external Port A I2C bus. This is idempotent: if the bus is
  // already started (e.g. by another Port A peripheral) it is a no-op.
  esp_err_t err = core2foraws_expports_i2c_begin();
  if( err != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to start Port A I2C bus: %s",
              esp_err_to_name( err ) );
    vSemaphoreDelete( _pm25_mutex );
    _pm25_mutex = NULL;
    return err;
  }

#ifdef CONFIG_SEED_LASER_PM25_USE_PAHUB
  // The sensor sits behind a PaHUB multiplexer. Initialize the hub and select
  // the channel the sensor is wired to before adding the device.
  err = unit_pahub_init();
  if( err != ESP_OK )
  {
    ESP_LOGE( _TAG, "PA Hub initialization failed: %s",
              esp_err_to_name( err ) );
    vSemaphoreDelete( _pm25_mutex );
    _pm25_mutex = NULL;
    return err;
  }

  err = unit_pahub_channel_set( CONFIG_SEED_LASER_PM25_PAHUB_CHANNEL );
  if( err != ESP_OK )
  {
    ESP_LOGE( _TAG, "PA Hub channel %d set failed: %s",
              CONFIG_SEED_LASER_PM25_PAHUB_CHANNEL, esp_err_to_name( err ) );
    vSemaphoreDelete( _pm25_mutex );
    _pm25_mutex = NULL;
    return err;
  }
#endif

  err = core2foraws_expports_i2c_device_add(
      SEED_LASER_PM25_ADDR, SEED_LASER_PM25_I2C_SPEED_HZ, &_pm25_dev );
  if( err != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to add PM2.5 sensor I2C device: %s",
              esp_err_to_name( err ) );
    vSemaphoreDelete( _pm25_mutex );
    _pm25_mutex = NULL;
    return err;
  }

  // Switch the module into I2C mode (disables UART auto-upload). Without
  // this the I2C reads return invalid data (datasheet section 7.1).
  const uint8_t select_cmd = SEED_LASER_PM25_CMD_SELECT;
  err = _pm25_i2c_write( &select_cmd, 1 );
  if( err != ESP_OK )
  {
    ESP_LOGE( _TAG, "Failed to send I2C select command: %s",
              esp_err_to_name( err ) );
    core2foraws_i2c_device_remove( _pm25_dev );
    _pm25_dev = NULL;
    vSemaphoreDelete( _pm25_mutex );
    _pm25_mutex = NULL;
    return err;
  }

#ifdef CONFIG_SEED_LASER_PM25_USE_PAHUB
  ESP_LOGI( _TAG, "PM2.5 sensor (HM3301) initialized via PaHUB channel %d",
            CONFIG_SEED_LASER_PM25_PAHUB_CHANNEL );
#else
  ESP_LOGI( _TAG, "PM2.5 sensor (HM3301) initialized on Port A" );
#endif
  ESP_LOGI( _TAG, "Allow >= %d ms warm-up before trusting readings",
            SEED_LASER_PM25_WARMUP_MS );
  return ESP_OK;
}

esp_err_t seed_laser_pm25_read( seed_laser_pm25_data_t *data )
{
  if( data == NULL )
  {
    return ESP_ERR_INVALID_ARG;
  }

  if( _pm25_mutex == NULL )
  {
    ESP_LOGE( _TAG, "Not initialized. Call seed_laser_pm25_init() first." );
    return ESP_ERR_INVALID_STATE;
  }

  if( xSemaphoreTake( _pm25_mutex,
                      pdMS_TO_TICKS( SEED_LASER_PM25_MUTEX_TIMEOUT_MS ) ) !=
      pdTRUE )
  {
    ESP_LOGE( _TAG, "Failed to acquire PM2.5 sensor mutex for read" );
    return ESP_ERR_TIMEOUT;
  }

  uint8_t frame[ SEED_LASER_PM25_FRAME_LEN ];
  esp_err_t err = _pm25_i2c_read( frame, SEED_LASER_PM25_FRAME_LEN );

  xSemaphoreGive( _pm25_mutex );

  if( err != ESP_OK )
  {
    ESP_LOGE( _TAG, "I2C read failed: %s", esp_err_to_name( err ) );
    return err;
  }

  // Validate the checksum: the low 8 bits of the sum of bytes 0..27 must
  // equal byte 28 (datasheet section 7.4). A mismatch means the frame is
  // corrupt and must be discarded.
  uint8_t sum = 0;
  for( int i = 0; i < SEED_LASER_PM25_FRAME_LEN - 1; i++ )
  {
    sum += frame[ i ];
  }
  if( sum != frame[ SEED_LASER_PM25_FRAME_LEN - 1 ] )
  {
    ESP_LOGW( _TAG, "Checksum mismatch (calc 0x%02X != frame 0x%02X)", sum,
              frame[ SEED_LASER_PM25_FRAME_LEN - 1 ] );
    return ESP_ERR_INVALID_CRC;
  }

  // Parse the big-endian 16-bit fields (datasheet section 7.3).
  data->sensor_number = _be16( &frame[ 2 ] );

  data->pm1_0_std = _be16( &frame[ 4 ] );
  data->pm2_5_std = _be16( &frame[ 6 ] );
  data->pm10_std = _be16( &frame[ 8 ] );

  data->pm1_0_atm = _be16( &frame[ 10 ] );
  data->pm2_5_atm = _be16( &frame[ 12 ] );
  data->pm10_atm = _be16( &frame[ 14 ] );

  data->particles_0_3um = _be16( &frame[ 16 ] );
  data->particles_0_5um = _be16( &frame[ 18 ] );
  data->particles_1_0um = _be16( &frame[ 20 ] );
  data->particles_2_5um = _be16( &frame[ 22 ] );
  data->particles_5_0um = _be16( &frame[ 24 ] );
  data->particles_10um = _be16( &frame[ 26 ] );

  return ESP_OK;
}

esp_err_t seed_laser_pm25_deinit( void )
{
  if( _pm25_mutex != NULL )
  {
    if( _pm25_dev != NULL )
    {
      core2foraws_i2c_device_remove( _pm25_dev );
      _pm25_dev = NULL;
    }

    vSemaphoreDelete( _pm25_mutex );
    _pm25_mutex = NULL;
    ESP_LOGI( _TAG, "PM2.5 sensor deinitialized" );
  }
  return ESP_OK;
}
