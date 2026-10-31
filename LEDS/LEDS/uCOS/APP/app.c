/*
*********************************************************************************************************
*                                         uC/OS-III APPLICATION TASKS
*   任务一：LED1 每 1s 翻转、LED2 每 3s 翻转、串口每 2s 发一次中文问候
*********************************************************************************************************
*/
#include "app.h"
#include "main.h"
#include "os_cpu.h"   /* for OS_CPU_SysTickInitFreq */      /* LED0_Pin/LED1_Pin 等引脚宏 */
#include <stdio.h>
#include "delay.h"
#include "i2c_soft.h"
#include "aht20.h"

/* --------------------------- 任务优先级（数字越小越优先） --------------------------- */
#define  APP_CFG_TASK_START_PRIO        2u
#define  APP_CFG_TASK_LED1_PRIO         3u
#define  APP_CFG_TASK_LED2_PRIO         4u
#define  APP_CFG_TASK_UART_PRIO         5u

/* --------------------------- 任务栈大小（单位：CPU_STK = 4 字节） ------------------- */
#define  APP_CFG_TASK_START_STK_SIZE    256u
#define  APP_CFG_TASK_LED1_STK_SIZE     128u
#define  APP_CFG_TASK_LED2_STK_SIZE     128u
#define  APP_CFG_TASK_UART_STK_SIZE     256u

static  OS_TCB    AppTaskStartTCB;
static  CPU_STK   AppTaskStartStk[APP_CFG_TASK_START_STK_SIZE];

static  OS_TCB    AppTaskLED1TCB;
static  CPU_STK   AppTaskLED1Stk[APP_CFG_TASK_LED1_STK_SIZE];

static  OS_TCB    AppTaskLED2TCB;
static  CPU_STK   AppTaskLED2Stk[APP_CFG_TASK_LED2_STK_SIZE];

static  OS_TCB    AppTaskUARTTCB;
static  CPU_STK   AppTaskUARTStk[APP_CFG_TASK_UART_STK_SIZE];

/* 任务 1：每 1s 翻转 LED0（PB0） */
static void AppTaskLED1(void *p_arg)
{
    OS_ERR  err;

    (void)p_arg;

    while (1)
    {
        HAL_GPIO_TogglePin(LED0_GPIO_Port, LED0_Pin);   /* 翻转 LED0 */
        OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);   /* 延时 1s */
    }
}

/* 任务 2：每 3s 翻转 LED1（PB1） */
static void AppTaskLED2(void *p_arg)
{
    OS_ERR  err;

    (void)p_arg;

    while (1)
    {
        HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);   /* 翻转 LED1 */
        OSTimeDlyHMSM(0u, 0u, 3u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);   /* 延时 3s */
    }
}

/* 任务 3：每 2s 采集一次 AHT20 温湿度并通过串口发送（printf 已重定向到 USART1） */
static void AppTaskUART(void *p_arg)
{
    OS_ERR  err;
    float   temp = 0.0f, hum = 0.0f;

    (void)p_arg;

    while (1)
    {
        aht20_trigger();                                               /* 触发测量 */
        OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_STRICT, &err); /* 等转换，让出CPU */
        if (aht20_read(&temp, &hum) == 0)                              /* 成功返回0 */
        {
            printf("温度:%.2f℃  湿度:%.2f%%\r\n", temp, hum);
        }
        else
        {
            printf("AHT20 读取失败\r\n");
        }
        OSTimeDlyHMSM(0u, 0u, 2u, 0u, OS_OPT_TIME_HMSM_STRICT, &err);  /* 凑满2s周期 */
    }
}

/* 启动任务：创建另外 3 个任务后删除自己 */
void AppTaskStart(void *p_arg)
{
    OS_ERR  err;

    (void)p_arg;

    delay_init();        /* 使能 DWT 周期计数器(软件I2C位时序用) */
    i2c_soft_init();     /* PB8=SCL, PB9=SDA, 开漏+上拉 */
    HAL_Delay(100);      /* AHT20 上电稳定 */
    aht20_init();        /* AHT20 初始化 */

    OSTaskCreate(&AppTaskLED1TCB, "LED1", AppTaskLED1, (void *)0,
                 APP_CFG_TASK_LED1_PRIO, AppTaskLED1Stk,
                 0u, APP_CFG_TASK_LED1_STK_SIZE, 0u, 0u,
                 (void *)0,
                 OS_OPT_TASK_STK_CHK, &err);

    OSTaskCreate(&AppTaskLED2TCB, "LED2", AppTaskLED2, (void *)0,
                 APP_CFG_TASK_LED2_PRIO, AppTaskLED2Stk,
                 0u, APP_CFG_TASK_LED2_STK_SIZE, 0u, 0u,
                 (void *)0,
                 OS_OPT_TASK_STK_CHK, &err);

    OSTaskCreate(&AppTaskUARTTCB, "UART", AppTaskUART, (void *)0,
                 APP_CFG_TASK_UART_PRIO, AppTaskUARTStk,
                 0u, APP_CFG_TASK_UART_STK_SIZE, 0u, 0u,
                 (void *)0,
                 OS_OPT_TASK_STK_CHK, &err);

    OSTaskDel(NULL, &err);   /* 删除启动任务自己，回收资源 */
}

/* 应用初始化：OSInit -> 创建启动任务 -> OSStart（由 main 调用） */
void AppInit(void)
{
    OS_ERR  err;

    OSInit(&err);

    OSTaskCreate(&AppTaskStartTCB, "AppTaskStart", AppTaskStart, (void *)0,
                 APP_CFG_TASK_START_PRIO, AppTaskStartStk,
                 0u, APP_CFG_TASK_START_STK_SIZE, 0u, 0u,
                 (void *)0,
                 OS_OPT_TASK_STK_CHK, &err);

    OS_CPU_SysTickInitFreq(SystemCoreClock);   /* init uC/OS system tick (1ms) */
    OSStart(&err);
}
