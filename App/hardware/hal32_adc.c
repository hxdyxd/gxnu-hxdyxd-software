/* 
 *
 *2019 08 04 & hxdyxd
 *
 */


#include "hal32_adc.h"
#include <string.h>

#include "app_debug.h"
#define HAL32_ADC_DBG  APP_DEBUG
#define HAL32_ADC_ERR  APP_ERROR


/*********************************ADC**************************************************/

static uint16_t adc1_dma_buffer[ADC1_BUFFER_SIZE];

static volatile uint8_t adc1_ok = 0;



void adc_rx_proc(hal32_adc_cb_t func_cb)
{
    if(adc1_ok) {
        if(func_cb) {
            func_cb(1, (void *)adc1_dma_buffer, sizeof(adc1_dma_buffer));
        }
        adc1_ok = 0;
    }
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc == &hadc1) {
        adc1_ok = 2;
    } else {
        HAL32_ADC_ERR("unknow ConvCplt\r\n");
    }
}


#if 0
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc == &hadc1) {
        if(adc1_ok) {
            printf("ovr\r\n");
        }
        adc1_ok = 1;
    } else if(hadc == &hadc3) {
        if(adc3_ok) {
            printf("ovr\r\n");
        }
        adc3_ok = 1;
    } else {
        printf("unknow ConvCplt\r\n");
    }
}
#endif


void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc)
{
    printf("LevelOut\r\n");
}


void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    printf("---------------HAL_ADC_ErrorCallback----------\r\n");
    if(hadc == &hadc1) {
        adc1_ok = 1;
    }
}

/*********************************ADC**************************************************/




void hal32_adc_init(void)
{
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) {
        HAL32_ADC_ERR("adc1 calibration error\r\n");
    }
    
    
    memset( (void *)adc1_dma_buffer, 0, sizeof(adc1_dma_buffer));
    
    if(HAL_ADC_Start_DMA(&hadc1, (void *)adc1_dma_buffer, ADC1_BUFFER_SIZE) == HAL_OK) {
        HAL32_ADC_DBG("start adc1 dma at 0x%p %p\r\n", adc1_dma_buffer, &hadc1);
    } else {
        HAL32_ADC_ERR("start adc1 dma error %d\r\n", HAL_ADC_GetError(&hadc1));
    }
    
    //HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    htim1.Instance->CCR3 = 256;
}

/******************* (C) COPYRIGHT 2019 hxdyxd *****END OF FILE****/
