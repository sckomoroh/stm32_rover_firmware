/*
 *  Created on: Dec 4, 2023
 *      Author: Yehor Zvihunov
 */

#include "key1_ctrl.h"

#include "main.h"

boot_t bsp_key1_pressed()
{
    return HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == 0 ? TRUE : FALSE;
}
