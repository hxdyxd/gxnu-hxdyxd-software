/* 2019 04 10 */
/* By hxdyxd */
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "app_debug.h"
#include "user_main.h"
#include "function_task.h"
#include "soft_timer.h"

#define MASTER     (1)
//int cnt = 0;
#define VOLTAGE_MAX   (15)
#define CURRENT_MAX   (2)

static struct gData_t global_data = {
    .rx_pack_cnt = 0,
    .lcd_page = LCD_PAGE_WELCOME,
    .lcd_page_duty = LCD_PAGE_DUTY_DEFAULT,
    .dyty_cycle_max = 0,
    .dyty_cycle_min = PWM_MAX_V,
    .output_max = 0,
    .output_min = 10000.0,
#if MASTER
    .is_master = 1,
    .is_sync = 0,
    .is_output = 0,
	.is_voltage = 1,
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

const float param_y_config[PARA_CHANNEL_NUMBER][PARA_NUM] = {
    {5, 10, 15,},
    {0.5, 1.0, 1.5,},
};

static void pid_controller_proc(void)
{
    static pidc_t *privous_pcontroller = &global_data.voltage_controller;
    float current_val;
    pidc_t *pcontroller = NULL;
    
    if(global_data.is_voltage || !global_data.is_output) {
        current_val = GET_ADC(VRMS);
        pcontroller = &global_data.voltage_controller;
    } else {
        current_val = GET_ADC(IRMS);
        pcontroller = &global_data.current_controller;
    }
    if(pcontroller != privous_pcontroller) {
        pcontroller->output = privous_pcontroller->output;
        privous_pcontroller = pcontroller;
    }
    
    uint16_t output = pid_ctrl(pcontroller, current_val);
    sine_wave_set_table(output);
    
    if(output > global_data.dyty_cycle_max)
        global_data.dyty_cycle_max = output;
    if(output < global_data.dyty_cycle_min)
        global_data.dyty_cycle_min = output;
    
    if(current_val > global_data.output_max)
        global_data.output_max = current_val;
    if(current_val < global_data.output_min)
        global_data.output_min = current_val;
}


static void set_pid_value_in_master_mode(void)
{
    if(global_data.is_master) {
        pid_set_value(&global_data.voltage_controller, global_data.voltage_set );
        pid_set_value(&global_data.current_controller, global_data.current_set );
    }
}

static void set_output_status(void)
{
    if(global_data.is_output)
        LED_HIGH(RELAY_OUT);
    else
        LED_LOW(RELAY_OUT);
}


/* output info */
static void lcd_flush_proc(void)
{
    uint8_t show_status = 1;
    switch(global_data.lcd_page) {
    case LCD_PAGE_WELCOME:
        lcd1602_write_string(0, 0, "SineWaveInverter");
        lcd1602_write_string(0, 1, "Yan_Xiangdong");
        show_status = 0;
        break;
    case LCD_PAGE_OUTPUT_INFO:
        for(int i=0;i<ADC1_CHANNEL_NUMBER;i++) {
            if(!global_data.is_master && i == IRMS) {
                if(global_data.is_voltage) {
                    lcd_printf(0, i, "%s:%.3f_%.3f", GET_ADC_INFO(i), GET_ADC(i), global_data.voltage_controller.setval);
                } else {
                    lcd_printf(0, i, "%s:%.3f_%.3f", GET_ADC_INFO(i), GET_ADC(i), global_data.current_controller.setval);
                }
            } else {
                lcd_printf(0, i, "%s:%.3f", GET_ADC_INFO(i), GET_ADC(i) );
            }
        }
        break;
	case LCD_PAGE_SET:
        switch(global_data.lcd_page_set) {
        case LCD_PAGE_SET_DEFAULT:
            lcd_printf(0, 0, "Vset:%.3f", global_data.voltage_set );
            lcd_printf(0, 1, "Iset:%.3f", global_data.current_set );
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
    case LCD_PAGE_CAL:
        switch(global_data.lcd_page_cal) {
        case LCD_PAGE_CAL_DEFAULT:
            lcd_printf(0, 0, "CAL_%s:%.3f", GET_ADC_INFO(0), GET_ADC(0));
            lcd_printf(0, 1, "CAL_%s:%.3f", GET_ADC_INFO(1), GET_ADC(1));
            break;
        case LCD_PAGE_CAL_CH:
            lcd_printf(0, 0, "CAL_CH:%s.%d", GET_ADC_INFO(global_data.lcd_cal_ch), global_data.lcd_cal_ch);
            lcd_printf(0, 1, "K:%.2f, B:%.2f", 
                gs_para[global_data.lcd_cal_ch].k,
                gs_para[global_data.lcd_cal_ch].b
            );
            
            break;
        case LCD_PAGE_CAL_INDEX:
            lcd_printf(0, 0, "CAL:%s,To %.2f", GET_ADC_INFO(global_data.lcd_cal_ch), global_data.para_y_div[global_data.lcd_cal_index]);
            lcd_printf(0, 1, "CAL_%s:%.3f", GET_ADC_INFO(global_data.lcd_cal_ch), GET_ADC(global_data.lcd_cal_ch));
            break;
        case LCD_PAGE_CAL_OK:
            lcd_printf(0, 0, "CAL_%s_OK:%.3f", GET_ADC_INFO(global_data.lcd_cal_ch), GET_ADC(global_data.lcd_cal_ch) );
            lcd_printf(0, 1, "K:%.2f, B:%.2f", 
                gs_para[global_data.lcd_cal_ch].k,
                gs_para[global_data.lcd_cal_ch].b
            );
            break;
        default:
            lcd1602_clear();
            break;
        }
        show_status = 0;
        break;
    case LCD_PAGE_DUTY:
        switch(global_data.lcd_page_duty) {
        case LCD_PAGE_DUTY_DEFAULT:
            lcd_printf(0, 0, "Duty:%d-%d", global_data.dyty_cycle_min, global_data.dyty_cycle_max);
            if(global_data.is_voltage) {
                lcd_printf(0, 1, "%s:%.3f-%.3f", GET_ADC_INFO(0), global_data.output_min, global_data.output_max);
            } else {
                lcd_printf(0, 1, "%s:%.3f-%.3f", GET_ADC_INFO(1), global_data.output_min, global_data.output_max);
            }
            break;
        case LCD_PAGE_DUTY_DIFF:
            lcd_printf(0, 0, "Duty:%d", global_data.dyty_cycle_max - global_data.dyty_cycle_min);
            if(global_data.is_voltage) {
                lcd_printf(0, 1, "%s:%.3f", GET_ADC_INFO(0), global_data.output_max - global_data.output_min);
            } else {
                lcd_printf(0, 1, "%s:%.3f", GET_ADC_INFO(1), global_data.output_max - global_data.output_min);
            }
            break;
        default:
            lcd1602_clear();
            break;
        }
        break;
    case LCD_PAGE_PKT:
        lcd_printf(0, 0, "PKT:%d", lost_cnt);
        lcd_printf(0, 1, "CTL:%d", global_data.adc_cnt*2);
        global_data.adc_cnt = 0;
        break;
    case LCD_PAGE_VER:
        lcd_printf(0, 0, "Date:%s", __DATE__);
        lcd_printf(0, 1, "Time:%s", __TIME__);
        show_status = 0;
        break;
    default:
        lcd1602_clear();
        break;
    }
    
    if(show_status) {
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
    
    global_data.output_min = 10000.0;
    global_data.output_max = 0;
    
    APP_DEBUG("status:%s\n", (global_data.is_master)?"M":"S");
   
//    MATLAB DEBUG
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

static void set_pid_value_in_cal_mode(void)
{
    if(global_data.is_master) {
        if(global_data.lcd_cal_ch == IRMS) {
            pid_set_value(&global_data.current_controller, global_data.voltage_cal_set );
            global_data.is_voltage = 0;
        } else if(global_data.lcd_cal_ch == VRMS) {
            pid_set_value(&global_data.voltage_controller, global_data.voltage_cal_set );
            global_data.is_voltage = 1;
        }
    }
}


static void usart_rx_callback(uint8_t *data, uint8_t len)
{
    data[len] = 0;
    lcd1602_write_string(15, 1, (char *)data);
    switch(*data) {
    case 96:
        //ok
        switch(global_data.lcd_page) {
        case LCD_PAGE_SET:
            //page_set next
            global_data.lcd_page_set = (global_data.lcd_page_set + 1) % LCD_PAGE_SET_NUM;
            break;
        case LCD_PAGE_CAL:
            //page cal next
            switch(global_data.lcd_page_cal) {
            case LCD_PAGE_CAL_DEFAULT:
                if(global_data.is_master && global_data.is_voltage && !global_data.is_output) {
                    global_data.lcd_page_cal = LCD_PAGE_CAL_CH;
                    global_data.lcd_cal_ch = 0;
                }
                break;
            case LCD_PAGE_CAL_CH:
                global_data.lcd_page_cal = LCD_PAGE_CAL_INDEX;
                global_data.lcd_cal_index = 0;
                for(int i=0; i<PARA_NUM; i++) {
                    global_data.para_y_div[i] = param_y_config[global_data.lcd_cal_ch][i];
                }
                global_data.voltage_cal_set = param_y_config[global_data.lcd_cal_ch][0];
                set_pid_value_in_cal_mode();
                global_data.is_output = 1;
                set_output_status();
                break;
            case LCD_PAGE_CAL_INDEX:
                global_data.para_x_div[global_data.lcd_cal_index] = global_data.voltage_cal_set;
                global_data.lcd_cal_index++;
                if(global_data.lcd_cal_index >= PARA_NUM) {
                    global_data.lcd_page_cal = LCD_PAGE_CAL_OK;
                    global_data.lcd_cal_index = 0;
                    param_value_reset(global_data.para_x_div, global_data.para_y_div, global_data.lcd_cal_ch, PARA_NUM);
                    global_data.is_output = 0;
                    set_output_status();
                } else {
                    global_data.voltage_cal_set = param_y_config[global_data.lcd_cal_ch][global_data.lcd_cal_index];
                    set_pid_value_in_cal_mode();
                }
                break;
            case LCD_PAGE_CAL_OK:
                param_value_save();
                global_data.lcd_page_cal = LCD_PAGE_CAL_DEFAULT;
                break;
            default:
                lcd1602_clear();
                break;
            }
            break;
        case LCD_PAGE_DUTY:
            global_data.lcd_page_duty = (global_data.lcd_page_duty + 1) % LCD_PAGE_DUTY_NUM;
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
            case LCD_PAGE_SET_DEFAULT:
                //page previous
                if(global_data.lcd_page == 0) {
                    global_data.lcd_page = LCD_PAGE_NUM;
                }
                global_data.lcd_page--;
                break;
            case LCD_PAGE_SET_VOLTAGE:
                //voltage_set sub
                if(global_data.voltage_set > 0) {
                    global_data.voltage_set -= 1;
                }
                break;
            case LCD_PAGE_SET_CURRENT:
                //current_set sub
                if(global_data.current_set > 0) {
                    global_data.current_set -= 0.1;
                }
                break;
            }
            set_pid_value_in_master_mode();
            break;
        case LCD_PAGE_CAL:
            switch(global_data.lcd_page_cal) {
            case LCD_PAGE_CAL_DEFAULT:
                //page previous
                if(global_data.lcd_page == 0) {
                    global_data.lcd_page = LCD_PAGE_NUM;
                }
                global_data.lcd_page--;
                break;
            case LCD_PAGE_CAL_CH:
                if(global_data.lcd_cal_ch == 0) {
                    global_data.lcd_cal_ch = PARA_CHANNEL_NUMBER;
                }
                global_data.lcd_cal_ch--;
                break;
            case LCD_PAGE_CAL_INDEX:
                if(global_data.voltage_cal_set > 0) {
                    global_data.voltage_cal_set -= global_data.lcd_cal_ch == IRMS ? 0.01 : 0.1;
                    set_pid_value_in_cal_mode();
                }
                break;
            case LCD_PAGE_CAL_OK:
                global_data.lcd_page_cal = LCD_PAGE_CAL_DEFAULT;
                break;
            default:
                break;
            }
            break;
        default:
            //page previous
            if(global_data.lcd_page == 0) {
                global_data.lcd_page = LCD_PAGE_NUM;
            }
            global_data.lcd_page--;
            break;
        }
        break;
	case 'b':
        //+
        switch(global_data.lcd_page) {
        case LCD_PAGE_SET:
            switch(global_data.lcd_page_set) {
            case LCD_PAGE_SET_DEFAULT:
                //page next
                global_data.lcd_page = (global_data.lcd_page + 1) % LCD_PAGE_NUM;
                break;
            case LCD_PAGE_SET_VOLTAGE:
                //voltage_set add
                if(global_data.voltage_set < VOLTAGE_MAX) {
                    global_data.voltage_set += 1;
                }
                break;
            case LCD_PAGE_SET_CURRENT:
                //current_set add
                if(global_data.current_set < CURRENT_MAX) {
                    global_data.current_set += 0.1;
                }
                break;
            }
            set_pid_value_in_master_mode();
            break;
        case LCD_PAGE_CAL:
            switch(global_data.lcd_page_cal) {
            case LCD_PAGE_CAL_DEFAULT:
                //page next
                global_data.lcd_page = (global_data.lcd_page + 1) % LCD_PAGE_NUM;
                break;
            case LCD_PAGE_CAL_CH:
                global_data.lcd_cal_ch = (global_data.lcd_cal_ch + 1) % PARA_CHANNEL_NUMBER;
                break;
            case LCD_PAGE_CAL_INDEX:
                if(global_data.voltage_cal_set < VOLTAGE_MAX) {
                    global_data.voltage_cal_set += global_data.lcd_cal_ch == IRMS ? 0.01 : 0.1;
                    set_pid_value_in_cal_mode();
                }
                break;
            case LCD_PAGE_CAL_OK:
                global_data.lcd_page_cal = LCD_PAGE_CAL_DEFAULT;
                break;
            default:
                break;
            }
            break;
        default:
            //page next
            global_data.lcd_page = (global_data.lcd_page + 1) % LCD_PAGE_NUM;
            break;
        }
        break;
    case 'c':
        //exit
        //ok
        switch(global_data.lcd_page) {
        case LCD_PAGE_SET:
            global_data.lcd_page_set = LCD_PAGE_SET_DEFAULT;
            break;
        case LCD_PAGE_CAL:
            switch(global_data.lcd_page_cal) {
            case LCD_PAGE_CAL_DEFAULT:
                break;
            case LCD_PAGE_CAL_CH:
                global_data.lcd_page_cal = LCD_PAGE_CAL_DEFAULT;
                break;
            case LCD_PAGE_CAL_INDEX:
                global_data.lcd_page_cal = LCD_PAGE_CAL_DEFAULT;
                global_data.is_output = 0;
                global_data.is_voltage = 1;
                set_output_status();
                break;
            case LCD_PAGE_CAL_OK:
                global_data.lcd_page_cal = LCD_PAGE_CAL_DEFAULT;
                break;
            }
            break;
        default:
            break;
        }
        break;
    case 'd':
        //output set
        if(global_data.lcd_page == LCD_PAGE_WELCOME || global_data.lcd_page == LCD_PAGE_VER || global_data.lcd_page == LCD_PAGE_CAL)
            break;
        global_data.is_output = !global_data.is_output;
        if(!global_data.is_master && !global_data.is_sync) {
            //Slave must be synchronized
            global_data.is_output = 0;
        }
        //update relay
        set_output_status();
        break;
    case 'e':
        //master & slave set
        if(global_data.lcd_page == LCD_PAGE_WELCOME || global_data.lcd_page == LCD_PAGE_VER || global_data.lcd_page == LCD_PAGE_CAL)
            break;
        global_data.is_master = !global_data.is_master;
        fsk_comm_set_mode(/*global_data.is_master ? TX_FLAG :*/ RX_FLAG);
        break;
    case 'f':
        //vlotage & current set
        if(global_data.lcd_page == LCD_PAGE_WELCOME || global_data.lcd_page == LCD_PAGE_VER || global_data.lcd_page == LCD_PAGE_CAL)
            break;
		global_data.is_voltage = !global_data.is_voltage;
        break;
    default:
        break;
    }
}


static void fsk_data_callback(uint8_t *data, uint16_t len)
{
    global_data.rx_pack_cnt++;
    LED_REV(LED_BASE);
    if(!global_data.is_master) {
        uint16_t tmp = 0;
        float set_value = 0;
        tmp = (data[0] << 8) | data[1];
        set_value = tmp *0.001;
        if(fabs(global_data.current_controller.setval - set_value) < 0.2) {
            set_value = global_data.current_controller.setval * 0.5 + set_value * 0.5;
        }
        pid_set_value(&global_data.current_controller, set_value);
        //printf("Ims:%.3f\r\n", tmp*0.001);
				
        tmp = (data[2] << 8) | data[3];
        set_value = tmp *0.001;
        if(fabs(GET_ADC(VRMS) - set_value) < 0.1) {
            //set_value = global_data.voltage_controller.setval * 0.95 + set_value * 0.05;
        }
        pid_set_value(&global_data.voltage_controller, set_value);
        //printf("Vms:%.3f\r\n", tmp*0.001);
        global_data.sync_keep_time = hal_read_TickCounter();
    }
}


static void adc_rx_callback(int id, void *pbuf, int len)
{
    global_data.adc_cnt++;
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
    
    current_val = ADC_12BIT_VOLTAGE_CALCULATE(current_val);
    voltage_val = ADC_12BIT_VOLTAGE_CALCULATE(voltage_val);
    
    value_adc_physical_set(current_val, IRMS);
    value_adc_physical_set(voltage_val, VRMS);
    
    GET_ADC(IRMS) = get_param_value(GET_ADC(IRMS), IRMS);
    GET_ADC(VRMS) = get_param_value(GET_ADC(VRMS), VRMS);
    
    
    //pid controller
    pid_controller_proc();
//    MATLAB DEBUG
//    cnt++;
//    if(cnt%64 == 0) {
//        for(int i=0;i<ADC1_CONV_NUMBER;i++) {
//            global_data.adc_voltage_buffer[i] = (*adc1pbuf)[i][0];
//            global_data.adc_current_buffer[i] = (*adc1pbuf)[i][1];
//        }
//        global_data.adc_buffer_ready = 1;
//    }
    if(global_data.is_master) {
        static uint8_t data[4] = {0,};
        uint16_t tmp = GET_ADC(IRMS)*1000;
        data[0] = tmp >> 8;
        data[1] = tmp & 0xff;
        tmp = GET_ADC(VRMS)*1000;
        data[2] = tmp >> 8;
        data[3] = tmp & 0xff;
        hal32_usart3_write(data, 4); //Send value to FSK Transmiter by USART3
    }
    TIMER_TASK(timer0, SYNC_TIMEOUT/4, (!global_data.is_master) ) {
        if(SYNC_TIMEOUT < hal_read_TickCounter() - global_data.sync_keep_time) {
            global_data.is_sync = 0;
            global_data.is_output = 0;
            set_output_status();
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
    PRINTF("\r\n\r\n[Sine Wave Inverter] Build , %s %s \r\n", __DATE__, __TIME__);
    
    data_interface_hal_init();
    
    soft_timer_init();
    soft_timer_create(0, 1, 1, lcd_flush_proc, 500);
    
    fsk_comm_set_mode(/*global_data.is_master ? TX_FLAG :*/ RX_FLAG);
    set_pid_value_in_master_mode();  //update pid set value
    set_output_status(); //update relay
    param_default_value_init();
#if FIRST_DOWNLOAD
    param_value_save();
#endif
}


void user_loop(void)
{
    soft_timer_proc();
    usart_rx_proc(usart_rx_callback);
    fsk_data_proc(fsk_data_callback);
    adc_rx_proc(adc_rx_callback);
}


/*****************************END OF FILE***************************/
