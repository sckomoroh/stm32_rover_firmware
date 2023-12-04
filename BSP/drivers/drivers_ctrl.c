/*
 * bsp_drivers.c
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#include "drivers_ctrl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hw_interfaces.h"
#include "main.h"
#include "oled/oled_ctrl.h"
#include "uart/uart_ctrl.h"

typedef enum {
    DT_FRONT_LEFT = 0,
    DT_FRONT_RIGHT = 1,
    DT_REAR_LEFT = 2,
    DT_REAR_RIGHT = 3
} driver_type_t;

#define PWM_M1_A TIM8->CCR1
#define PWM_M1_B TIM8->CCR2

#define PWM_M2_A TIM8->CCR3
#define PWM_M2_B TIM8->CCR4

#define PWM_M3_A TIM1->CCR4
#define PWM_M3_B TIM1->CCR1

#define PWM_M4_A TIM1->CCR2
#define PWM_M4_B TIM1->CCR3

struct {
    int64_t front_left, front_right;
    int64_t rear_left, rear_right;
} _encoders_data;

struct {
    int32_t front_left, front_right;
    int32_t rear_left, rear_right;
} _speed_param;

static void _drivers_update_encoders_values();
static void _drivers_set_front_left_pwm(int32_t pwm);
static void _drivers_set_front_right_pwm(int32_t pwm);
static void _drivers_set_rear_left_pwm(int32_t pwm);
static void _drivers_set_rear_right_pwm(int32_t pwm);
static void _drivers_set_speeds(int32_t fl, int32_t fr, int32_t rl, int32_t rr);

void bsp_drivers_init()
{
    // Encoders initialization
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_1 | TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1 | TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_1 | TIM_CHANNEL_2);

    // Drivers initialization
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
}

void bsp_drivers_process() { _drivers_update_encoders_values(); }

void bsp_drivers_process_encoders_cmd()
{
    bsp_uart_send_data(&_encoders_data, sizeof(_encoders_data));
    memset(&_encoders_data, 0, sizeof(_encoders_data));
}

void bsp_drivers_process_speeds_cmd(void* data)
{
    bsp_oled_set_string(1, "Set speeds");
//    _bsp_uart_print("[BSP DRIVERS] Set speeds");

    char buffer[24] = {0};

    memset(&_speed_param, 0, sizeof(_speed_param));
    memcpy(&_speed_param, data, sizeof(_speed_param));

    sprintf(buffer, "%d %d %d %d", _speed_param.front_left, _speed_param.front_right,
            _speed_param.rear_left, _speed_param.rear_right);
    bsp_oled_set_string(2, buffer);

    _drivers_set_speeds(_speed_param.front_left, _speed_param.front_right, _speed_param.rear_left,
                        _speed_param.rear_right);
}

static void _drivers_update_encoders_values()
{
    _encoders_data.front_left += (short)TIM2->CNT;
    TIM2->CNT = 0;
    _encoders_data.front_right += (short)TIM3->CNT;
    TIM3->CNT = 0;
    _encoders_data.rear_left += (short)TIM4->CNT;
    TIM4->CNT = 0;
    _encoders_data.rear_right += (short)TIM5->CNT;
    TIM5->CNT = 0;
}

static void _drivers_set_front_left_pwm(int32_t pwm)
{
    if (pwm >= 0) {
        PWM_M1_A = pwm;
        PWM_M1_B = 0;
    }
    else {
        PWM_M1_A = 0;
        PWM_M1_B = -pwm;
    }
}

static void _drivers_set_front_right_pwm(int32_t pwm)
{
    if (pwm >= 0) {
        PWM_M2_A = pwm;
        PWM_M2_B = 0;
    }
    else {
        PWM_M2_A = 0;
        PWM_M2_B = -pwm;
    }
}

static void _drivers_set_rear_left_pwm(int32_t pwm)
{
    if (pwm >= 0) {
        PWM_M3_A = pwm;
        PWM_M3_B = 0;
    }
    else {
        PWM_M3_A = 0;
        PWM_M3_B = -pwm;
    }
}

static void _drivers_set_rear_right_pwm(int32_t pwm)
{
    if (pwm >= 0) {
        PWM_M4_A = pwm;
        PWM_M4_B = 0;
    }
    else {
        PWM_M4_A = 0;
        PWM_M4_B = -pwm;
    }
}

static void _drivers_set_speeds(int32_t fl, int32_t fr, int32_t rl, int32_t rr)
{
    _drivers_set_front_left_pwm(fl);
    _drivers_set_front_right_pwm(fr);
    _drivers_set_rear_left_pwm(rl);
    _drivers_set_rear_right_pwm(rr);
}
