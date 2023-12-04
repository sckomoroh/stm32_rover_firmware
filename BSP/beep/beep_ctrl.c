#include "beep_ctrl.h"

#include "main.h"

void bsp_beep_on()
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, SET);
}

void bsp_beep_off()
{
    HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, RESET);
}
