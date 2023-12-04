#pragma once

#include <stdint.h>

void bsp_uart_init();

void bsp_uart_complete_process();

uint8_t bsp_uart_get_cmd();

void *bsp_uart_get_data();

uint8_t bsp_uart_get_payload_size();

void bsp_uart_send_data(void* data, uint16_t data_size);

void _bsp_uart_print(char* text);
