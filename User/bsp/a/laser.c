/* Includes ----------------------------------------------------------------- */
#include "bsp_laser.h"

#include "gpio.h"

/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private typedef ---------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private function  -------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
uint8_t Laser_On(void) {
  HAL_GPIO_WritePin(LASER_GPIO_Port, LASER_Pin, GPIO_PIN_SET);
  return 0;
}

uint8_t Laser_Off(void) {
  HAL_GPIO_WritePin(LASER_GPIO_Port, LASER_Pin, GPIO_PIN_RESET);
  return 0;
}
