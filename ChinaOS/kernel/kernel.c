/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------+
** File name:            Kernel.c
** Last version:         V1.00
** Descriptions:         ChinaOS操作系统管理文件.
** Hardware platform:    ARM7、ARM9、Cortex-M0、Cortex-M3
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2010年8月25日  14:27:47
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
#define DEBUG_LOCAL_EN                              0                       /* 本地调试开关(默认配置): 0:关; 1:开   */
/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <string.h>
#include    <syscall.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <include/ticker.h>
#include    <kernel/kernel.h>
#include    <include/debug.h>
#include    <library/console/console.h>
#include    <library/malloc/malloc.h>
#include    <library/misc/misc.h>
#include    <library/bit/bit.h>
#include    <library/link/list.h>
#include    <device.h>

/*********************************************************************************************************************
                                                   宏定义区
*********************************************************************************************************************/
/* 系统相关配置 ----------------------------------------------------------------------------------------------------*/
#define     THREAD_MAIN_PRIORITY                    1                       /* 根线程优先级(范围:[0, 31])           */
#define     THREAD_MAIN_STACK_SIZE                  2040                    /* 根线程程栈深度(单位:byte)            */
#define     THREAD_IDLE_STACK_SIZE                  256                     /* 空闲线程栈深度(单位:byte)            */

/*********************************************************************************************************************
                                                   类型定义区
*********************************************************************************************************************/
/* 空闲线程控制表类型 ----------------------------------------------------------------------------------------------*/
struct __idle_pcl
{
    REGISTER           *pStackTop;                                          /* 线程栈顶指针(要求: 结构体第一元素)   */
    REGISTER           *pStackBottom;                                       /* 线程栈底指针                         */
    INT32U              Priority;                                           /* 线程优先权                           */
    char               *pName;                                              /* 线程名称                             */
};
typedef     struct __idle_pcl                       IDLE_PCL;               /* 空闲线程控制表类型                   */

/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
INT32U                  OS_ActivePrio;                                      /* 激活的优先级位标志                   */
struct list_head        OS_ActiveProc[33];                                  /* 激活的线程队列                       */
THREAD                 *OS_This;                                            /* 当前运行的线程                       */
THREAD                 *OS_LiveList;                                        /* 激活线程链表                         */

struct list_head        OS_DeadList;                                        /* 僵尸线程链表                         */
static THREAD           OS_Idle;                                            /* 空闲线程控制表                       */
static REGISTER         OS_aIdleStack[THREAD_IDLE_STACK_SIZE];              /* 空闲线程栈                           */

/*********************************************************************************************************************
** Function name:           main
** Descriptions:            根线程
** Input parameters:
** Output parameters:
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-25  14:29:25
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
extern int main(void *option);

/*********************************************************************************************************************
** Function name:           mallocor_setup
** Descriptions:            安装内存分配器
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-20  18:30:6
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
STATUS mallocor_setup(void);

/*********************************************************************************************************************
** Function name:           stack_init
** Descriptions:            初始化线程栈
** Input parameters:        pProcess     : 线程函数名
**                          pOption      : 选项
**                          pStackBottom : 栈底
** Output parameters:       
** Returned value:          当前栈顶指针
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-26  9:22:9
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
REGISTER *stack_init(int (*pProcess)(void *), void *pOption, void *pStackBottom);


/*********************************************************************************************************************
** Function name:           thread_wakeup
** Descriptions:            线程
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-11-27  0:7:10
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Test recorde:            
*********************************************************************************************************************/
INLINE void vivify_thread(THREAD *pControl, INT32U Priority)
{
    list_del(&pControl->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {
        OS_ActivePrio &= ~(1ul << Priority);                                /* 清除就绪标志                         */
    }
}

/*********************************************************************************************************************
** Function name:           thread_register
** Descriptions:            注册线程
** Input parameters:        pThread : 线程控制表
** Output parameters:       OS_LiveList : 线程注册链表
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-5  17:8:18
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void thread_register(THREAD *pThread)
{
    THREAD          **ppNode;

    
    /*
     * 将派生线程注册到激活链表的尾部
     */
    ppNode = &OS_LiveList;
    while (NULL != *ppNode)
    {
        ppNode = &((*ppNode)->pLive);
    }
    
    *ppNode = pThread;                                                      /* 链接到尾部                           */
}

/*********************************************************************************************************************
** Function name:           thread_unregister
** Descriptions:            注销线程
** Input parameters:        pThread : 线程控制表
** Output parameters:       OS_LiveList : 线程注册链表
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-5  20:38:58
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void thread_unregister(THREAD *pThread)
{
    THREAD          **ppNode;

    
    /*
     * 将线程从激活链表中删除
     */
    ppNode = &OS_LiveList;
    while (pThread != *ppNode)
    {
        ppNode = &((*ppNode)->pLive);
    }
    
    *ppNode = pThread->pLive;
}

/*********************************************************************************************************************
** Function name:           exit
** Descriptions:            线程出口
** Input parameters:        Info     : 线程返回值
** Output parameters:
** Returned value:
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-30  14:24:35
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
void exit(INT32S Info)
{
    THREAD          *pThread;                                               /* 线程控制块                           */
    INT32U           Priority;
    int              Key;


    pThread = OS_This;
    if (NULL != pThread->pExit)
    {
        pThread->pExit(Info);
    }

    /* 
     * 删除运行控制登记表
     */
    Priority = pThread->Priority;
    Key = atom_operate_lock();    
    thread_unregister(pThread);

    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {   
        OS_ActivePrio &= ~(1ul << Priority);
    }

    /*
     * 删除线程资源
     * 1) 线程定时器;
     */
    OS_timer_del(&pThread->Timer);
    
    /*
     * 插入到僵尸线程链表,等待系统回收线程资源.
     */
    list_add(&pThread->Node, &OS_DeadList);
    atom_operate_unlock(Key);

   /* 
    * 切换到其它线程
    */
    OS_thread_switch();
}

/*********************************************************************************************************************
** Function name:           kill
** Descriptions:            杀死其它线程(除本线程外)
** Input parameters:        Thread : 线程ID号
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-27  11:42:14
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
STATUS kill(INT32S Thread)
{
    /*
     * 本函数是否只能杀死其它线程, 不能杀死本线程.
     * 若要杀死本线程,请调用exit函数.
     */
    THREAD     *pThread;                                                    /* 线程控制块                           */
    INT32U      Priority;                                                   /* 优先级                               */
    int         Key;


    /*
     * 获取指定线程控制表
     */
    if (NULL == Thread)
    {
        return ERR_BAD_PARAM;
    }
    pThread = (THREAD*)Thread;

    /*
     * 执行线程终止回调函数
     */
    if (NULL != pThread->pKill)
    {
        pThread->pKill();
    }

    /*
     * 注销线程
     */
    Key = atom_operate_lock();
    thread_unregister(pThread);
    Priority = pThread->Priority;

    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {
        OS_ActivePrio &= ~(1ul << Priority);                                /* 清除就绪标志                         */
    }

    OS_timer_del(&pThread->Timer);                                          /* 删除线程定时器                       */
    atom_operate_unlock(Key);
    
    free(pThread);

    return OK;
}



/*********************************************************************************************************************
** Function name:           signal
** Descriptions:            注册信号事件
** Input parameters:        SigNum     : 事件信号
**                          pFunction  : 处理函数
** Output parameters:       
** Returned value:          ==OK    : 操作成功
**                          !=OK    : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-25  16:20:32
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
INT32S signal(INT32S SigNum, void *pFunction)
{
    THREAD             *This;
    int                 Key;
    
    
    Key = atom_operate_lock();
    This = OS_This;
    atom_operate_unlock(Key);

    switch (SigNum)
    {
        case SIG_TERM :
            /* 设置当前线程的返回回调函数 */
            This->pExit = (void(*)(INT32S))pFunction;
            break;

        case SIG_KILL :
            /* 设置被杀后回调函数 */
            This->pKill = (void(*)(void))pFunction;
            break;

        default :
            return ERR;
    }

    return OK;
}

/*********************************************************************************************************************
** Function name:           shift_thread
** Descriptions:            线程交替
** Input parameters:        pStackTop : 当前线程的栈顶指针
** Output parameters:
** Returned value:          下一个运行线程的栈顶指针
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-1-5  8:45:59
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
REGISTER * shift_thread(REGISTER *pStackTop)
{
    INT32S          Priority;

    OS_This->pStackTop = pStackTop;                                         /* 保存当前线程的栈顶指针               */

    Priority = bit_scan_forward(OS_ActivePrio);
    OS_This  = container_of(&OS_ActiveProc[Priority].next->next, THREAD, Node);

    return OS_This->pStackTop;
}

/*********************************************************************************************************************
** Function name:           scheduler
** Descriptions:            调度决策
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-4  18:22:40
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void scheduler(void)
{
    /*
     * 当前线程优先级数值是就绪表中最小值, 即当前线程优先级最高.
     */
    if (bit_scan_forward(OS_ActivePrio) < OS_This->Priority)
    {
        OS_thread_switch();
    }
}

/*********************************************************************************************************************
** Function name:           labour
** Descriptions:            派生线程
** Input parameters:        pName       : 线程名称
**                          pProcess    : 线程函数名
**                          pOption     : 参数
**                          StackDepth  : 栈深度
**                          Priority    : 优先权[0, 31]
** Output parameters:
** Returned value:          !=NULL      : 线程ID
**                          ==NULL      : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-26  8:50:3
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
INT32S labour(const char   *pName,
              int         (*pProcess)(void *),
              void         *pOption,
              INT32U        StackDepth,
              INT32U        Priority)
{
    INT8U       *pStackBottom;                                              /* 栈底地址                             */
    THREAD      *pControl;                                                  /* 线程控制表                           */


    /* 
     * 0) 参数过滤
     */
    if ((Priority & ~0x1F) ||
        (NULL == pProcess) ||
        (0 == StackDepth))
    {
        DBG_ERR("输入参数错误");
        return NULL;
    }
    StackDepth = align_upside(StackDepth, sizeof(REGISTER));
    
    /*
     * 1) 初始化栈空间
     */
    pStackBottom = (INT8U*)malloc(StackDepth + sizeof(THREAD));             /* 栈空间 + 控制块空间                  */
    if (NULL == pStackBottom)
    {
        DBG_WARN("内存不足\r\n");
        return NULL;
    }
    
    pControl = (THREAD*)pStackBottom;
    memset(pControl, NULL, StackDepth + sizeof(THREAD));                    /* 清零控制块内容                       */
    pStackBottom  += sizeof(THREAD);                                        /* 4字节对齐问题                        */
    
#   if OS_STACK_GROWTH == HIGH_TO_LOW
    pStackBottom += StackDepth - sizeof(REGISTER);
#   endif

    pControl->pStackBottom = (REGISTER*)pStackBottom;
    pControl->pStackTop    = stack_init(pProcess, pOption, pStackBottom);   /* 保存栈顶指针                         */
    pControl->Priority     = Priority;
    pControl->pName        = pName;
    thread_register(pControl);
    
    /*
     * 2) 注册线程
     * 将线程控制表插入到指定线程队列
     */
    list_add(&pControl->Node, &OS_ActiveProc[Priority]);
    OS_ActivePrio |= 1ul << Priority;
    
    /*
     * 3) 调度决策
     * 如果派生线程的优先级低于当前线程优先级,则返回至当前线程,否则调度运行派生线程.
     */
    if (Priority < OS_This->Priority)
    {
        OS_thread_switch();                                                 /* 调度切换情景                         */
    }

    return (INT32S)pControl;
}

/*********************************************************************************************************************
** Function name:           getid
** Descriptions:            获得当前线程ID
** Input parameters:
** Output parameters:
** Returned value:          线程标识符
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-30  9:23:9
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
INT32S getid(void)
{
    return (INT32S)OS_This;
}

/*********************************************************************************************************************
** Function name:           rename
** Descriptions:            线程命名
** Input parameters:        pName : 线程名称
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-4  23:9:4
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int rename(char *pName)
{
    OS_This->pName = pName;
    return OK;
}

/*********************************************************************************************************************
** Function name:           change_priority
** Descriptions:            改变线程优先级
** Input parameters:        id       : 线程ID
**                          Priority : 优先级
** Output parameters:
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-30  14:14:18
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
INT32S change_priority(INT32U id, INT32U Priority)
{
    THREAD         *pThis;

    pThis = (THREAD *)id;
    pThis->Priority = Priority;
    return OK;
}

/*********************************************************************************************************************
** Function name:           idle
** Descriptions:            空闲线程
** Input parameters:        option  : (保留)
** Output parameters:       none
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-30  15:35:0
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
static int idle(void *option)
{
    /* 设想
     * 1) 系统闲时进行内存整理;
     * 2) 系统空闲时进行shell服务
     */
    
    /* 
     * 1) 安装硬/软组件.
     *    为了设备安装时正常申请内存,必须先执行mallocor初始化,再执行设备安装.
     */
    if (OK != mallocor_setup())
    {
        DBG_ERR("内存分配器安装失败\r\n");
    }
    if (OK != console_setup())
    {
        DBG_ERR("控制台安装失败\r\n");
    }
    systicker_setup();
    device_setup();
    
    labour("main", main, NULL, 2040, THREAD_MAIN_PRIORITY);

    while (1)
    {
        struct list_head        *pNode;
        /*
         * 2) 释放线程空间
         */
        list_for_each(pNode, &OS_DeadList)
        {
            THREAD *pThread = container_of(&pNode->next, THREAD, Node);

            list_del(&pThread->Node);
            free(pThread);
        }
        
        /*
         * 5) 提供控制台服务
         */
        command();
    }
}

/*********************************************************************************************************************
** Function name:           sleep
** Descriptions:            睡眠
** input parameters:        Time: 延时时间(单位: 毫秒)
** output parameters:       
** Returned value:          == OK: 正常唤醒
**                          != OK: 异常唤醒
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-01-06
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test record:
**--------------------------------------------------------------------------------------------------------------------
*********************************************************************************************************************/
INT32S sleep(INT32U Time)
{    
    THREAD     *pThread;                                                    /* 线程控制块                           */
    INT32U      Priority;                                                   /* 优先级                               */
    int         Key;


    if (0 == Time)
    {
        return OK;
    }


   /* 
    * 1) 将当前线程挂起,删除运行控制登记表.
    */  
    pThread = OS_This;  
    Priority = pThread ->Priority;

    Key = atom_operate_lock();
    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {   
        OS_ActivePrio &= ~(1ul << Priority);
    }

    OS_timer_add(&OS_This->Timer, Time);
    atom_operate_unlock(Key);   
   
   /* 
    * 2) 切换线程
    */
    OS_thread_switch();                                                     /* 切换到其它线程                       */

    OS_timer_del(&OS_This->Timer);                                          
   
    return OK;
}

/*********************************************************************************************************************
** Function name:           wakeup
** Descriptions:            唤醒
** Input parameters:        Thread      : 线程ID
** Output parameters:
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-26  11:26:55
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
INT32S wakeup(INT32S Thread)
{
    /*
     * 不存在指定线程时, 返回出错信息;
     */

    return ERR;
}

/*********************************************************************************************************************
** Function name:           startup
** Descriptions:            启动ChinaOS操作系统
** Input parameters:
** Output parameters:
** Returned value:
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-30  14:31:46
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
void startup(void)
{
    REGISTER            *pStackBottom;
    int                  i;
    
    
    /*
     * 1) 复位管理信息
     */
    OS_ActivePrio = 0;
    INIT_LIST_HEAD(&OS_DeadList);
    for (i = 0; i < 33; i++)
    {
        INIT_LIST_HEAD(&OS_ActiveProc[i]);
    }
    
    /*
     * 2) 构建空闲线程
     */
    pStackBottom  = OS_aIdleStack;
#if OS_STACK_GROWTH == HIGH_TO_LOW
    pStackBottom += THREAD_IDLE_STACK_SIZE - 1;                             /* 调整至栈底                           */
#endif
    OS_Idle.pStackBottom = pStackBottom;
    OS_Idle.pStackTop = stack_init(idle, NULL, pStackBottom);
    OS_Idle.Priority  = 32;
    OS_Idle.pName     = "idle";

    list_add(&OS_Idle.Node, &OS_ActiveProc[32]);
    OS_This = &OS_Idle;

    thread_register(OS_This);
    
    OS_activate_system();                                                   /* idle线程将被系统激活                 */
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

