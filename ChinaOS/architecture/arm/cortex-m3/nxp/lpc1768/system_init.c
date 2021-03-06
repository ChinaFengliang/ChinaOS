/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            init.c
** Last version:         V1.00
** Descriptions:         系统初始化文件.
** Hardware platform:    lpc17xx
** SoftWare platform:   
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Fengliang
** Created date:         2010年9月3日  14:45:10
** Version:              V1.00
** Descriptions:         ARM7TDMI-S处理器，最高运行频率为72MHz
**
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************************/

/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <string.h>
#include    <include/macro.h>
#include    <library/ioport.h>
#include    "../lpc17xx.h"

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 时钟源 ----------------------------------------------------------------------------------------------------------*/
#define Fosc                12000000                                        /* 系统主振荡器频率: 12MHz              */
#define Firc                 4000000                                        /* 内部RC振荡器频率: 4MHz               */
#define Frtc                       0                                        /* 实时钟振荡器频率: 0MHz               */
/* 模块频率 --------------------------------------------------------------------------------------------------------*/
#define Fcpu               100000000                                        /* CPU工作频率: 100MHz                  */
#define Fusb                48000000                                        /* USB工作频率: 48MHz                   */
/* 倍频频率 --------------------------------------------------------------------------------------------------------*/
#define Fcco               300000000                                        /* PLL倍频器输出频率: 300MHz (越小越好) */
/* 设置参数 --------------------------------------------------------------------------------------------------------*/
#define M                         25                                        /* 倍频器(PLL)参数M(越小越好)           */
#define N                          2                                        /* 倍频器(PLL)参数N(越小越好)           */

/* 提示信息 --------------------------------------------------------------------------------------------------------*/
#if (100000000 < Fcpu)
#error "Fcpu must less than 72000000!"
#endif

#if (48000000 != Fusb)
#error "Fusb must be 48000000!"
#endif

#if (Fcco < 275000000)
#error "Fcco must be [275000000, 550000000]!"
#endif

#if (550000000 < Fcco)
#error "Fcco must be [275000000, 550000000]!"
#endif



/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/



/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/



/*********************************************************************************************************************
** Function name:           system_clock_init
** Descriptions:            系统时钟信号初始化
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              
** Created Date:            2011-8-24  10:30:15
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void system_clock_init(void)
{
    /* 1.1) 确定系统主振荡器 
     * 读取主振荡器状态,确保主振荡器处于稳定工作状态。
     */
    dword_modify(SCS, (1ul<<4),                                             /* 选择主振荡器频率范围: 1MHz~20MHz     */
                      (1ul<<5));                                            /* 使能主振荡器                         */
    while (0 == ((1ul<<6) & read_dword(SCS)));                              /* 等待主振荡器信号稳定                 */

    /* 
     * 1.2) 设置PLL倍频器参数
     */
    /* 1.2.1) 断开PLL倍频器 */     
    write_dword(PLL0CON, (0ul<<1) |                                         /* 旁路PLL倍频器                        */
                         (0ul<<0));                                         /* 禁能PLL倍频器                        */
    write_dword(PLL0FEED, 0xAA);                                            /* 执行验证序列A                        */
    write_dword(PLL0FEED, 0x55);                                            /* 执行验证序列B                        */

    /* 1.2.2) 选择PLL时钟源 */ 
    write_dword(CLKSRCSEL, 0x01);                                           /* 选择主振荡器作为PLL输入时钟源        */
    
    /* 1.2.3) 设置倍频参数 */
    write_dword(PLL0CFG, ((N-1) << 16)|                                     /* 设置PLL倍频参数                      */
                         ((M-1) << 0));
    write_dword(PLL0FEED, 0xAA);                                            /* 执行验证序列A                        */
    write_dword(PLL0FEED, 0x55);                                            /* 执行验证序列B                        */
        
    /* 1.2.4 激活倍频器频率锁定行为 */
    dword_set_bits(PLL0CON, 1ul<<0);
    write_dword(PLL0FEED, 0xAA);                                            /* 执行验证序列A                        */
    write_dword(PLL0FEED, 0x55);                                            /* 执行验证序列B                        */

    /* 1.2.5) 配置CPU时钟频率 */
    write_dword(CCLKCFG, Fcco/Fcpu - 1);                                    /* 设置CPU时钟分频系数                  */    
    
    /* 1.2.6) 切换至倍频信号 */
    while (0 == ((1ul<<26) & read_dword(PLL0STAT)));                        /* 等待PLL倍频器锁定目标信号频率        */
    dword_set_bits(PLL0CON, 1ul<<1);                                       /* 连接PLL倍频器                        */
    write_dword(PLL0FEED, 0xAA);                                            /* 执行验证序列A                        */
    write_dword(PLL0FEED, 0x55);                                            /* 执行验证序列B                        */

    /*
     * 3) 配置外设时钟预分频系数
     *    00 : PCLK_peripheral = Fcpu/4;
     *    01 : PCLK_peripheral = Fcpu;
     *    10 : PCLK_peripheral = Fcpu/2;
     *    11 : PCLK_peripheral = Fcpu/8,
     *    except for CAN1, CAN2, and CAN filtering when “11” selects = Fcpu/6
     */
    write_dword(PCLKSEL0, (0ul << 0)  |                                     /* WDT                                  */
                          (3ul << 2)  |                                     /* TIMER0                               */
                          (3ul << 4)  |                                     /* TIMER1                               */
                          (3ul << 6)  |                                     /* UART0                                */
                          (0ul << 8)  |                                     /* UART1                                */
                          (0ul << 10) |                                     /* (保留)                               */
                          (0ul << 12) |                                     /* PWM1                                 */
                          (0ul << 14) |                                     /* I2C0                                 */
                          (0ul << 16) |                                     /* SPI                                  */
                          (0ul << 18) |                                     /* (保留)                               */
                          (0ul << 20) |                                     /* SSP1                                 */
                          (0ul << 22) |                                     /* DAC                                  */
                          (0ul << 24) |                                     /* ADC                                  */
                          (0ul << 26) |                                     /* CAN1                                 */
                          (0ul << 28) |                                     /* CAN2                                 */
                          (0ul << 30));                                     /* ACF                                  */
    write_dword(PCLKSEL1, (0ul << 0)  |                                     /* Quadrature Encoder Interface         */   
                          (0ul << 2)  |                                     /* GPIO interrupts                      */
                          (0ul << 4)  |                                     /* Pin Connect block.                   */
                          (0ul << 6)  |                                     /* I2C1                                 */
                          (0ul << 8)  |                                     /* (保留)                               */
                          (0ul << 10) |                                     /* SSP0                                 */  
                          (0ul << 12) |                                     /* TIMER2                               */
                          (0ul << 14) |                                     /* TIMER3                               */
                          (0ul << 16) |                                     /* UART2                                */
                          (0ul << 18) |                                     /* UART3                                */
                          (0ul << 20) |                                     /* I2C2                                 */
                          (0ul << 22) |                                     /* I2S                                  */
                          (0ul << 24) |                                     /* (保留)                               */
                          (0ul << 26) |                                     /* Repetitive Interrupt Timer           */
                          (0ul << 28) |                                     /* System Control block                 */
                          (0ul << 30));                                     /* Motor Control PWM                    */
    write_dword(USBCLKCFG, Fcco/Fusb - 1);                                  /* 设置USB时钟分频系数                  */

	//write_dword(PCONP, 0xFFFFFFFF);                                         /* 使能所有外设供电                     */
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

