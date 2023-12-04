/*
 * bsp_io_i2c.h
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#ifndef OLED_BSP_IO_I2C_H_
#define OLED_BSP_IO_I2C_H_

#include <stdint.h>

#include "main.h"

#define SDA_IN()                     \
    {                                    \
        GPIOB->CRH &= 0XFFFF0FFF;        \
        GPIOB->CRH |= (uint32_t)8 << 12; \
    }
#define SDA_OUT()                    \
    {                                    \
        GPIOB->CRH &= 0XFFFF0FFF;        \
        GPIOB->CRH |= (uint32_t)3 << 12; \
    }

#define IIC_SCL(a) HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, a)
#define IIC_SDA(a) HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, a)
#define READ_SDA HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin)

void iic_delay(void);
void iic_init(void);
void iic_start(void);
void iic_stop(void);
void iic_send_byte(uint8_t txd);
uint8_t iic_read_byte(unsigned char ack);
uint8_t iic_wait_ack(void);
void iic_ack(void);
void iic_no_ack(void);

uint8_t iic_write_byte(uint8_t addr, uint8_t reg, uint8_t data);
uint8_t iic_write_len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);

#endif /* OLED_BSP_IO_I2C_H_ */
