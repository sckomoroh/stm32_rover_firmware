/*
 *  Created on: Dec 4, 2023
 *      Author: Yehor Zvihunov
 */

#include "bsp.h"

#include <stdio.h>

#include "common.h"
#include "commands.h"
#include "main.h"

#include "beep/beep_ctrl.h"
#include "key/key1_ctrl.h"
#include "oled/oled_ctrl.h"
#include "uart/uart_ctrl.h"
#include "drivers/drivers_ctrl.h"
#include "imu/imu_ctrl.h"

struct
{
    boot_t key_pressed;
} _bsp_data;

void bsp_init()
{
    _bsp_data.key_pressed = FALSE;

    bsp_oled_init();
    bsp_uart_init();
    bsp_imu_init();
    bsp_drivers_init();

    bsp_oled_set_string(0, "Hello");
}

void bsp_process()
{
    // Process key
    _bsp_data.key_pressed = bsp_key1_pressed();

    // Process drivers
    bsp_drivers_process();

    // Process beep
    if (_bsp_data.key_pressed == TRUE)
    {
        bsp_beep_on();
        bsp_oled_clear();
    }
    else
    {
        bsp_beep_off();
    }

    // Process UART
    switch (bsp_uart_get_cmd())
    {
    case SET_STRING_CMD:
        bsp_oled_process_cmd(bsp_uart_get_data());
        break;
    case SET_SPEED_CMD:
        bsp_drivers_process_speeds_cmd(bsp_uart_get_data());
        break;
    case GET_ENCODERS_CMD:
        bsp_drivers_process_encoders_cmd();
        break;
    case GET_IMU_CMD:
        bsp_imu_process_cmd();
        break;
    }

    bsp_uart_complete_process();

    // Process OLED
    bsp_oled_process();

    HAL_Delay(10);
}
