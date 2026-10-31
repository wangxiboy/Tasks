/**
  * @file     aht20.c
  * @brief    AHT20 温湿度传感器驱动（I2C 从机）
  *           流程：初始化(0xBE) -> 触发测量(0xAC) -> 等>=80ms -> 读6字节解包
  *           数据：湿度20bit, 温度20bit
  */
#include "aht20.h"
#include "i2c_soft.h"

#define AHT20_ADDR   0x38    /* 7位从机地址 */
#define AHT20_WRITE  0x70    /* 写地址(0x38<<1 | 0) */
#define AHT20_READ   0x71    /* 读地址(0x38<<1 | 1) */

void aht20_init(void)
{
  i2c_start();
  i2c_write_byte(AHT20_WRITE);
  i2c_write_byte(0xBE);   /* 初始化命令 */
  i2c_write_byte(0x08);
  i2c_write_byte(0x00);
  i2c_stop();
}

void aht20_trigger(void)
{
  i2c_start();
  i2c_write_byte(AHT20_WRITE);
  i2c_write_byte(0xAC);   /* 触发测量命令 */
  i2c_write_byte(0x33);
  i2c_write_byte(0x00);
  i2c_stop();
}

uint8_t aht20_read(float *temp, float *hum)
{
  uint8_t  b[6] = {0};
  uint32_t hr = 0, tr = 0;

  i2c_start();
  i2c_write_byte(AHT20_READ);
  for (uint8_t i = 0; i < 5; i++) b[i] = i2c_read_byte(1);   /* 前5字节回ACK */
  b[5] = i2c_read_byte(0);                                    /* 末字节回NACK */
  i2c_stop();

  if ((b[0] & 0x08) == 0) return 1;   /* 状态字节 bit3(校准完成)未置位 => 失败 */

  /* 湿度20bit = b1<<16 | b2<<8 | b3, 再右移4位 */
  hr = ((uint32_t)b[1] << 16 | (uint32_t)b[2] << 8 | b[3]) >> 4;
  /* 温度20bit = (b3低4bit)<<16 | b4<<8 | b5 */
  tr = (((uint32_t)b[3] & 0x0F) << 16 | (uint32_t)b[4] << 8 | b[5]);

  *hum  = (float)hr / 1048576.0f * 100.0f;      /* /2^20 *100% */
  *temp = (float)tr / 1048576.0f * 200.0f - 50.0f; /* /2^20 *200 -50 C */

  return 0;
}
