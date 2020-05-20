/* 2019 04 10 */
/* By hxdyxd */

#ifndef _USER_MAIN_H_
#define _USER_MAIN_H_

#include <stdint.h>
#include "adc_algorithm.h"


#define SYNC_TIMEOUT   (100)


#define LCD_PAGE_WELCOME        (0)
#define LCD_PAGE_OUTPUT_INFO    (1)
#define LCD_PAGE_SET            (2)
#define LCD_PAGE_CAL            (3)
#define LCD_PAGE_DUTY           (4)
#define LCD_PAGE_PKT            (5)
#define LCD_PAGE_VER            (6)
#define LCD_PAGE_NUM            (7)



#define LCD_PAGE_SET_DEFAULT   (0)
#define LCD_PAGE_SET_VOLTAGE   (1)
#define LCD_PAGE_SET_CURRENT   (2)
#define LCD_PAGE_SET_NUM       (3)


#define LCD_PAGE_CAL_DEFAULT   (0)
#define LCD_PAGE_CAL_CH        (1)
#define LCD_PAGE_CAL_INDEX     (2)
#define LCD_PAGE_CAL_OK        (3)
#define LCD_PAGE_CAL_NUM       (4)


#define LCD_PAGE_DUTY_DEFAULT   (0)
#define LCD_PAGE_DUTY_DIFF      (1)
#define LCD_PAGE_DUTY_NUM       (2)


struct gData_t {
    uint32_t rx_pack_cnt;
    uint32_t adc_cnt;
    uint8_t lcd_page;
    uint8_t lcd_page_set;
    pidc_t voltage_controller;
    pidc_t current_controller;
    
    uint16_t dyty_cycle_max;
    uint16_t dyty_cycle_min;
    float output_max;
    float output_min;
    
    uint8_t is_master;
    uint8_t is_sync;
    uint8_t is_output;
	uint8_t is_voltage;
    uint32_t sync_keep_time;
	float voltage_set;
	float current_set;
    
    uint8_t lcd_page_duty;
    
    //CAL
    uint8_t lcd_page_cal;
    uint8_t lcd_cal_ch;
    uint8_t lcd_cal_index;
    float voltage_cal_set;
    float para_x_div[PARA_NUM];
    float para_y_div[PARA_NUM];

//    MATLAB DEBUG
//    uint16_t adc_voltage_buffer[ADC1_CONV_NUMBER];
//    uint16_t adc_current_buffer[ADC1_CONV_NUMBER];
//    uint8_t adc_buffer_ready;

};


void user_system_setup(void);
void user_setup(void);
void user_loop(void);


#endif
/*****************************END OF FILE***************************/
