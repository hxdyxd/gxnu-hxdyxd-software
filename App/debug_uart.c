/* 2019 04 10 */
/* By hxdyxd */

#include <stdio.h>
#include <time.h>

#include "usart.h"
#include "data_interface_hal.h"

void debug_hex(uint8_t *p, uint8_t len, uint8_t lf)
{
    int i;
    for(i=0;i<len;i++) {
        printf("%02x", *p );
        p++;
    }
    if(lf) {
        printf("\n");
    }
}


#if 1
int stdout_putchar (int ch)
{
    uint8_t c = ch;
    if(c == '\n') {
        c = '\r';
        HAL_UART_Transmit(&huart2, &c, 1, 20);
        c = ch;
    }
    HAL_UART_Transmit(&huart2, &c, 1, 20);
    return ch;
}

int stderr_putchar (int ch)
{
    return ch;
}
#endif


clock_t clock(void)
{
    return hal_read_TickCounter();
}

time_t time(time_t *time)
{
    return (hal_read_TickCounter()/1000);
}

void exit(int status)
{
    
}

int system(const char * cmdstring)
{
    return 0;
}

