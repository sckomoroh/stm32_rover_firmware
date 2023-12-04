#include "bsp_io_i2c.h"

static void Delay_For_Pin(uint8_t nCount) {
	uint8_t i = 0;
	for (; nCount != 0; nCount--) {
		for (i = 0; i < 10; i++)
			;
	}
}

#define delay_us  Delay_For_Pin

void iic_init(void) {
	delay_us(1);
}

void iic_start(void) {
	SDA_OUT();
	IIC_SDA(1);
	IIC_SCL(1);
	delay_us(4);
	IIC_SDA(0);
	delay_us(4);
	IIC_SCL(0);
}

void iic_stop(void) {
	SDA_OUT();
	IIC_SCL(0);
	IIC_SDA(0);
	delay_us(4);
	IIC_SCL(1);
	IIC_SDA(1);
	delay_us(4);
}

uint8_t iic_wait_ack(void) {
	uint8_t ucErrTime = 0;
	SDA_IN();
	IIC_SDA(1);
	delay_us(1);
	IIC_SCL(1);
	delay_us(1);
	while (READ_SDA) {
		ucErrTime++;
		if (ucErrTime > 250) {
			iic_stop();
			return 1;
		}
	}
	IIC_SCL(0);
	return 0;
}

void iic_ack(void) {
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(0);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}

void iic_no_ack(void) {
	IIC_SCL(0);
	SDA_OUT();
	IIC_SDA(1);
	delay_us(2);
	IIC_SCL(1);
	delay_us(2);
	IIC_SCL(0);
}

void iic_send_byte(uint8_t txd) {
	uint8_t t;
	SDA_OUT();
	IIC_SCL(0);
	for (t = 0; t < 8; t++) {
		IIC_SDA((txd & 0x80) >> 7);
		txd <<= 1;
		delay_us(2);
		IIC_SCL(1);
		delay_us(2);
		IIC_SCL(0);
		delay_us(2);
	}
}

uint8_t iic_read_byte(unsigned char ack) {
	unsigned char i, receive = 0;
	SDA_IN();
	for (i = 0; i < 8; i++) {
		IIC_SCL(0);
		delay_us(2);
		IIC_SCL(1);
		receive <<= 1;
		if (READ_SDA)
			receive++;
		delay_us(1);
	}
	if (!ack)
		iic_no_ack();
	else
		iic_ack();
	return receive;
}

uint8_t iic_write_byte(uint8_t addr, uint8_t reg, uint8_t data) {
	iic_start();
	iic_send_byte((addr << 1) | 0);
	if (iic_wait_ack()) {
		iic_stop();
		return 1;
	}
	iic_send_byte(reg);
	iic_wait_ack();
	iic_send_byte(data);
	if (iic_wait_ack()) {
		iic_stop();
		return 1;
	}
	iic_stop();
	return 0;
}

uint8_t iic_write_len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf) {
	uint8_t i;
	iic_start();
	iic_send_byte((addr << 1) | 0);
	if (iic_wait_ack()) {
		iic_stop();
		return 1;
	}
	iic_send_byte(reg);
	iic_wait_ack();
	for (i = 0; i < len; i++) {
		iic_send_byte(buf[i]);
		if (iic_wait_ack()) {
			iic_stop();
			return 1;
		}
	}
	iic_stop();
	return 0;
}

