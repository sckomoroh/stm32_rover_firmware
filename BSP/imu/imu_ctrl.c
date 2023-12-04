/*
 *  Created on: Dec 4, 2023
 *      Author: Yehor Zvihunov
 */

#include "imu_ctrl.h"

#include <math.h>

#include "driver/bsp_mpu9250.h"
#include "uart/uart_ctrl.h"

struct {
    float accel[3];
    float gyro[3];
} _imu_full_data;

struct {
    int16_t accel[3];
    int16_t gyro[3];
} _imu_raw_data;

enum {
    IMU_X = 0,
    IMU_Y = 1,
    IMU_Z = 2,
};

#define GRAVITY 9.81

static void _process_gyro_data();
static void _process_accel_data();

void bsp_imu_init() { mpu9250_init(); }

void bsp_imu_process_cmd()
{
    _process_gyro_data();
    _process_accel_data();

    bsp_uart_send_data(&_imu_full_data, sizeof(_imu_full_data));
}

/*
+-------------------+   +-----------------------+
| Gyroscope sensity |   | Accelerometer sensity |
+-------------------+   +-----------------------+
| 0 | 250   | 131   |   | 0 | 2      | 16384    |
+---+-------+-------+   +---+--------+----------+
| 1 | 500   | 65.5  |   | 1 | 4      | 8192     |
+---+-------+-------+   +---+--------+----------+
| 2 | 1000  | 32.8  |   | 2 | 8      | 4096     |
+---+-------+-------+   +---+--------+----------+
| 3 | 2000  | 16.4  |   | 3 | 16     | 2048     |
+---+-------+-------+   +---+--------+----------+

gyro_data = sensor_data / div * PI / 180
accel_data = sensor_data / div * gravity (9.81)
*/

static void _process_gyro_data()
{
    static int16_t x, y, z;
    mpu_get_gyro(&x, &y, &z);

    // 65.5 - see hardcoded value in bsp_mpu9250.c in mpu_set_gyro_fsr and value in table
    _imu_full_data.gyro[IMU_X] = (float)x / 65.5 * M_PI / 180.0;
    _imu_full_data.gyro[IMU_Y] = (float)y / 65.5 * M_PI / 180.0;
    _imu_full_data.gyro[IMU_Z] = (float)z / 65.5 * M_PI / 180.0;
}

static void _process_accel_data()
{
    static int16_t x, y, z;
    mpu_get_accel(&x, &y, &z);

    // 16384 - see hardcoded value in bsp_mpu9250.c in mpu_set_accel_fsr and value in table
    _imu_full_data.accel[IMU_X] = (float)x / 16384 * GRAVITY;
    _imu_full_data.accel[IMU_Y] = (float)y / 16384 * GRAVITY;
    _imu_full_data.accel[IMU_Z] = (float)z / 16384 * GRAVITY;
}
