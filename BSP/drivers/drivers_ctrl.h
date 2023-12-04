/*
 * bsp_drivers.h
 *
 *  Created on: Dec 1, 2023
 *      Author: yzvihunov
 */

#ifndef DRIVERS_BSP_DRIVERS_H_
#define DRIVERS_BSP_DRIVERS_H_

#include <stdint.h>

void bsp_drivers_init();
void bsp_drivers_process();
void bsp_drivers_process_encoders_cmd();
void bsp_drivers_process_speeds_cmd(void *data);

#endif /* DRIVERS_BSP_DRIVERS_H_ */
