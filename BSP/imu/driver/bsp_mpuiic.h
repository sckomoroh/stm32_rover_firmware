/*
 * bsp_mpuiic.h
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#ifndef IMU_BSP_MPUIIC_H_
#define IMU_BSP_MPUIIC_H_

#include "main.h"

// SCL PB13, SDA PB15
#define MPU_SDA_IN()                     \
    {                                    \
        GPIOB->CRH &= 0X0FFFFFFF;        \
        GPIOB->CRH |= (uint32_t)8 << 28; \
    }
#define MPU_SDA_OUT()                    \
    {                                    \
        GPIOB->CRH &= 0X0FFFFFFF;        \
        GPIOB->CRH |= (uint32_t)3 << 28; \
    }

#define MPU_IIC_SCL(a) HAL_GPIO_WritePin(MPU_SCL_GPIO_Port, MPU_SCL_Pin, a)
#define MPU_IIC_SDA(a) HAL_GPIO_WritePin(MPU_SDA_GPIO_Port, MPU_SDA_Pin, a)
#define READ_SDA HAL_GPIO_ReadPin(MPU_SDA_GPIO_Port, MPU_SDA_Pin)

void mpu_iic_delay(void);
void mpu_iic_init(void);
void mpu_iic_start(void);
void mpu_iic_stop(void);
void mpu_iic_send_byte(uint8_t txd);
uint8_t mpu_iic_read_byte(unsigned char ack);
uint8_t mpu_iic_wait_ack(void);
void mpu_iic_ack(void);
void mpu_iic_no_ack(void);

#endif /* IMU_BSP_MPUIIC_H_ */
