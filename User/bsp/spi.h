#pragma once

/* Includes ----------------------------------------------------------------- */
#include <spi.h>

#include "bsp/bsp.h"

/* Exported constants ------------------------------------------------------- */
/* Exported macro ----------------------------------------------------------- */
/* Exported types ----------------------------------------------------------- */

/* 要添加使用SPI的新设备，需要先在此添加对应的枚举值 */

/* SPI实体枚举，与设备对应 */
typedef enum {
  BSP_SPI_OLED,
  BSP_SPI_IMU,
  /* BSP_SPI_XXX,*/
  BSP_SPI_NUM,
  BSP_SPI_ERR,
} BSP_SPI_t;

/* SPI支持的中断回调函数类型，具体参考HAL中定义 */
typedef enum {
  BSP_SPI_TX_CPLT_CB,
  BSP_SPI_RX_CPLT_CB,
  BSP_SPI_TX_RX_CPLT_CB,
  BSP_SPI_TX_HALF_CPLT_CB,
  BSP_SPI_RX_HALF_CPLT_CB,
  BSP_SPI_TX_RX_HALF_CPLT_CB,
  BSP_SPI_ERROR_CB,
  BSP_SPI_ABORT_CPLT_CB,
  BSP_SPI_CB_NUM,
} BSP_SPI_Callback_t;

/* Exported functions prototypes -------------------------------------------- */
SPI_HandleTypeDef *BSP_SPI_GetHandle(BSP_SPI_t spi);
int8_t BSP_SPI_RegisterCallback(BSP_SPI_t spi, BSP_SPI_Callback_t type,
                                void (*callback)(void));
