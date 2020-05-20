/* 
 *
 *2018 09 21 & hxdyxd
 *
 */

#include <stdio.h>
#include <string.h>

#include "data_interface_hal.h"



/* some low level platform function */
/* public hal function */

void data_interface_hal_init(void)
{
    sine_wave_table_init();
    sine_wave_set_table(950);
    sine_wave_set_status(1);
    
    lcd1602_init();
    //key_inout_init();
    hal32_usart_init();
    hal32_adc_init();
    //fsk communication
    fsk_comm_init();
}




/******************* (C) COPYRIGHT 2018 hxdyxd *****END OF FILE****/
