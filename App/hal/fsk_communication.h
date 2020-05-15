/* 2020 01 14 */
/* By hxdyxd */
#ifndef _FSK_COMM_H
#define _FSK_COMM_H

#include <stdint.h>

typedef enum {
    RX_FLAG = 0,
    TX_FLAG = 1
}MODE_FLAG;



void fsk_comm_init(void);

typedef void (*FSK_DATA_CALLBACK)(uint8_t *data, uint16_t len);
void fsk_data_proc(FSK_DATA_CALLBACK fsk_func);
void fsk_comm_set_mode(MODE_FLAG mode);

#endif
