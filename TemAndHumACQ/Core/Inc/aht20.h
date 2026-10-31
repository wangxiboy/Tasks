#ifndef __AHT20_H
#define __AHT20_H

#include "stm32f1xx_hal.h"

void    aht20_init(void);                       /* 上电初始化 */
void    aht20_trigger(void);                    /* 触发测量 */
uint8_t aht20_read(float *temp, float *hum);    /* 读并解包, 成功返回0 */

#endif
