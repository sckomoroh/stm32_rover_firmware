/*
 * bsp_mpuiic.c
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#include "bsp_mpuiic.h"

// Microsecond delay
static void Delay_For_Pin(uint8_t nCount) {
	uint8_t i = 0;
	for (; nCount != 0; nCount--) {
		for (i = 0; i < 10; i++)
			;
	}
}

#define delay_us  Delay_For_Pin

// Initialize the IIC
void mpu_iic_init(void) {
	delay_us(1);
}

// Generates the IIC initiation signal
void mpu_iic_start(void) {
	MPU_SDA_OUT();
	MPU_IIC_SDA(1);
	MPU_IIC_SCL(1);
	delay_us(4);
	MPU_IIC_SDA(0);
	delay_us(4);
	MPU_IIC_SCL(0);
}

// Generates an IIC stop signal
void mpu_iic_stop(void) {
	MPU_SDA_OUT();
	MPU_IIC_SCL(0);
	MPU_IIC_SDA(0);
	delay_us(4);
	MPU_IIC_SCL(1);
	MPU_IIC_SDA(1);
	delay_us(4);
}

// Wait for the answer signal to arrive.
// Return value: 1, receive and reply failed 0, receive and reply succeeded
uint8_t mpu_iic_wait_ack(void) {
	uint8_t ucErrTime = 0;
	MPU_SDA_IN();
	MPU_IIC_SDA(1);
	delay_us(1);
	MPU_IIC_SCL(1);
	delay_us(1);
	while (READ_SDA) {
		ucErrTime++;
		if (ucErrTime > 250) {
			mpu_iic_stop();
			return 1;
		}
	}
	MPU_IIC_SCL(0);
	return 0;
}

// Generate AN ACK reply
void mpu_iic_ack(void) {
	MPU_IIC_SCL(0);
	MPU_SDA_OUT();
	MPU_IIC_SDA(0);
	delay_us(2);
	MPU_IIC_SCL(1);
	delay_us(2);
	MPU_IIC_SCL(0);
}

// No ACK response is generated
void mpu_iic_no_ack(void) {
	MPU_IIC_SCL(0);
	MPU_SDA_OUT();
	MPU_IIC_SDA(1);
	delay_us(2);
	MPU_IIC_SCL(1);
	delay_us(2);
	MPU_IIC_SCL(0);
}

// The IIC sends a byte that returns whether the slave machine answered, 1, yes, 0, no
void mpu_iic_send_byte(uint8_t txd) {
	uint8_t t;
	MPU_SDA_OUT();
	MPU_IIC_SCL(0);
	for (t = 0; t < 8; t++) {
		MPU_IIC_SDA((txd & 0x80) >> 7);
		txd <<= 1;
		delay_us(2);
		MPU_IIC_SCL(1);
		delay_us(2);
		MPU_IIC_SCL(0);
		delay_us(2);
	}
}

// Read 1 byte, ack=1, send ACK, ack=0, send nACK
uint8_t mpu_iic_read_byte(unsigned char ack) {
	unsigned char i, receive = 0;
	MPU_SDA_IN();
	for (i = 0; i < 8; i++) {
		MPU_IIC_SCL(0);
		delay_us(2);
		MPU_IIC_SCL(1);
		receive <<= 1;
		if (READ_SDA)
			receive++;
		delay_us(1);
	}
	if (!ack)
		mpu_iic_no_ack();
	else
		mpu_iic_ack();
	return receive;
}

