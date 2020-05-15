/* 2019 04 10 */
/* By hxdyxd */

#ifndef _hal32_usart_h
#define _hal32_usart_h

#include "data_interface_hal.h"

//UART
#define UART_BUFFER_SIZE     (256)
#define USART_RX_TIMEOUT_MS  (10)



void hal32_usart_init(void);
void hal32_usart_write(uint8_t *p, uint8_t len);
void hal32_usart3_write(uint8_t *p, uint8_t len);
void usart_rx_proc( void (* usart_rx_callback)(uint8_t *, uint8_t) );


#endif
/*****************************END OF FILE***************************/
