/* 2019 06 25 */
/* By hxdyxd */
#ifndef __ADC_ALGORITHM_H
#define __ADC_ALGORITHM_H

#include <stdint.h>
#include <stdio.h>
#include "data_interface_hal.h"

#define FIRST_DOWNLOAD    (0)



/* ADC CHANNEL */
#define VRMS           (0)
#define IRMS           (1)

#define GET_ADC(ch)        (value_adc_adjustment_key[ch].physical_val)
#define GET_ADC_INFO(ch)   (value_adc_adjustment_key[ch].info)

struct adc_adjustment_t
{
    //parametra
    float key; //hardware gain
    char *info;
    
    //var
    float physical_val;
};

extern struct adc_adjustment_t value_adc_adjustment_key[ADC1_CHANNEL_NUMBER];

//CAL
struct param_t
{
    double k;
    double b;
};

#define PARA_NUM   (3)
#define PARA_CHANNEL_NUMBER  (ADC1_CHANNEL_NUMBER)
extern struct param_t gs_para[PARA_CHANNEL_NUMBER];


//PID
typedef struct {
    float kp;
    float ki;
    float kd;
    float le;
    float se;
    float setval;
    float output;
    uint16_t i_max;
    int16_t max_output;
    int16_t min_output;
}pidc_t;


//Linear regression
#define  EASY_LR(x,x1,y1,x2,y2)                (((y2) - (y1))/((x2) - (x1))*((x) - (x1)) + (y1))


/*******************************************************************************
* Function Name  : value_adc_adjustment.
* Description    : set the original voltage or current values
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
float value_adc_physical_set(float adc_voltage, int id);


/*******************************************************************************
* Function Name  : param_default_value_init.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void param_default_value_init(void);


/*******************************************************************************
* Function Name  : param_value_reset.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void param_value_reset(float *x, float *y, int channel, uint8_t count);


/*******************************************************************************
* Function Name  : param_value_save.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void param_value_save(void);


/*******************************************************************************
* Function Name  : get_param_value.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
float get_param_value(float input, int i);


/*******************************************************************************
* Function Name  : pid_init.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void pid_init(pidc_t *pid, float kp, float ki, float kd);


/*******************************************************************************
* Function Name  : pid_set_output_limit.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void pid_set_output_limit(pidc_t *pid, float max_output, float min_output);


/*******************************************************************************
* Function Name  : pid_set_value.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void pid_set_value(pidc_t *pid, float setval);


/*******************************************************************************
* Function Name  : pid_ctrl.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
float pid_ctrl(pidc_t *pid, float curval);


/*******************************************************************************
* Function Name  : No_Max_Min_Filter.
* Description    : 
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
uint16_t No_Max_Min_Filter(uint16_t *in_dat, uint16_t num, uint8_t channel_num, uint8_t type);


#endif
/*****************************END OF FILE***************************/
