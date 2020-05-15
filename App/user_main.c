/* 2019 04 10 */
/* By hxdyxd */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "app_debug.h"
#include "user_main.h"
#include "function_task.h"
#include "soft_timer.h"
#include "adc_algorithm.h"

#define MASTER     (1)
//int cnt = 0;

struct gData_t{
    uint32_t rx_pack_cnt;
    uint8_t lcd_page;
    uint8_t lcd_page_set;
    pidc_t voltage_controller;
    pidc_t current_controller;
    uint16_t dyty_cycle_max;
    uint16_t dyty_cycle_min;
    uint8_t is_master;
    uint8_t is_sync;
    uint8_t is_output;
	uint8_t is_voltage;
    uint32_t sync_keep_time;
	float voltage_set;
	float current_set;
//    uint16_t adc_voltage_buffer[ADC1_CONV_NUMBER];
//    uint16_t adc_current_buffer[ADC1_CONV_NUMBER];
//    uint8_t adc_buffer_ready;
    
} global_data = {
    .rx_pack_cnt = 0,
    .lcd_page = LCD_PAGE_OUTPUT_INFO,
    .dyty_cycle_max = 0,
    .dyty_cycle_min = PWM_MAX_V,
#if MASTER
    .is_master = 1,
    .is_sync = 0,
    .is_output = 0,
	.is_voltage = 0,
#else
    .is_master = 0,
    .is_sync = 0,
    .is_output = 0,
	.is_voltage = 0,
#endif
    .sync_keep_time = 0,
	.voltage_set = 5.0,
	.current_set = 0.4,
//    .adc_buffer_ready = 0,
    .voltage_controller = {
        .kp = 10.0,
        .ki = 0,
        .kd = 1,
        .i_max = 0,
        .setval = 6.0,
        .max_output = PWM_MAX_V,
        .min_output = 0,
    },
    .current_controller = {
        .kp = 50.0,
        .ki = 0,
        .kd = 1,
        .i_max = 0,
        .setval = 0,
        .max_output = PWM_MAX_V,
        .min_output = 0,
    },
};
uint8_t lost_cnt = 0;


void pid_controller_proc(void)
{
    float current_val;
    pidc_t *pcontroller = NULL;
    
    if(global_data.is_voltage) {
        current_val = GET_ADC(VRMS);
        pcontroller = &global_data.voltage_controller;
    } else {
        current_val = GET_ADC(IRMS);
        pcontroller = &global_data.current_controller;
    }
    
    
    uint16_t output = pid_ctrl(pcontroller, current_val);
    sine_wave_set_table(output);
    
    if(output > global_data.dyty_cycle_max)
        global_data.dyty_cycle_max = output;
    if(output < global_data.dyty_cycle_min)
        global_data.dyty_cycle_min = output;
}



void lcd_flush_proc(void)
{
    switch(global_data.lcd_page) {
    case LCD_PAGE_OUTPUT_INFO:
        for(int i=0;i<ADC1_CHANNEL_NUMBER;i++) {
            if(!global_data.is_master && i==IRMS) {
                lcd_printf(0, i, "%s:%.3f_%.3f", GET_ADC_INFO(i), GET_ADC(i), global_data.current_controller.setval);
            } else {
                lcd_printf(0, i, "%s:%.3f", GET_ADC_INFO(i), GET_ADC(i) );
            }
        }
        break;
	case LCD_PAGE_SET:
        switch(global_data.lcd_page_set) {
        case LCD_PAGE_SET_DEFAULT:
            lcd_printf(0, 0, "Iset:%.3f", global_data.current_set );
            lcd_printf(0, 1, "Vset:%.3f", global_data.voltage_set );
            break;
        case LCD_PAGE_SET_VOLTAGE:
            lcd_printf(0, 0, "V set mode");
            lcd_printf(0, 1, "Vset:%.3f", global_data.voltage_set );
            break;
        case LCD_PAGE_SET_CURRENT:
            lcd_printf(0, 0, "I set mode");
            lcd_printf(0, 1, "Iset:%.3f", global_data.current_set );
            break;
        default:
            lcd1602_clear();
            break;
        }
		break;
    case LCD_PAGE_PKT:
        lcd1602_clear();
        lcd_printf(0, 0, "PKT:%d", lost_cnt);
        break;
    default:
        lcd1602_clear();
        break;
    }
    uint8_t icon = 15;
		if(global_data.is_voltage) {
				lcd1602_write_char(icon--, 0, 'V');
		} else {
				lcd1602_write_char(icon--, 0, 'A');
		}
		
    if(global_data.is_master) {
        lcd1602_write_char(icon--, 0, 'M');
    } else {
        lcd1602_write_char(icon--, 0, 'S');
        if(global_data.is_sync)
            lcd1602_write_char(icon--, 0, 'T');
    }
    if(global_data.is_output) {
        lcd1602_write_char(icon--, 0, 'O');
    }
    
    APP_DEBUG("ADC_INFO ");
    for(int i=0;i<ADC1_CHANNEL_NUMBER;i++) {
        if(!global_data.is_master && i==IRMS) {
            printf(" %s:%.3f(%.3f) ", GET_ADC_INFO(i), GET_ADC(i), global_data.current_controller.setval);
        } else {
            printf(" %s:%.3f ", GET_ADC_INFO(i), GET_ADC(i));
        }
    }
    printf("\n");
    APP_DEBUG("duty: %d-%d\n", global_data.dyty_cycle_min, global_data.dyty_cycle_max);
    global_data.dyty_cycle_min = PWM_MAX_V;
    global_data.dyty_cycle_max = 0;
    
    APP_DEBUG("status:%s\n", (global_data.is_master)?"M":"S");
    
//    static uint32_t previous_cnt = 0;
//    
//    printf("%d\n", cnt - previous_cnt);
//    previous_cnt = cnt;
//    
//    if(global_data.adc_buffer_ready) {
//        for(int i=0;i<ADC1_CONV_NUMBER;i++) {
//            printf("%d, ", global_data.adc_voltage_buffer[i]);
//        }
//        printf("\n");
//        
//        global_data.adc_buffer_ready = 0;
//    }
    LED_REV(LED_BASE2);
}


void usart_rx_callback(uint8_t *data, uint8_t len)
{
    data[len] = 0;
    lcd1602_write_string(15, 1, (char *)data);
    switch(*data) {
    case 96:
        //ok
        switch(global_data.lcd_page) {
        case LCD_PAGE_SET:
            global_data.lcd_page_set = (global_data.lcd_page_set + 1) % LCD_PAGE_SET_NUM;
            break;
        default:
            break;
        }
        break;
    case 'a':
        //-
        switch(global_data.lcd_page) {
        case LCD_PAGE_SET:
            switch(global_data.lcd_page_set) {
            case LCD_PAGE_SET_VOLTAGE:
                global_data.voltage_set -= 1;
                break;
            case LCD_PAGE_SET_CURRENT:
                global_data.current_set -= 0.1;
                break;
            }
            break;
        default:
            break;
        }
        break;
	case 'b':
        //+
        switch(global_data.lcd_page) {
        case LCD_PAGE_SET:
            switch(global_data.lcd_page_set) {
            case LCD_PAGE_SET_VOLTAGE:
                global_data.voltage_set += 1;
                break;
            case LCD_PAGE_SET_CURRENT:
                global_data.current_set += 0.1;
                break;
            }
            break;
        default:
            break;
        }
        break;
    case 'c':
        //output set
        global_data.is_output = !global_data.is_output;
        if(global_data.is_output)
            LED_HIGH(RELAY_OUT);
        else
            LED_LOW(RELAY_OUT);
        break;
    case 'd':
        //page next
        global_data.lcd_page = (global_data.lcd_page + 1) % LCD_PAGE_NUM;
        switch(global_data.lcd_page) {
        case LCD_PAGE_SET:
            global_data.lcd_page_set = LCD_PAGE_SET_DEFAULT;
            break;
        default:
            break;
        }
        break;
    case 'e':
        //master slave set
        global_data.is_master = !global_data.is_master;
        fsk_comm_set_mode(global_data.is_master ? TX_FLAG : RX_FLAG);
        break;
    case 'f':
        //vlotage current set
		global_data.is_voltage = !global_data.is_voltage;
        break;
    default:
        break;
    }
}


void fsk_data_callback(uint8_t *data, uint16_t len)
{
    global_data.rx_pack_cnt++;
    LED_REV(LED_BASE);
    if(global_data.is_master) {
        memset(data, 0, len);
        uint16_t tmp = GET_ADC(IRMS)*1000;
        data[0] = tmp >> 8;
        data[1] = tmp & 0xff;
        tmp = GET_ADC(VRMS)*1000;
        data[2] = tmp >> 8;
        data[3] = tmp & 0xff;
				if(global_data.is_voltage)
						pid_set_value(&global_data.voltage_controller, global_data.voltage_set );
				else
						pid_set_value(&global_data.current_controller, global_data.current_set );
        //type: 
    } else {
        uint16_t tmp = 0;
        tmp = (data[0] << 8) | data[1];
				if(!global_data.is_voltage) {
						pid_set_value(&global_data.current_controller, tmp*0.001);
				}
        printf("Ims:%.3f\r\n", tmp*0.001);
				
        tmp = (data[2] << 8) | data[3];
				if(global_data.is_voltage) {
						pid_set_value(&global_data.voltage_controller, tmp*0.001);
				}
        printf("Vms:%.3f\r\n", tmp*0.001);
        global_data.sync_keep_time = hal_read_TickCounter();
    }
}


void adc_rx_callback(int id, void *pbuf, int len)
{
    ADC1_PBUF_TYPE adc1pbuf = (ADC1_PBUF_TYPE)pbuf;
    //Izero
    int zero = 0;
    for(int i=0;i<ADC1_CONV_NUMBER;i++) {
        zero += (*adc1pbuf)[i][1];
    }
    zero /= ADC1_CONV_NUMBER;
    //-----------------------------
    //Irms
    float sum = 0;
    for(int i=0;i<ADC1_CONV_NUMBER;i++) {
        int tmp = (*adc1pbuf)[i][1] - zero;
        sum += tmp * tmp;
    }
    float current_val = sqrt(sum/ADC1_CONV_NUMBER);
    //-----------------------------
    //Vrms
    sum = 0;
    for(int i=0;i<ADC1_CONV_NUMBER;i++) {
        int tmp = (*adc1pbuf)[i][0];
        sum += tmp * tmp;
    }
    float voltage_val = sqrt(sum*2/ADC1_CONV_NUMBER);
    
    value_adc_physical_set(ADC_12BIT_VOLTAGE_CALCULATE(current_val), IRMS);
    value_adc_physical_set(ADC_12BIT_VOLTAGE_CALCULATE(voltage_val), VRMS);
    
    //pid controller
    pid_controller_proc();
//    cnt++;
//    if(cnt%64 == 0) {
//        for(int i=0;i<ADC1_CONV_NUMBER;i++) {
//            global_data.adc_voltage_buffer[i] = (*adc1pbuf)[i][0];
//            global_data.adc_current_buffer[i] = (*adc1pbuf)[i][1];
//        }
//        global_data.adc_buffer_ready = 1;
//    }
    TIMER_TASK(timer0, SYNC_TIMEOUT/4, (!global_data.is_master) ) {
        if(SYNC_TIMEOUT < hal_read_TickCounter() - global_data.sync_keep_time) {
            global_data.is_sync = 0;
            pid_set_value(&global_data.current_controller, 0);
        } else {
            global_data.is_sync = 1;
        }
    }
}


void user_system_setup(void)
{
}

void user_setup(void)
{
    PRINTF("\r\n\r\n[Sine Wave Inverters] Build , %s %s \r\n", __DATE__, __TIME__);
    
    data_interface_hal_init();
    
    soft_timer_init();
    soft_timer_create(0, 1, 1, lcd_flush_proc, 500);
    
    lcd1602_write_string(0, 0, "hello");
    lcd1602_write_string(0, 1, "stm32");
    fsk_comm_set_mode(global_data.is_master ? TX_FLAG : RX_FLAG);
}


void user_loop(void)
{
    soft_timer_proc();
    usart_rx_proc(usart_rx_callback);
    fsk_data_proc(fsk_data_callback);
    adc_rx_proc(adc_rx_callback);
}


/*****************************END OF FILE***************************/