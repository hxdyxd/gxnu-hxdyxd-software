/* 2019 04 10 */
/* By hxdyxd */

#include "hal32_usart.h"


/*********************************UART**************************************************/

static uint8_t usart_buffer[UART_BUFFER_SIZE + 1];
static volatile uint32_t usart_rx_timer = 0;
static volatile uint16_t usart_rx_counter = 0;
static volatile uint8_t USART_RX_FLAG = 0;


void usart_rx_proc( void (* usart_rx_callback)(uint8_t *, uint8_t) )
{
    if(usart_rx_counter != 0 && (hal_read_TickCounter() - usart_rx_timer) > USART_RX_TIMEOUT_MS ) {
        //timeout detect
        USART_RX_FLAG = 1;
        usart_rx_callback(usart_buffer, usart_rx_counter);
        usart_rx_counter = 0;
        USART_RX_FLAG = 0;
        HAL_UART_AbortReceive_IT(&huart2);
        HAL_UART_Receive_IT(&huart2, usart_buffer, 1);
        //APP_WARN("RXD\r\n");
    }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if( USART_RX_FLAG == 0 ) {
        if(usart_rx_counter != 0 && (hal_read_TickCounter() - usart_rx_timer) > USART_RX_TIMEOUT_MS ) {
            //timeout detect
            USART_RX_FLAG = 1;
        } else if(usart_rx_counter >= UART_BUFFER_SIZE) {
            //usart_rx_counter = 0;
            usart_rx_counter = 0;
            HAL_UART_AbortReceive_IT(&huart2);
            HAL_UART_Receive_IT(&huart2, usart_buffer, 1);
        } else {
            usart_rx_counter++;
            HAL_UART_Receive_IT(huart, usart_buffer+usart_rx_counter, 1);
            usart_rx_timer = hal_read_TickCounter();
        }
    }
}


void hal32_usart_init(void)
{
    //UART
    usart_rx_counter = 0;
    HAL_UART_Receive_IT(&huart2, usart_buffer, 1);
}


void hal32_usart_write(uint8_t *p, uint8_t len)
{
    static uint32_t timer = 0;
#if 1
    if(hal_read_TickCounter() - timer < 12)
        return;
#else
    while(hal_read_TickCounter() - timer < 12);
#endif
    HAL_UART_Transmit(&huart2, p, len, 100);
    timer = hal_read_TickCounter();
}


void hal32_usart3_write(uint8_t *p, uint8_t len)
{
    static uint32_t timer = 0;
#if 1
    if(hal_read_TickCounter() - timer < 12)
        return;
#else
    while(hal_read_TickCounter() - timer < 12);
#endif
    HAL_UART_Transmit(&huart3, p, len, 100);
    timer = hal_read_TickCounter();
}

/*********************************UART**************************************************/
/*****************************END OF FILE***************************/
