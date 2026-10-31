#ifndef __I2C_SOFT_H
#define __I2C_SOFT_H

#include "stm32f1xx_hal.h"

void    i2c_soft_init(void);
void    i2c_start(void);
void    i2c_stop(void);
void    i2c_write_byte(uint8_t dat);
uint8_t i2c_read_byte(uint8_t ack);   /* ack=1 回ACK, ack=0 回NACK */

#endif
