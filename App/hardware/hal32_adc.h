/* 
 *
 *2019 08 04 & hxdyxd
 *
 */

#ifndef _HAL32_ADC_H
#define _HAL32_ADC_H

#include "adc.h"
#include "tim.h"



#define ADC_DUAL_BUFFER      (1)

/* ADC */
#define ADC_16BIT_VOLTAGE_CALCULATE(v)    ((v)*3.3/0x10000)
#define ADC_12BIT_VOLTAGE_CALCULATE(v)    ((v)*3.3/0x1000)

#define ADC1_CONV_NUMBER      (256)
#define ADC1_CHANNEL_NUMBER   (2)
#define ADC1_BUFFER_SIZE     (ADC1_CONV_NUMBER*ADC1_CHANNEL_NUMBER*ADC_DUAL_BUFFER)


typedef uint16_t (*ADC1_PBUF_TYPE)[ADC1_CONV_NUMBER][ADC1_CHANNEL_NUMBER];

typedef void (*hal32_adc_cb_t)(int id, void *pbuf, int len);

void hal32_adc_init(void);
void adc_rx_proc(hal32_adc_cb_t func_cb);

#endif
/******************* (C) COPYRIGHT 2019 hxdyxd *****END OF FILE****/
