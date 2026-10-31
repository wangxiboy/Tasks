/**
  * @file     delay.c
  * @brief    微秒级延时（基于 DWT 计数器，不干扰 HAL 的 SysTick）
  *           用于软件 I2C 的位时序控制
  */
#include "delay.h"

void delay_init(void)
{
  /* 使能 DWT 的 CYCCNT 周期计数器（Cortex-M3 支持） */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL        |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us)
{
  /* 72MHz 主频下，CYCCNT 每 +1 即为 1/72us */
  DWT->CYCCNT = 0;
  uint32_t ticks = us * (SystemCoreClock / 1000000UL);
  while (DWT->CYCCNT < ticks);
}
