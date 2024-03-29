/*
        BMI088 陀螺仪+加速度计传感器。

*/

#include "dev_bmi088.h"

#include <stdbool.h>
#include <string.h>

#include "bsp_delay.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "comp_utils.h"

#define BMI088_REG_ACCL_CHIP_ID (0x00)
#define BMI088_REG_ACCL_ERR (0x02)
#define BMI088_REG_ACCL_STATUS (0x03)
#define BMI088_REG_ACCL_X_LSB (0x12)
#define BMI088_REG_ACCL_X_MSB (0x13)
#define BMI088_REG_ACCL_Y_LSB (0x14)
#define BMI088_REG_ACCL_Y_MSB (0x15)
#define BMI088_REG_ACCL_Z_LSB (0x16)
#define BMI088_REG_ACCL_Z_MSB (0x17)
#define BMI088_REG_ACCL_SENSORTIME_0 (0x18)
#define BMI088_REG_ACCL_SENSORTIME_1 (0x19)
#define BMI088_REG_ACCL_SENSORTIME_2 (0x1A)
#define BMI088_REG_ACCL_INT_STAT_1 (0x1D)
#define BMI088_REG_ACCL_TEMP_MSB (0x22)
#define BMI088_REG_ACCL_TEMP_LSB (0x23)
#define BMI088_REG_ACCL_CONF (0x40)
#define BMI088_REG_ACCL_RANGE (0x41)
#define BMI088_REG_ACCL_INT1_IO_CONF (0x53)
#define BMI088_REG_ACCL_INT2_IO_CONF (0x54)
#define BMI088_REG_ACCL_INT1_INT2_MAP_DATA (0x58)
#define BMI088_REG_ACCL_SELF_TEST (0x6D)
#define BMI088_REG_ACCL_PWR_CONF (0x7C)
#define BMI088_REG_ACCL_PWR_CTRL (0x7D)
#define BMI088_REG_ACCL_SOFTRESET (0x7E)

#define BMI088_REG_GYRO_CHIP_ID (0x00)
#define BMI088_REG_GYRO_X_LSB (0x02)
#define BMI088_REG_GYRO_X_MSB (0x03)
#define BMI088_REG_GYRO_Y_LSB (0x04)
#define BMI088_REG_GYRO_Y_MSB (0x05)
#define BMI088_REG_GYRO_Z_LSB (0x06)
#define BMI088_REG_GYRO_Z_MSB (0x07)
#define BMI088_REG_GYRO_INT_STAT_1 (0x0A)
#define BMI088_REG_GYRO_RANGE (0x0F)
#define BMI088_REG_GYRO_BANDWIDTH (0x10)
#define BMI088_REG_GYRO_LPM1 (0x11)
#define BMI088_REG_GYRO_SOFTRESET (0x14)
#define BMI088_REG_GYRO_INT_CTRL (0x15)
#define BMI088_REG_GYRO_INT3_INT4_IO_CONF (0x16)
#define BMI088_REG_GYRO_INT3_INT4_IO_MAP (0x18)
#define BMI088_REG_GYRO_SELF_TEST (0x3C)

#define BMI088_CHIP_ID_ACCL (0x1E)
#define BMI088_CHIP_ID_GYRO (0x0F)

#define BMI088_ACCL_RX_BUFF_LEN (19)
#define BMI088_GYRO_RX_BUFF_LEN (6)

#define BMI088_ACCL_NSS_SET() \
  HAL_GPIO_WritePin(ACCL_CS_GPIO_Port, ACCL_CS_Pin, GPIO_PIN_SET)
#define BMI088_ACCL_NSS_RESET() \
  HAL_GPIO_WritePin(ACCL_CS_GPIO_Port, ACCL_CS_Pin, GPIO_PIN_RESET)

#define BMI088_GYRO_NSS_SET() \
  HAL_GPIO_WritePin(GYRO_CS_GPIO_Port, GYRO_CS_Pin, GPIO_PIN_SET)
#define BMI088_GYRO_NSS_RESET() \
  HAL_GPIO_WritePin(GYRO_CS_GPIO_Port, GYRO_CS_Pin, GPIO_PIN_RESET)

typedef enum {
  BMI_ACCL,
  BMI_GYRO,
} BMI_Device_t;

static uint8_t tx_rx_buf[2];
static uint8_t dma_buf[BMI088_ACCL_RX_BUFF_LEN + BMI088_GYRO_RX_BUFF_LEN];

static bool inited = false;

static void BMI_WriteSingle(BMI_Device_t dv, uint8_t reg, uint8_t data) {
  tx_rx_buf[0] = (reg & 0x7f);
  tx_rx_buf[1] = data;

  BSP_Delay(1);
  switch (dv) {
    case BMI_ACCL:
      BMI088_ACCL_NSS_RESET();
      break;

    case BMI_GYRO:
      BMI088_GYRO_NSS_RESET();
      break;
  }

  HAL_SPI_Transmit(BSP_SPI_GetHandle(BSP_SPI_IMU), tx_rx_buf, 2u, 20u);

  switch (dv) {
    case BMI_ACCL:
      BMI088_ACCL_NSS_SET();
      break;

    case BMI_GYRO:
      BMI088_GYRO_NSS_SET();
      break;
  }
}

static err_t BMI_ReadSingle(BMI_Device_t dv, uint8_t reg) {
  BSP_Delay(1);
  switch (dv) {
    case BMI_ACCL:
      BMI088_ACCL_NSS_RESET();
      break;

    case BMI_GYRO:
      BMI088_GYRO_NSS_RESET();
      break;
  }
  tx_rx_buf[0] = (uint8_t)(reg | 0x80);
  HAL_SPI_Transmit(BSP_SPI_GetHandle(BSP_SPI_IMU), tx_rx_buf, 1u, 20u);
  HAL_SPI_Receive(BSP_SPI_GetHandle(BSP_SPI_IMU), tx_rx_buf, 2u, 20u);

  switch (dv) {
    case BMI_ACCL:
      BMI088_ACCL_NSS_SET();
      return tx_rx_buf[1];

    case BMI_GYRO:
      BMI088_GYRO_NSS_SET();
      return tx_rx_buf[0];
  }
  return RM_OK;
}

static void BMI_Read(BMI_Device_t dv, uint8_t reg, uint8_t *data, uint8_t len) {
  ASSERT(data);

  switch (dv) {
    case BMI_ACCL:
      BMI088_ACCL_NSS_RESET();
      break;

    case BMI_GYRO:
      BMI088_GYRO_NSS_RESET();
      break;
  }
  tx_rx_buf[0] = (uint8_t)(reg | 0x80);
  HAL_SPI_Transmit(BSP_SPI_GetHandle(BSP_SPI_IMU), tx_rx_buf, 1u, 20u);
  HAL_SPI_Receive_DMA(BSP_SPI_GetHandle(BSP_SPI_IMU), data, len);
}

static void BMI088_RxCpltCallback(void *arg) {
  bmi088_t *bmi088 = arg;
  BaseType_t switch_required;
  if (HAL_GPIO_ReadPin(ACCL_CS_GPIO_Port, ACCL_CS_Pin) == GPIO_PIN_RESET) {
    BMI088_ACCL_NSS_SET();
    xSemaphoreGiveFromISR(bmi088->sem.accl_raw, &switch_required);
  }
  if (HAL_GPIO_ReadPin(GYRO_CS_GPIO_Port, GYRO_CS_Pin) == GPIO_PIN_RESET) {
    BMI088_GYRO_NSS_SET();
    xSemaphoreGiveFromISR(bmi088->sem.gyro_raw, &switch_required);
  }
  portYIELD_FROM_ISR(switch_required);
}

static void BMI088_AcclIntCallback(void *arg) {
  bmi088_t *bmi088 = arg;
  BaseType_t switch_required;
  xSemaphoreGiveFromISR(bmi088->sem.accl_new, &switch_required);
  portYIELD_FROM_ISR(switch_required);
}

static void BMI088_GyroIntCallback(void *arg) {
  bmi088_t *bmi088 = arg;
  BaseType_t switch_required;
  xSemaphoreGiveFromISR(bmi088->sem.gyro_new, &switch_required);
  portYIELD_FROM_ISR(switch_required);
}

int8_t bmi088_init(bmi088_t *bmi088, const bmi088_cali_t *cali) {
  ASSERT(bmi088);
  ASSERT(cali);

  if (inited) return ERR_INITED;
  inited = true;

  bmi088->sem.gyro_new = xSemaphoreCreateBinary();
  bmi088->sem.accl_new = xSemaphoreCreateBinary();
  bmi088->sem.gyro_raw = xSemaphoreCreateBinary();
  bmi088->sem.accl_raw = xSemaphoreCreateBinary();

  bmi088->cali = cali;

  BMI_WriteSingle(BMI_ACCL, BMI088_REG_ACCL_SOFTRESET, 0xB6);
  BMI_WriteSingle(BMI_GYRO, BMI088_REG_GYRO_SOFTRESET, 0xB6);
  BSP_Delay(30);

  /* Switch accl to SPI mode. */
  BMI_ReadSingle(BMI_ACCL, BMI088_CHIP_ID_ACCL);

  if (BMI_ReadSingle(BMI_ACCL, BMI088_REG_ACCL_CHIP_ID) != BMI088_CHIP_ID_ACCL)
    return ERR_NODEV;

  if (BMI_ReadSingle(BMI_GYRO, BMI088_REG_GYRO_CHIP_ID) != BMI088_CHIP_ID_GYRO)
    return ERR_NODEV;

  BSP_GPIO_DisableIRQ(ACCL_INT_Pin);
  BSP_GPIO_DisableIRQ(GYRO_INT_Pin);

  BSP_SPI_RegisterCallback(BSP_SPI_IMU, BSP_SPI_RX_CPLT_CB,
                           BMI088_RxCpltCallback, bmi088);
  BSP_GPIO_RegisterCallback(ACCL_INT_Pin, BMI088_AcclIntCallback, bmi088);
  BSP_GPIO_RegisterCallback(GYRO_INT_Pin, BMI088_GyroIntCallback, bmi088);

  /* Accl init. */
  /* Filter setting: Normal. */
  /* ODR: 0xAB: 800Hz. 0xAA: 400Hz. 0xA9: 200Hz. 0xA8: 100Hz. 0xA6: 25Hz. */
  BMI_WriteSingle(BMI_ACCL, BMI088_REG_ACCL_CONF, 0xAA);

  /* 0x00: +-3G. 0x01: +-6G. 0x02: +-12G. 0x03: +-24G. */
  BMI_WriteSingle(BMI_ACCL, BMI088_REG_ACCL_RANGE, 0x01);

  /* INT1 as output. Push-pull. Active low. Output. */
  BMI_WriteSingle(BMI_ACCL, BMI088_REG_ACCL_INT1_IO_CONF, 0x08);

  /* Map data ready interrupt to INT1. */
  BMI_WriteSingle(BMI_ACCL, BMI088_REG_ACCL_INT1_INT2_MAP_DATA, 0x04);

  /* Turn on accl. Now we can read data. */
  BMI_WriteSingle(BMI_ACCL, BMI088_REG_ACCL_PWR_CTRL, 0x04);
  BSP_Delay(50);

  /* Gyro init. */
  /* 0x00: +-2000. 0x01: +-1000. 0x02: +-500. 0x03: +-250. 0x04: +-125. */
  BMI_WriteSingle(BMI_GYRO, BMI088_REG_GYRO_RANGE, 0x01);

  /* Filter bw: 47Hz. */
  /* ODR: 0x02: 1000Hz. 0x03: 400Hz. 0x06: 200Hz. 0x07: 100Hz. */
  BMI_WriteSingle(BMI_GYRO, BMI088_REG_GYRO_BANDWIDTH, 0x03);

  /* INT3 and INT4 as output. Push-pull. Active low. */
  BMI_WriteSingle(BMI_GYRO, BMI088_REG_GYRO_INT3_INT4_IO_CONF, 0x00);

  /* Map data ready interrupt to INT3. */
  BMI_WriteSingle(BMI_GYRO, BMI088_REG_GYRO_INT3_INT4_IO_MAP, 0x01);

  /* Enable new data interrupt. */
  BMI_WriteSingle(BMI_GYRO, BMI088_REG_GYRO_INT_CTRL, 0x80);

  BSP_Delay(10);

  BSP_GPIO_EnableIRQ(ACCL_INT_Pin);
  BSP_GPIO_EnableIRQ(GYRO_INT_Pin);
  return RM_OK;
}

bool bmi088_accl_wait_new(bmi088_t *bmi088, uint32_t timeout) {
  return xSemaphoreTake(bmi088->sem.accl_new, pdMS_TO_TICKS(timeout)) == pdTRUE;
}

bool bmi088_gyro_wait_new(bmi088_t *bmi088, uint32_t timeout) {
  return xSemaphoreTake(bmi088->sem.gyro_new, pdMS_TO_TICKS(timeout)) == pdTRUE;
}

err_t bmi088_accl_start_dma_recv() {
  BMI_Read(BMI_ACCL, BMI088_REG_ACCL_X_LSB, dma_buf, BMI088_ACCL_RX_BUFF_LEN);
  return RM_OK;
}

err_t bmi088_accl_wait_dma_cplt(bmi088_t *bmi088) {
  xSemaphoreTake(bmi088->sem.accl_raw, portMAX_DELAY);
  return RM_OK;
}

err_t bmi088_gyro_start_dma_recv() {
  BMI_Read(BMI_GYRO, BMI088_REG_GYRO_X_LSB, dma_buf + BMI088_ACCL_RX_BUFF_LEN,
           BMI088_GYRO_RX_BUFF_LEN);
  return RM_OK;
}

err_t bmi088_gyro_wait_dma_cplt(bmi088_t *bmi088) {
  xSemaphoreTake(bmi088->sem.gyro_raw, portMAX_DELAY);
  return RM_OK;
}

err_t bmi088_parse_accl(bmi088_t *bmi088) {
  ASSERT(bmi088);

#if 1
  int16_t raw_x, raw_y, raw_z;
  memcpy(&raw_x, dma_buf + 1, sizeof(raw_x));
  memcpy(&raw_y, dma_buf + 3, sizeof(raw_y));
  memcpy(&raw_z, dma_buf + 5, sizeof(raw_z));

  bmi088->accl.x = (float)raw_x;
  bmi088->accl.y = (float)raw_y;
  bmi088->accl.z = (float)raw_z;

#else
  const int16_t *praw_x = (int16_t *)(dma_buf + 1);
  const int16_t *praw_y = (int16_t *)(dma_buf + 3);
  const int16_t *praw_z = (int16_t *)(dma_buf + 5);

  bmi088->accl.x = (float)*praw_x;
  bmi088->accl.y = (float)*praw_y;
  bmi088->accl.z = (float)*praw_z;

#endif

  /* 3G: 10920. 6G: 5460. 12G: 2730. 24G: 1365. */
  bmi088->accl.x /= 5460.0f;
  bmi088->accl.y /= 5460.0f;
  bmi088->accl.z /= 5460.0f;

  int16_t raw_temp = (int16_t)((dma_buf[17] << 3) | (dma_buf[18] >> 5));

  if (raw_temp > 1023) raw_temp -= 2048;

  bmi088->temp = (float)raw_temp * 0.125f + 23.0f;

  return RM_OK;
}

err_t bmi088_parse_gyro(bmi088_t *bmi088) {
  ASSERT(bmi088);

#if 1
  /* Gyroscope imu_raw -> degrees/sec -> radians/sec */
  int16_t raw_x, raw_y, raw_z;
  memcpy(&raw_x, dma_buf + BMI088_ACCL_RX_BUFF_LEN, sizeof(raw_x));
  memcpy(&raw_y, dma_buf + BMI088_ACCL_RX_BUFF_LEN + 2, sizeof(raw_y));
  memcpy(&raw_z, dma_buf + BMI088_ACCL_RX_BUFF_LEN + 4, sizeof(raw_z));

  bmi088->gyro.x = (float)raw_x;
  bmi088->gyro.y = (float)raw_y;
  bmi088->gyro.z = (float)raw_z;

#else
  /* Gyroscope imu_raw -> degrees/sec -> radians/sec */
  const int16_t *raw_x = (int16_t *)(dma_buf + BMI088_ACCL_RX_BUFF_LEN);
  const int16_t *raw_y = (int16_t *)(dma_buf + BMI088_ACCL_RX_BUFF_LEN + 2);
  const int16_t *raw_z = (int16_t *)(dma_buf + BMI088_ACCL_RX_BUFF_LEN + 4);

  bmi088->gyro.x = (float)*raw_x;
  bmi088->gyro.y = (float)*raw_y;
  bmi088->gyro.z = (float)*raw_z;
#endif

  /* FS125: 262.144. FS250: 131.072. FS500: 65.536. FS1000: 32.768.
   * FS2000: 16.384.*/
  bmi088->gyro.x /= 32.768f;
  bmi088->gyro.y /= 32.768f;
  bmi088->gyro.z /= 32.768f;

  bmi088->gyro.x *= M_DEG2RAD_MULT;
  bmi088->gyro.y *= M_DEG2RAD_MULT;
  bmi088->gyro.z *= M_DEG2RAD_MULT;

  bmi088->gyro.x -= bmi088->cali->gyro_offset.x;
  bmi088->gyro.y -= bmi088->cali->gyro_offset.y;
  bmi088->gyro.z -= bmi088->cali->gyro_offset.z;

  return RM_OK;
}

float bmi088_get_update_freq(bmi088_t *bmi088) {
  RM_UNUSED(bmi088);
  return 400.0f;
}
