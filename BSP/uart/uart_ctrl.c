#include "uart_ctrl.h"

#include <stdlib.h>
#include <string.h>

#include "hw_interfaces.h"

#define UART_HEADER_BUFFER_SIZE 3
#define MAX_UART_BUFFER 64

typedef enum {
    URS_WAIT_HEADER,
    URS_WAIT_PAYLOAD,
    URS_WAIT_PROCESS,
} uart_receive_state_t;

struct {
    uint8_t buffer[MAX_UART_BUFFER];
    uart_receive_state_t state;
    uint8_t cmd;
    uint8_t payload_size;
} _uart_data;

static void _process_received_data();

static void _bsp_uart_send_u8(uint8_t b);

static void _bsp_uart_send_array_u8(uint8_t* buffer, uint16_t size);

uint8_t bsp_uart_get_payload_size() { return _uart_data.payload_size; }

void bsp_uart_init()
{
    _uart_data.cmd = 0;
    _uart_data.payload_size = 0;
    _uart_data.state = URS_WAIT_HEADER;

    HAL_UART_Receive_IT(&huart1, _uart_data.buffer, UART_HEADER_BUFFER_SIZE);
}

void bsp_uart_complete_process()
{
    _uart_data.state = URS_WAIT_HEADER;
    _uart_data.cmd = 0;

    HAL_UART_Receive_IT(&huart1, _uart_data.buffer, UART_HEADER_BUFFER_SIZE);
}

uint8_t bsp_uart_get_cmd() { return _uart_data.cmd; }

void* bsp_uart_get_data() { return _uart_data.buffer; }

void _bsp_uart_print(char* text)
{
	_bsp_uart_send_array_u8((uint8_t*)text, strlen(text));
	_bsp_uart_send_u8(10);
	_bsp_uart_send_u8(13);
}

void bsp_uart_send_data(void* data, uint16_t data_size)
{
    _bsp_uart_send_array_u8((uint8_t*)data, data_size);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    UNUSED(huart);

    _process_received_data();

    HAL_UART_Receive_IT(&huart1, _uart_data.buffer, UART_HEADER_BUFFER_SIZE);
}

static void _process_received_data()
{
    static void* buffer = NULL;

    switch (_uart_data.state) {
    case URS_WAIT_HEADER:
        if (_uart_data.buffer[0] == 'C') {
            _uart_data.cmd = _uart_data.buffer[1];
            _uart_data.payload_size = _uart_data.buffer[2];
            if (_uart_data.payload_size > 0) {
                _uart_data.state = URS_WAIT_PAYLOAD;
                HAL_UART_Receive_IT(&huart1, _uart_data.buffer, _uart_data.payload_size);
            }
            else {
                _uart_data.state = URS_WAIT_PROCESS;
            }
        }

        break;
    case URS_WAIT_PAYLOAD:
        buffer = malloc(_uart_data.payload_size);
        memcpy(buffer, _uart_data.buffer, _uart_data.payload_size);

        // for (size_t i = 0; i < size / 2; ++i)
        // {
        //     uint8_t temp = bytes[i];
        //     bytes[i] = bytes[size - 1 - i];
        //     bytes[size - 1 - i] = temp;
        // }

        _uart_data.state = URS_WAIT_PROCESS;
    }
}

static void _bsp_uart_send_u8(uint8_t b) { HAL_UART_Transmit(&huart1, (uint8_t*)&b, 1, 0xFFFF); }

static void _bsp_uart_send_array_u8(uint8_t* buffer, uint16_t size)
{
#if ENABLE_UART_DMA
    HAL_UART_Transmit_DMA(&huart1, buffer, size);
#else
    while (size--) {
    	_bsp_uart_send_u8(*buffer);
        buffer++;
    }
#endif  //  ENABLE_UART_DMA
}
