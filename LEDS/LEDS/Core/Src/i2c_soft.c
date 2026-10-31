/**
  * @file     i2c_soft.c
  * @brief    软件 I2C（GPIO 模拟，SCL=PB8, SDA=PB9）
  *           原理：按 I2C 协议用 GPIO 拉高拉低产生时序。
  *           要点：SDA 配成开漏，靠模块上拉电阻读总线电平。
  */
#include "i2c_soft.h"
#include "delay.h"

#define SCL_GPIO   GPIOB
#define SCL_PIN    GPIO_PIN_8
#define SDA_GPIO   GPIOB
#define SDA_PIN    GPIO_PIN_9

#define SCL_H()   HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_SET)
#define SCL_L()   HAL_GPIO_WritePin(SCL_GPIO, SCL_PIN, GPIO_PIN_RESET)
#define SDA_H()   HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_SET)
#define SDA_L()   HAL_GPIO_WritePin(SDA_GPIO, SDA_PIN, GPIO_PIN_RESET)
#define SDA_READ() HAL_GPIO_ReadPin(SDA_GPIO, SDA_PIN)

void i2c_soft_init(void)
{
  GPIO_InitTypeDef g = {0};
  __HAL_RCC_GPIOB_CLK_ENABLE();
  g.Pin   = SCL_PIN;
  g.Mode  = GPIO_MODE_OUTPUT_OD;   /* 开漏：写1=释放，靠上拉拉高 */
  g.Pull  = GPIO_PULLUP;
  g.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SCL_GPIO, &g);
  g.Pin   = SDA_PIN;
  HAL_GPIO_Init(SDA_GPIO, &g);
  SCL_H();
  SDA_H();
}

/* 起始：SCL 高电平时 SDA 由高->低 */
void i2c_start(void)
{
  SDA_H(); SCL_H(); delay_us(5);
  SDA_L();            delay_us(5);
  SCL_L();
}

/* 停止：SCL 高电平时 SDA 由低->高 */
void i2c_stop(void)
{
  SDA_L(); SCL_H(); delay_us(5);
  SDA_H();            delay_us(5);
}

void i2c_write_byte(uint8_t dat)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (dat & 0x80) SDA_H(); else SDA_L();
    delay_us(2);
    SCL_H(); delay_us(2); SCL_L();
    dat <<= 1;
  }
  SDA_H();                /* 释放 SDA，等从机应答 */
  delay_us(2); SCL_H(); delay_us(2); SCL_L();
}

uint8_t i2c_read_byte(uint8_t ack)
{
  uint8_t dat = 0;
  SDA_H();                /* 释放 SDA，让从机驱动 */
  for (uint8_t i = 0; i < 8; i++)
  {
    SCL_H(); delay_us(2);
    dat = (dat << 1) | SDA_READ();
    SCL_L(); delay_us(2);
  }
  if (ack) SDA_L(); else SDA_H();   /* ACK=低, NACK=高 */
  SCL_H(); delay_us(2); SCL_L();
  return dat;
}
