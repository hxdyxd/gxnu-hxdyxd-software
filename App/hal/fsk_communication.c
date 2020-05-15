/* 2020 01 14 */
/* By hxdyxd */
#include <string.h>
#include "fsk_communication.h"
#include "data_interface_hal.h"
#include "sx1276-Fsk.h"
#include "sx1276-FskMisc.h"


#define FSK_BITRATE       (9600)
#define FSK_PACK_FREQ     (50)

#define SYNCWORD_BYTE   (4)
#define PREAMBLE_BYTE   (4)
//preamble

#define FSK_BUFFER_BIT   ((FSK_BITRATE)/(FSK_PACK_FREQ))
#define FSK_BUFFER_BYTE   ((FSK_BUFFER_BIT)/8)


#define FSK_PACKET_BIT   (FSK_BUFFER_BIT-SYNCWORD_BYTE*8-PREAMBLE_BYTE*8)
#define FSK_PACKET_BYTE   (FSK_BUFFER_BYTE-SYNCWORD_BYTE-PREAMBLE_BYTE)



static struct {
    uint16_t index;
    uint8_t buffer[FSK_BUFFER_BYTE];
    uint8_t tx_flag;
} fsk_data;



uint8_t fsk_user_buffer[FSK_PACKET_BYTE];
__IO uint8_t fsk_user_rxe = 0;
__IO uint8_t fsk_user_txr = 0;

void fsk_data_proc(FSK_DATA_CALLBACK fsk_func)
{
    if(fsk_data.tx_flag == TX_FLAG) {
        if(fsk_user_txr)
            return;
        fsk_func(fsk_user_buffer, FSK_PACKET_BYTE);
        fsk_user_txr = 1;
    } else if(fsk_user_rxe) {
        fsk_func(fsk_user_buffer, FSK_PACKET_BYTE);
        fsk_user_rxe = 0;
    }
}



enum {
    WAIT_SYNC = 0,
    SYNCING = 1
} rx_status = 0;


void fsk_comm_init(void)
{
    LED_HIGH(LORA_NRST);
    HAL_Delay(10);
    LED_LOW(LORA_NRST);
    HAL_Delay(60);
    LED_HIGH(LORA_NRST);
    HAL_Delay(60);
    //FSK Continuous mode
    SX1276FskInit_Debug();
    fsk_comm_set_mode(RX_FLAG);
}

/*
 * mode = 1: OUTPUT
 * mode = 0: INPUT
 */
static void stm32_fsk_data_gpio_config(uint8_t mode)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /*Configure GPIO pin : PtPin */
    GPIO_InitStruct.Pin = FSK_DATA_Pin;
    if(mode) {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    } else {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    }
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FSK_DATA_GPIO_Port, &GPIO_InitStruct);
}


const uint32_t fsk_sync_word = 0x967E8169;
void fsk_comm_set_mode(MODE_FLAG mode)
{
    if(mode == TX_FLAG) {
        stm32_fsk_data_gpio_config(1);
        fsk_data.tx_flag = TX_FLAG;
        SX1276FskSetOpMode( RF_OPMODE_TRANSMITTER );
        
        memset(fsk_data.buffer, 0, FSK_BUFFER_BYTE);
        memcpy(fsk_data.buffer, "\xaa\xaa\xaa\xaa", 4);
        memcpy(&fsk_data.buffer[4], &fsk_sync_word, 4);
    } else {
        stm32_fsk_data_gpio_config(0);
        fsk_data.tx_flag = RX_FLAG;
        SX1276FskSetOpMode( RF_OPMODE_RECEIVER );
    }
    fsk_data.index = 0;
}



/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(fsk_data.tx_flag == TX_FLAG) {
        //Transmiter
        uint16_t xbit = fsk_data.index%8;
        uint16_t xindex = fsk_data.index/8;
        if(fsk_data.buffer[xindex] & (0x80>>xbit))
            LED_HIGH(FSK_DATA);
        else
            LED_LOW(FSK_DATA);
        fsk_data.index = (fsk_data.index+1) % FSK_BUFFER_BIT;
        if(fsk_data.index == 0) {
            if(fsk_user_txr) {
                memcpy(&fsk_data.buffer[8], fsk_user_buffer, FSK_PACKET_BYTE);
                fsk_user_txr = 0;
            }
            static uint8_t cnt = 0;
            fsk_data.buffer[FSK_BUFFER_BYTE-1] = cnt + 'A';
            cnt = (cnt+1) % 26;
            if(fsk_data.buffer[FSK_BUFFER_BYTE-1] == 'A') {
                sine_wave_set_status(0);
                sine_wave_set_status(1);
            }
        }
    } else {
        static uint32_t sync_word = 0;
        switch(rx_status) {
        case WAIT_SYNC:
        {
            sync_word <<= 1;
            if(GPIO_READ(FSK_DATA))
                sync_word |= 1;
            if(sync_word == 0x69817e96) {
                fsk_data.index = 0;
                rx_status = SYNCING;
            }
            break;
        }
        case SYNCING:
        {
            uint16_t xindex = fsk_data.index/8;
            fsk_data.buffer[xindex] <<= 1;
            if(GPIO_READ(FSK_DATA))
                fsk_data.buffer[xindex] |= 1;
            fsk_data.index++;
            if(fsk_data.index >= FSK_PACKET_BIT) {
                if(fsk_data.buffer[FSK_PACKET_BYTE-1] == 'Z') {
                    sine_wave_set_status(0);
                    sine_wave_set_status(1);
                }
                sync_word = 0;
                rx_status = WAIT_SYNC;
                memcpy(fsk_user_buffer, fsk_data.buffer, FSK_PACKET_BYTE);
                fsk_user_rxe = 1;
                {
                    extern uint8_t lost_cnt;
                    static uint8_t previoys_val = 0;
                    if( fsk_user_buffer[FSK_PACKET_BYTE-1] - previoys_val != 1 && previoys_val != 'Z') {
                        lost_cnt ++;
                    }
                    previoys_val = fsk_user_buffer[FSK_PACKET_BYTE-1];
                }
            }
            break;
        }
        default:
            break;
        }
    }
    /* end if */
}



/******************* (C) COPYRIGHT 2018 hxdyxd *****END OF FILE****/
