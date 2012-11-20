/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            millisecond.c
** Last version:         V1.00
** Descriptions:         毫秒定时器文件.
** Hardware platform:    lpc1768
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2012年2月9日  23:35:11
** Version:              V1.00
** Descriptions:         The original version
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
#include    <include/macro.h>
#include    <include/types.h>
#include    <library/ioport.h>
#include    "../../lpc17xx.h"
#include    "./timer.h"


/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
static const TIMER_FEATURE mtimer_features =
{
    0x40008000,                                                             /* 寄存器基址                           */
    12499,                                                                  /* 计时级别: 毫秒级                     */
};


/*********************************************************************************************************************
** Function name:           mtimer_setup
** Descriptions:            安装毫秒定时器
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-9  23:46:48
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int mtimer_setup(void)
{
    /* 要求:
     * 1、使定时计数器对周期1毫秒的时钟信号进行计数;
     */

    /* 1)
     * 私有构建过程
     */
    dword_set_bits(PCONP, 1ul<<2);                                          /* 使能定时器1                          */

    /* 2)
     * 类构建过程
     */
    timer_setup(&mtimer_features);
    
    return OK;
}

/*********************************************************************************************************************
** Function name:           mtimer_cleanup
** Descriptions:            卸载毫秒定时器
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-10  0:18:47
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int mtimer_cleanup(void)
{
    timer_cleanup(&mtimer_features);
    dword_clear_bits(PCONP, 1ul<<2);                                          /* 禁能定时器1                          */

    return OK;
}

/*********************************************************************************************************************
** Function name:           mtimer_gettick
** Descriptions:            读取毫秒定时器的当前嘀嗒数
** Input parameters:        
** Output parameters:       
** Returned value:          当前嘀嗒数
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-10  0:18:50
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INT32U mtimer_gettick(void)
{
    return timer_count(&mtimer_features);
}

/*********************************************************************************************************************
                                                    设备定义区
*********************************************************************************************************************/
struct timer_t mtimer = 
{
    mtimer_setup,
    mtimer_cleanup,
    mtimer_gettick,
};
EXPORT_TO_DEVICE(mtimer, "毫秒定时器");

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

