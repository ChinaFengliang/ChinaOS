/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**--------------File Info---------------------------------------------------------------------------------------------
** File name:           ticker.h
**
** Last version:        V1.00
** Descriptions:        系统滴答定时器驱动头文件.
**--------------------------------------------------------------------------------------------------------------------
** Created by:          Fengliang
** Created date:        2009-10-07
** Version:             V1.00
** Descriptions:        The original version
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************************/
#ifndef __SYSTEM_TICKER__H
#define __SYSTEM_TICKER__H

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                   头文件区
*********************************************************************************************************************/
#include    <include/types.h>

/*********************************************************************************************************************
                                                   宏定义区
*********************************************************************************************************************/


/*********************************************************************************************************************
                                                   类型定义区
*********************************************************************************************************************/
/* 定时器类型 ------------------------------------------------------------------------------------------------------*/
struct __timer
{
    struct __timer        *Prev;                                            /* 上一个定时器                         */
    struct __timer        *Next;                                            /* 下一个定时器                         */
    INT32U                 Time;                                            /* 定时值                               */

#if 0
    /*
     * 以下只是想法, 目前还未应用:
     * 1) 使用bit[0]标志当前定时器功能类型.
     *    ==0: 运行定时器;
     *    ==1: 超时定时器.
     * 2) 其它位保留.
     */
    INT32U                  Flag;                                           /* 标志组                               */
#endif
};
typedef struct __timer                              TIMER;                  /* 定时器类型                           */

/*********************************************************************************************************************
** Function name:           OS_ticker_setup
** Descriptions:            安装系统定时器
** Input parameters:
** Output parameters:
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-26  15:46:29
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
int OS_ticker_setup(void);

/*********************************************************************************************************************
** Function name:           timer_set
** Descriptions:            设置报警定时器
** Input parameters:        pTimer          : 定时器地址
**                          Microsecond     : 报警时刻
** Output parameters:
** Returned value:          ==USB_OK : 操作成功
**                          !=USB_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-26  15:42:17
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
int OS_timer_add(TIMER * const pTimer, INT32U Microsecond);

/*********************************************************************************************************************
** Function name:           OS_timer_del
** Descriptions:            清除报警定时器
** Input parameters:        pTimer: 定时器
** Output parameters:
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-26  15:32:12
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
int OS_timer_del(TIMER * const pTimer);

/*********************************************************************************************************************
** Function name:           timestamp
** Descriptions:            获取当前时间
** Input parameters:        
** Output parameters:       
** Returned value:          当前时刻(单位: 毫秒)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  17:24:8
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INT32U timestamp(void);

#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* __SYSTEM_TICKER__H                   */
/*********************************************************************************************************************
 文件结束
*********************************************************************************************************************/

