/*
 * bsp_mpu9250.c
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#include "bsp_mpu9250.h"
#include "main.h"
#include "bsp_mpuiic.h"

void MPU_ADDR_CTRL(void) {
	HAL_GPIO_WritePin(MPU_AD0_GPIO_Port, MPU_AD0_Pin, GPIO_PIN_RESET);
}

uint8_t mpu9250_init(void) {
	MPU_ADDR_CTRL();
	mpu_iic_init();
	mpu_delay_ms(10);

	uint8_t res = 0;

	mpu_write_byte(MPU9250_ADDR, MPU_PWR_MGMT1_REG, 0X80);
	mpu_delay_ms(100);
	mpu_write_byte(MPU9250_ADDR, MPU_PWR_MGMT1_REG, 0X00);

	// Gyroscope sensor  ±500dps=±500°/s ±32768 (gyro/32768*500)*PI/180(rad/s)=gyro/3754.9(rad/s)
	mpu_set_gyro_fsr(1);
	// Acceleration sensor ±2g=±2*9.8m/s^2 ±32768 accel/32768*19.6=accel/1671.84
	mpu_set_accel_fsr(0);
	// Set the sampling rate to 50Hz
	mpu_set_rate(50);

	// Turn off all interrupts
	mpu_write_byte(MPU9250_ADDR, MPU_INT_EN_REG, 0X00);
	// The I2C main mode is off
	mpu_write_byte(MPU9250_ADDR, MPU_USER_CTRL_REG, 0X00);
	// Close the FIFO
	mpu_write_byte(MPU9250_ADDR, MPU_FIFO_EN_REG, 0X00);
	// The INT pin is low, enabling bypass mode to read the magnetometer directly
	mpu_write_byte(MPU9250_ADDR, MPU_INTBP_CFG_REG, 0X82);
	// Read the ID of MPU9250
	res = mpu_read_byte(MPU9250_ADDR, MPU_DEVICE_ID_REG);
	// printf("MPU6500 Read ID=0x%02X\n", res);
	// Check whether the device ID is correct
	if (res == MPU6500_ID1 || res == MPU6500_ID2) {
		// Set CLKSEL,PLL X axis as reference
		mpu_write_byte(MPU9250_ADDR, MPU_PWR_MGMT1_REG, 0X01);
		// Acceleration and gyroscope both work
		mpu_write_byte(MPU9250_ADDR, MPU_PWR_MGMT2_REG, 0X00);
		// Set the sampling rate to 50Hz
		mpu_set_rate(50);
	} else {
		return 1;
	}

	return 0;
}

// Set the full range of the MPU9250 gyroscope sensor:fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
// return value :0, the setting succeeds, other, the setting fails
uint8_t mpu_set_gyro_fsr(uint8_t fsr) {
	return mpu_write_byte(MPU9250_ADDR, MPU_GYRO_CFG_REG, fsr << 3); //设置陀螺仪满量程范围
}

// Set the full range of the MPU9250 acceleration sensor: FSR :0,±2G; 1,±4g; 2,±8g; 3,±16g
// return value :0, the setting succeeds, other, the setting fails
uint8_t mpu_set_accel_fsr(uint8_t fsr) {
	return mpu_write_byte(MPU9250_ADDR, MPU_ACCEL_CFG_REG, fsr << 3);
}

// Set the digital low-pass filter of the MPU9250. The return value is 0. The setting succeeds
uint8_t MPU_Set_LPF(uint16_t lpf) {
	uint8_t data = 0;
	if (lpf >= 188) {
		data = 1;
	} else if (lpf >= 98) {
		data = 2;
	} else if (lpf >= 42) {
		data = 3;
	} else if (lpf >= 20) {
		data = 4;
	} else if (lpf >= 10) {
		data = 5;
	} else {
		data = 6;
	}

	return mpu_write_byte(MPU9250_ADDR, MPU_CFG_REG, data); //设置数字低通滤波器
}

// Set the sampling rate of MPU9250 (assuming Fs=1KHz), rate:4~1000(Hz),
// return value :0, the setting succeeds, other, the setting fails
uint8_t mpu_set_rate(uint16_t rate) {
	uint8_t data;
	if (rate > 1000) {
		rate = 1000;
	}

	if (rate < 4) {
		rate = 4;
	}

	data = 1000 / rate - 1;
	data = mpu_write_byte(MPU9250_ADDR, MPU_SAMPLE_RATE_REG, data);

	return MPU_Set_LPF(rate / 2);
}

// Read gyroscope value (original value), return value :0, success, other, error code
uint8_t mpu_get_gyro(int16_t *gx, int16_t *gy, int16_t *gz) {
	uint8_t buf[6], res;
	res = mpu_read_len(MPU9250_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);
	if (res == 0) {
		*gx = ((uint16_t) buf[0] << 8) | buf[1];
		*gy = ((uint16_t) buf[2] << 8) | buf[3];
		*gz = ((uint16_t) buf[4] << 8) | buf[5];
	}

	return res;
}

// Read acceleration value (original value), return value :0, success, other, error code
uint8_t mpu_get_accel(int16_t *ax, int16_t *ay, int16_t *az) {
	uint8_t buf[6], res;
	res = mpu_read_len(MPU9250_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);
	if (res == 0) {
		*ax = ((uint16_t) buf[0] << 8) | buf[1];
		*ay = ((uint16_t) buf[2] << 8) | buf[3];
		*az = ((uint16_t) buf[4] << 8) | buf[5];
	}

	return res;
}

// Millisecond delay function
void mpu_delay_ms(uint16_t time) {
	HAL_Delay(time);
}

// IIC continuous write, buF is the address of the data to be written.  Return value :0, normal, otherwise, error code
uint8_t mpu_write_len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf) {
	uint8_t i;
	mpu_iic_start();
	mpu_iic_send_byte((addr << 1) | 0);
	if (mpu_iic_wait_ack()) {
		mpu_iic_stop();
		return 1;
	}

	mpu_iic_send_byte(reg);
	mpu_iic_wait_ack();
	for (i = 0; i < len; i++) {
		mpu_iic_send_byte(buf[i]);
		if (mpu_iic_wait_ack()) {
			mpu_iic_stop();
			return 1;
		}
	}

	mpu_iic_stop();
	return 0;
}

// IIC reads continuously and saves data to BUF.  Return value :0, normal, otherwise, error code
uint8_t mpu_read_len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf) {
	mpu_iic_start();
	mpu_iic_send_byte((addr << 1) | 0);
	if (mpu_iic_wait_ack()) {
		mpu_iic_stop();
		return 1;
	}

	mpu_iic_send_byte(reg);
	mpu_iic_wait_ack();
	mpu_iic_start();
	mpu_iic_send_byte((addr << 1) | 1);
	mpu_iic_wait_ack();
	while (len) {
		if (len == 1) {
			*buf = mpu_iic_read_byte(0);
		}
		else {
			*buf = mpu_iic_read_byte(1);
		}

		len--;
		buf++;
	}
	mpu_iic_stop();
	return 0;
}

// IIC writes a byte, return value :0, normal, otherwise, error code
uint8_t mpu_write_byte(uint8_t addr, uint8_t reg, uint8_t data) {
	mpu_iic_start();
	mpu_iic_send_byte((addr << 1) | 0);
	if (mpu_iic_wait_ack()) {
		mpu_iic_stop();

		return 1;
	}

	mpu_iic_send_byte(reg);
	mpu_iic_wait_ack();
	mpu_iic_send_byte(data);
	if (mpu_iic_wait_ack()) {
		mpu_iic_stop();

		return 1;
	}

	mpu_iic_stop();

	return 0;
}

// IIC reads one byte and returns the read data
uint8_t mpu_read_byte(uint8_t addr, uint8_t reg) {
	uint8_t res;
	mpu_iic_start();
	mpu_iic_send_byte((addr << 1) | 0);
	mpu_iic_wait_ack();
	mpu_iic_send_byte(reg);
	mpu_iic_wait_ack();
	mpu_iic_start();
	mpu_iic_send_byte((addr << 1) | 1);
	mpu_iic_wait_ack();
	res = mpu_iic_read_byte(0);
	mpu_iic_stop();

	return res;
}

