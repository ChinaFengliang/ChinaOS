/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            ticker.c
** Last version:         V1.00
** Descriptions:         系统之数据类型头文件.
** Hardware platform:    lpc1768
** SoftWare platform:    ChinaOS操作系统
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011年7月19日  22:57:30
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
#include    <string.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <nvic/interrupt.h>
#include    "./ticker.h"
#include    <include/debug.h>
#include    <nxp/lpc1768/lpc17xx.h>
#include    <syscall.h>
#include    <kernel/kernel.h>
#include    <library/bit/bit.h>
#include    <library/ioport.h>

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/
#define     SYSTICK_CONTROL                         0xE000E010              /* Systick控制及状态寄存器              */
#define     SYSTICK_RELOAD                          0xE000E014              /* Systick重装载数值寄存器              */
#define     SYSTICK_CURRENT                         0xE000E018              /* Systick计数数值寄存器                */
#define     SYSTICK_CALIBRATE                       0xE000E01C              /* Systick校准数值寄存器                */

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
static TIMER        *OS_pTimer;                                             /* 当前定时器                           */
extern INT32U        OS_ActivePrio;                                         /* 激活的优先级标志                     */
extern THREAD       *OS_ActiveProc[33];                                     /* 激活的线程队列                       */
extern THREAD       *OS_This;                                               /* 当前线程                             */

/*********************************************************************************************************************
** Function name:           get_left_time
** Descriptions:            获得当前报警剩余时间
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-4-24  13:48:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE INT32U get_left_time(void)
{
    return read_dword(SYSTICK_CURRENT);
}

/*********************************************************************************************************************
** Function name:           timer_start
** Descriptions:            定时器开始计时
** Input parameters:        Time: 定时时间(单位: 毫秒)
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-4-24  13:35:46
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void timer_start(INT32U Time)
{
    write_dword(SYSTICK_RELOAD, Time);                                      /* 设置下一次报警值                     */ 
    write_dword(SYSTICK_CURRENT,0);                                         /* 复位定时器                           */    
    dword_set_bits(SYSTICK_CONTROL, (1ul << 1) |                            /* 计数器中断使/禁能(0:禁能, 1:使能)    */
                                    (1ul << 0));                            /* 计数器使/禁能(0:禁能, 1:使能)        */
}

/*********************************************************************************************************************
** Function name:           timer_start
** Descriptions:            定时器开始计时
** Input parameters:        Time: 定时时间(单位: 毫秒)
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-4-24  13:35:46
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void timer_stop(void)
{
    dword_clr_bits(SYSTICK_CONTROL, 1ul << 1);                              /* 关定时器中断                         */
    write_dword(0xE000ED04, 1ul << 25);                                     /* 清除SysTick悬起状态                  */
}

/*********************************************************************************************************************
** Function name:           OS_timer_exception
** Descriptions:            提供定时服务,将已到期的线程唤醒.
** input parameters:        none
** output parameters:       none
** Returned value:          none
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-01-06
** Test record:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test record:
*********************************************************************************************************************/
void systimer_exception(void)
{
    INT32U          Time;                                                   /* 当前定时值                           */
    INT32U          Priority;                                               /* 线程优先级                           */
    TIMER          *pTimer;                                                 /* 临时定时器                           */
    

    /*
     * 1) 清除中断标志.
     */
   OS_thread_switch();

    /*
     * 2) 容错处理.
     * 防止在删除定时器的过程中(临界代码), 定时器已产生中断.
     */
    pTimer = OS_pTimer;
    if (NULL == pTimer)
    {
        goto exit;
    }
    Time = pTimer->Time;
    
    do
    {
        THREAD         *pThread;                                            /* 定时器所属线程                       */
        
        /*
         * 3) 将定时器所属线程就绪.
         */
        pThread = (THREAD *)(((INT32U)pTimer) - (INT32U)&((THREAD*)0)->Timer);
        Priority = pThread->Priority;

        if (NULL == OS_ActiveProc[Priority])
        {   /* 由0个节点增加到1个节点 */
            pThread->pNext = pThread;
            pThread->pPrev = pThread;
            OS_ActiveProc[Priority] = pThread;
            OS_ActivePrio |= 1ul << Priority;                               /* 设置优先级就绪标志                   */
        }
        else
        {   /* 加在队列的后面 */
            pThread->pPrev = OS_ActiveProc[Priority]->pPrev;
            pThread->pNext = OS_ActiveProc[Priority];
            OS_ActiveProc[Priority]->pPrev->pNext = pThread;
            OS_ActiveProc[Priority]->pPrev = pThread;
        }

        /*
         * 4) 切换到下一个定时器.
         */
        pTimer = pTimer->Next;                                              /* 切换到下一个定时器                   */
        
        if (Time != pTimer->Time)
        {
            timer_start(pTimer->Time - pTimer->Prev->Time);
            OS_pTimer = pTimer;
            return;
        }

    } while (pTimer != OS_pTimer);                                          /* 遍历环形定时器链表                   */

    OS_pTimer = NULL;                                                       /* 指向空链环                           */
    
exit:    
    timer_stop();
}

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
int OS_ticker_setup(void)
{    
    /* 要求:
     * 1、使定时计数器对周期1毫秒的时钟信号进行计数;
     */

    OS_pTimer = NULL;
    write_dword(SYSTICK_CONTROL, (1ul<<2) |                             /* 计数器时钟源: 内核时钟(FCLK)             */
                                 (1ul<<1) |                             /* 计数器为0时产生systick中断请求           */     
                                 (0ul<<0));                             /* 计数器禁能                               */
    return OS_OK;
}

/*********************************************************************************************************************
** Function name:           OS_timer_add
** Descriptions:            设置报警定时器
** Input parameters:        pTimer   : 定时器
**                          Millisecond : 延时时间(单位: 毫秒)
** Output parameters:
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-26  15:42:17
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
int OS_timer_add(TIMER * const pTimer, INT32U Millisecond)
{
    /* 本函数所考虑的情况如下:
     * 对定时器管理档案进行修改时，需要使用关中断的方式严格控制以保持独立操作特性.
     * 对定时器计数器是一直向前自加，所以采取容错的方式进行保障.
     * 情况一: 添加节点信息, 不更新硬件设置.
     * 情况二: 添加节点信息, 并更新硬件设置.
     * 实现机制:
     * 1) 以环形双向链表结构管理;
     * 2) 以前置方式将节点信息插入到链表;
     */
    TIMER       *pTmp = OS_pTimer;
    INT32U       LeftTime;                                                  /* 剩余时间                             */
    INT32U       Time;                                                      /* 定时时刻                             */


   /*
    * 将目标定时器链入管理链表环 
    */
    if (NULL == pTmp)                                                       /* 添加唯一的定时器                     */
    {
        pTimer->Time = Millisecond;
        pTimer->Next = pTimer;                                              /* 指向自身                             */
        pTimer->Prev = pTimer;
        goto replace;
    }

    LeftTime = get_left_time();
    Time = pTmp->Time + Millisecond - LeftTime;
    pTimer->Time = Time;
    
    while (OS_pTimer != pTmp->Next)                                         /* 查找定位                             */
    {
        INT32U       TimeFormer;
        INT32U       TimeLatter;
        
        TimeFormer = pTmp->Time;
        TimeLatter = pTmp->Next->Time;

        if ((Time - TimeFormer) <= (TimeLatter - TimeFormer))               /* 判断指定定时器是否在两者之间         */
        {
            break;
        }

        pTmp = pTmp->Next;
    }

    /* 
     * 插入至pTmp节点后
     */
    pTimer->Next     = pTmp->Next;                                          
    pTimer->Prev     = pTmp;
    pTmp->Next->Prev = pTimer;
    pTmp->Next       = pTimer;

    
    if (LeftTime <= Millisecond)                                            /* 若目标定时器的定时值非最近时限       */
    {
        return OS_OK;
    }

   /*
    * 切换当前定时器 
    */    
replace:
    OS_pTimer = pTimer;                                                     /* 设为当前计数器                       */
    timer_start(Millisecond);

    return OS_OK;
}

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
int OS_timer_del(TIMER * const pTimer)
{
    if ((NULL==pTimer->Prev) || (NULL==pTimer->Next))
    {
        return OS_ERR;
    }

    if (pTimer == OS_pTimer)
    {
        /* 指定定时器 == 当前定时器时 */
        if (pTimer == pTimer->Next)
        {   /* 当前定时器为唯一定时器时 */
            timer_stop();
            OS_pTimer = NULL;                                               /* 删除唯一的定时器                     */
        }
        else
        {   /* 当前定时器不为唯一定时器时 */
            INT32U       LeftTime;                                          /* 剩余时间                             */

            LeftTime  = get_left_time();
            OS_pTimer = pTimer->Next;
            timer_start(OS_pTimer->Time - pTimer->Time + LeftTime);
        }
    }

    /* 指定定时器 != 当前定时器时 */
    pTimer->Prev->Next = pTimer->Next;                                      /* 删除指定定时器控制列表               */
    pTimer->Next->Prev = pTimer->Prev;
    memset(pTimer, NULL, sizeof(TIMER));

    return OS_OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

