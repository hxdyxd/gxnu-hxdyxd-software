#ifndef _SPI_HAL_H
#define _SPI_HAL_H


#include <stdint.h>

uint8_t read_single_reg(uint8_t addr);
void SX1276Write( uint8_t addr, uint8_t data );
void SX1276Read( uint8_t addr, uint8_t *data );
void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size );
void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size );


#endif
