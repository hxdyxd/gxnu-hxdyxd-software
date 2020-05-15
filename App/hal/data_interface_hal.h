/* 
 *
 *2018 09 21 & hxdyxd
 *
 */


#ifndef __data_interface_hal_H__
#define __data_interface_hal_H__

#include <stdint.h>

#include "gpio.h"
#include "tim.h"
#include "tim.h"
#include "usart.h"
//#include "key_inout.h"
//#include "hal32_usart.h"
#include "sine_pwm_control.h"
#include "lcd1602.h"
#include "hal32_usart.h"
#include "spi_hal.h"
#include "hal32_adc.h"
#include "fsk_communication.h"

//UART
#define UART_BUFFER_SIZE     (256)
#define USART_RX_TIMEOUT_MS  (10)


//ENCODER
#define ENCODER_CNT    (TIM3->CNT)


/* LEDS */
#define LED_OFF(id)   HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_ON(id)    HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_HIGH(id)  HAL_GPIO_WritePin(id, GPIO_PIN_SET)
#define LED_LOW(id)   HAL_GPIO_WritePin(id, GPIO_PIN_RESET)
#define LED_REV(id)   HAL_GPIO_TogglePin(id)
#define GPIO_HIGH(id) LED_HIGH(id)
#define GPIO_LOW(id)  LED_LOW(id)
#define GPIO_READ(id)  (HAL_GPIO_ReadPin(id) == GPIO_PIN_SET)


#define LED_BASE       LED_BASE_GPIO_Port , LED_BASE_Pin
#define LED_BASE2      LED_BASE2_GPIO_Port , LED_BASE2_Pin
#define RELAY_OUT      RELAY_OUT_GPIO_Port , RELAY_OUT_Pin
#define LORA_CS        LORA_CS_GPIO_Port , LORA_CS_Pin
#define LORA_NRST      LORA_NRST_GPIO_Port , LORA_NRST_Pin
#define FSK_DATA       FSK_DATA_GPIO_Port , FSK_DATA_Pin



/*******************************************************************************
* Function Name  : data_interface_hal_init.
* Description    : Hardware adaptation layer initialization.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void data_interface_hal_init(void);


/*******************************************************************************
* Function Name  : hal_read_TickCounter.
* Description    : Hardware adaptation layer TICK get.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
#define hal_read_TickCounter() HAL_GetTick()


#endif
/******************* (C) COPYRIGHT 2018 hxdyxd *****END OF FILE****/
