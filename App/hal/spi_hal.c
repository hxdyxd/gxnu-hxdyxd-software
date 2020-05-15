/* 
 *
 *2020 01 03 & hxdyxd
 *
 */
#include "data_interface_hal.h"
#include "spi_hal.h"
#include "spi.h"

uint8_t read_single_reg(uint8_t addr)
{
    uint8_t data;
    SX1276Read(addr, &data);
    return data;
}

void SX1276Write( uint8_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

void SX1276Read( uint8_t addr, uint8_t *data )
{
    SX1276ReadBuffer( addr, data, 1 );
}

void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    LED_LOW(LORA_CS);
    addr = addr | 0x80;

    HAL_SPI_Transmit(&hspi1, &addr, 1, 0xff);
    HAL_SPI_Transmit(&hspi1, buffer, size, 0xff);
    LED_HIGH(LORA_CS);
}

void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    LED_LOW(LORA_CS);
    addr = addr & 0x7f;
    
    HAL_SPI_Transmit(&hspi1, &addr, 1, 0xff);
    HAL_SPI_Receive(&hspi1, buffer, size, 0xff);
    LED_HIGH(LORA_CS);
}


