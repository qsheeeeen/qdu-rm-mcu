/*
        IST8310 地磁传感器。

*/

#include "dev_ist8310.h"

#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "bsp_delay.h"
#include "bsp_gpio.h"
#include "bsp_i2c.h"
#include "task.h"

#define IST8310_WAI (0x00)
#define IST8310_STAT1 (0x02)
#define IST8310_DATAXL (0x03)
#define IST8310_STAT2 (0x09)
#define IST8310_CNTL1 (0x0A)
#define IST8310_CNTL2 (0x0B)
#define IST8310_STR (0x0C)
#define IST8310_TEMPL (0x1C)
#define IST8310_TEMPH (0x1D)
#define IST8310_AVGCNTL (0x41)
#define IST8310_PDCNTL (0x42)

#define IST8310_CHIP_ID (0x10)

#define IST8310_IIC_ADDRESS (0x0E << 1)

#define IST8310_LEN_RX_BUFF (6)

#define IST8310_SET() \
  HAL_GPIO_WritePin(CMPS_RST_GPIO_Port, CMPS_RST_Pin, GPIO_PIN_SET)
#define IST8310_RESET() \
  HAL_GPIO_WritePin(CMPS_RST_GPIO_Port, CMPS_RST_Pin, GPIO_PIN_RESET)

uint8_t ist8310_rxbuf[IST8310_LEN_RX_BUFF];

static TaskHandle_t thread_alert;
static bool inited = false;

static void IST8310_WriteSingle(uint8_t reg, uint8_t data) {
  HAL_I2C_Mem_Write(BSP_I2C_GetHandle(BSP_I2C_COMP), IST8310_IIC_ADDRESS, reg,
                    I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
}

static uint8_t IST8310_ReadSingle(uint8_t reg) {
  uint8_t buf = 0;
  HAL_I2C_Mem_Read(BSP_I2C_GetHandle(BSP_I2C_COMP), IST8310_IIC_ADDRESS, reg,
                   I2C_MEMADD_SIZE_8BIT, &buf, 1, 100);
  return buf;
}

static void IST8310_Read(uint8_t reg, uint8_t *data, uint8_t len) {
  ASSERT(data);

  HAL_I2C_Mem_Read_DMA(BSP_I2C_GetHandle(BSP_I2C_COMP), IST8310_IIC_ADDRESS,
                       reg, I2C_MEMADD_SIZE_8BIT, data, len);
}

static void IST8310_MemRxCpltCallback(void) {
  BaseType_t switch_required;
  xTaskNotifyFromISR(thread_alert, SIGNAL_IST8310_MAGN_RAW_REDY,
                     eSetValueWithOverwrite, &switch_required);
  portYIELD_FROM_ISR(switch_required);
}

static void IST8310_IntCallback(void) {
  BaseType_t switch_required;
  xTaskNotifyFromISR(thread_alert, SIGNAL_IST8310_MAGN_NEW_DATA,
                     eSetValueWithOverwrite, &switch_required);
  portYIELD_FROM_ISR(switch_required);
}

err_t ist8310_init(ist8310_t *ist8310, const ist8310_cali_t *cali) {
  ASSERT(ist8310);
  ASSERT(cali);
  if (inited) return ERR_INITED;
  VERIFY((thread_alert = xTaskGetCurrentTaskHandle()) != NULL);

  ist8310->cali = cali;

  IST8310_RESET();
  BSP_Delay(50);
  IST8310_SET();
  BSP_Delay(50);

  if (IST8310_ReadSingle(IST8310_WAI) != IST8310_CHIP_ID) return ERR_NODEV;

  BSP_GPIO_DisableIRQ(CMPS_INT_Pin);

  BSP_I2C_RegisterCallback(BSP_I2C_COMP, HAL_I2C_MEM_RX_CPLT_CB,
                           IST8310_MemRxCpltCallback);
  BSP_GPIO_RegisterCallback(CMPS_INT_Pin, IST8310_IntCallback, ist8310);

  /* Init. */
  /* 0x00: Stand-By mode. 0x01: Single measurement mode. */

  /* 0x08: Data ready function enable. DRDY signal active low*/
  IST8310_WriteSingle(IST8310_CNTL2, 0x08);

  IST8310_WriteSingle(IST8310_AVGCNTL, 0x09);
  IST8310_WriteSingle(IST8310_PDCNTL, 0xC0);
  IST8310_WriteSingle(IST8310_CNTL1, 0x0B);
  BSP_Delay(10);

  inited = true;

  BSP_GPIO_EnableIRQ(CMPS_INT_Pin);
  return RM_OK;
}

bool ist8310_wait_new(uint32_t timeout) {
  return xTaskNotifyWait(0, 0, (uint32_t *)SIGNAL_IST8310_MAGN_NEW_DATA,
                         pdMS_TO_TICKS(timeout));
}

err_t ist8310_start_dma_recv() {
  IST8310_Read(IST8310_DATAXL, ist8310_rxbuf, IST8310_LEN_RX_BUFF);
  return RM_OK;
}

uint32_t ist8310_wait_dma_cplt() {
  return xTaskNotifyWait(0, 0, (uint32_t *)SIGNAL_IST8310_MAGN_RAW_REDY, 0);
}

err_t ist8310_parse(ist8310_t *ist8310) {
  ASSERT(ist8310);

#if 1
  /* Magn -> T */
  int16_t raw_x, raw_y, raw_z;
  memcpy(&raw_x, ist8310_rxbuf + 0, sizeof(raw_x));
  memcpy(&raw_y, ist8310_rxbuf + 2, sizeof(raw_y));
  memcpy(&raw_z, ist8310_rxbuf + 4, sizeof(raw_z));

  ist8310->magn.x = (float)raw_x;
  ist8310->magn.y = (float)raw_y;
  ist8310->magn.z = (float)-raw_z;

#else
  const int16_t *raw_x = (int16_t *)(ist8310_rxbuf + 0);
  const int16_t *raw_y = (int16_t *)(ist8310_rxbuf + 2);
  const int16_t *raw_z = (int16_t *)(ist8310_rxbuf + 4);

  ist8310->magn.x = (float)*raw_x;
  ist8310->magn.y = (float)*raw_y;
  ist8310->magn.z = -(float)*raw_z;
#endif

  ist8310->magn.x *= 3.0f / 20.0f;
  ist8310->magn.y *= 3.0f / 20.0f;
  ist8310->magn.z *= 3.0f / 20.0f;

  ist8310->magn.x = (ist8310->magn.x - ist8310->cali->magn_offset.x) *
                    ist8310->cali->magn_scale.x;
  ist8310->magn.y = (ist8310->magn.y - ist8310->cali->magn_offset.y) *
                    ist8310->cali->magn_scale.y;
  ist8310->magn.z = (ist8310->magn.z - ist8310->cali->magn_offset.y) *
                    ist8310->cali->magn_scale.z;

  return RM_OK;
}
