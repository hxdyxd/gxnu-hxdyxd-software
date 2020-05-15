/* 
 *
 *2020 01 01 & hxdyxd
 *
 */
#ifndef _SINE_PWM_CONTROL_H
#define _SINE_PWM_CONTROL_H

#include <stdint.h>

#define SAMP_PER_PERIOD    (256)

/* constant */
#define PIx2               (2*3.14)
#define TABLE_SIZE         ((SAMP_PER_PERIOD)*2)
#define SAMP_RATE          (1.0 / (SAMP_PER_PERIOD))

#define PWM_MAX_V          (1124)


uint16_t sine_wave_get_dutycycle_maxval(void);
void sine_wave_set_table(uint16_t max);
void sine_wave_set_status(uint8_t enable);


#endif
/******************* (C) COPYRIGHT 2020 hxdyxd *****END OF FILE****/
