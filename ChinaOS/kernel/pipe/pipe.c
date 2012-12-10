/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            pipe.c
** Last version:         V1.00
** Descriptions:         阻塞管道文件.
** Hardware platform:    
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011年5月22日  0:34:19
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
#include    <syscall.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <kernel/kernel.h>
#include    <library/malloc/malloc.h>
#include    <library/math/math.h>
#include    "./pipe.h"

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/
struct pipe_t
{    
    INT8U         *pBuffer;                                                 /* the buffer holding the data          */    
    INT32U         Lenth;                                                   /* 管道长度                             */
    INT32U         Width;                                                   /* 管道宽度 number of bytes per element */
    INT32U         In;                                                      /* 输入索引                             */    
    INT32U         Out;                                                     /* 输出索引 data is extracted from off. */
    THREAD        *pReadThread;                                             /* 管道接收端线程                       */
};  

/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/


/*********************************************************************************************************************
** Function name:           pipe_new
** Descriptions:            新建管道
** Input parameters:        Width  : 管道宽度(元素大小)
**                          Length : 管道长度
** Output parameters:       
** Returned value:          ==NULL : 操作失败
**                          !=NULL : 操作成功(包含管道信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-23  19:33:38
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void * pipe_new(unsigned int Width, unsigned int Length)    
{    
    struct pipe_t           *pPipe;                                         /* 管道对象指针                         */


    if ((0 == Length) || (0 == Width))
    {
        return NULL;
    }
    
    /*
     * round up to the next power of 2, since our 'let the indices   
     * wrap' tachnique works only in this case.   
     */    
    if (Length & (Length - 1))
    {    
        Length = (Length & -Length) << 1;
    }
  
    pPipe = (struct pipe_t *)malloc(Length * Width + sizeof(struct pipe_t));    
    if (NULL != pPipe)    
    {
        pPipe->pReadThread = NULL;
        pPipe->Lenth   = Length;
        pPipe->Width   = Width;
        pPipe->In      = pPipe->Out = 0;
        pPipe->pBuffer = (unsigned char *)pPipe + sizeof(struct pipe_t);
    }

    return (void *)pPipe;
}  

/*********************************************************************************************************************
** Function name:           pipe_free
** Descriptions:            释放管道
** Input parameters:        Pipe : 管道对象
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-23  19:53:17
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void pipe_free(void * Pipe)
{
    free(Pipe);
}

/*********************************************************************************************************************
** Function name:           pipe_write
** Descriptions:            向管道写入数据
** Input parameters:        Pipe      : 管道对象
**                          pElements : 数据缓存区
**                          Length    : 数据元素个数
** Output parameters:       
** Returned value:          实际写入的数据元素个数
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-23  19:53:22
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:             当pipe已经满时，写入会不会覆盖
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int pipe_write(void * Pipe, void * pElements, unsigned Length)
{
    THREAD              *Thread;                                            /* 线程控制块                           */    
    INT32U               Size;                                              /* 数据长度                             */
    INT32U               Priority;                                          /* 优先级                               */
    int                  Key;                                               /* 临界锁                               */
    struct pipe_t       *pPipe = (struct pipe_t *)Pipe;                     /* 管道                                 */
    char                *pInputData;                                        /* 输入数据                             */


    if ((NULL == Pipe) || (NULL == pElements))
    {
        return ERR_BAD_PARAM;
    }

    /*
     * 1) 拷贝数据
     */
    pInputData = (char *)pElements;
    
    /*
     * pPipe->In - pPipe->Out : 表示队列里面已使用的空间大小;
     * pPipe->Lenth - (pPipe->In - pPipe->Out)表示队列未使用的空间.
     */
    Length = min(Length, pPipe->Lenth - pPipe->In + pPipe->Out);            /* 取两者之小，表示实际要拷贝的字节数   */
  
    /*
     * first put the data starting from pPipe->in to buffer end
     */    
    Size = min(Length, pPipe->Lenth - (pPipe->In & (pPipe->Lenth - 1)));    
    memcpy(pPipe->pBuffer + pPipe->Width * (pPipe->In & (pPipe->Lenth - 1)),
           pInputData,
           (size_t)Size * pPipe->Width);
  
    /*
     * then put the rest (if any) at the beginning of the buffer
     */
    memcpy(pPipe->pBuffer, pInputData + Size * pPipe->Width,
           (size_t)(Length - Size) * pPipe->Width);    
  
    pPipe->In += Length;

    if (NULL == (Thread = pPipe->pReadThread))                              /* 没有线程等待                         */
    {
        goto exit;
    }

    /* 
     * 2) 线程就绪
     *    将管道等待队列中线程控制表插入到指定线程队列.
     */    
    Priority = Thread->Priority;     
    Key = atom_operate_lock();
    list_add(&Thread->Node, &OS_ActiveProc[Priority]);
    OS_ActivePrio |= 1ul << Priority;
    atom_operate_unlock(Key);

    /* 
     * 3) 切换线程
     */
    scheduler();

exit:
    return Length;    
}
  
/*********************************************************************************************************************
** Function name:           pipe_read
** Descriptions:            从管道读数据
** Input parameters:        Pipe      : 管道对象
**                          pElements : 数据缓存区
**                          Length    : 数据元素个数
** Output parameters:       
** Returned value:          实际读出的数据元素个数
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng liang
** Created Date:            2011-5-23  19:53:30
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int pipe_read(void * Pipe, void * pElements, unsigned Length, int Timeout)
{    
    INT32U               Size;    
    THREAD              *pThread;                                           /* 线程                                 */    
    INT32U               Priority;                                          /* 优先级                               */    
    int                  Key;
    char                *pOutputData;                                       /* 输出数据                             */
    struct pipe_t       *pPipe = (struct pipe_t *)Pipe;   


    if ((NULL == Pipe) || (NULL == pElements))
    {
        return ERR_BAD_PARAM;
    }
    
    /*
     * 从管道缓存区读出数据(满递增的方式)
     */
    pOutputData = (char *)pElements;
    Key = atom_operate_lock();
    if (pPipe->In != pPipe->Out)
    {
        Length = min(Length, pPipe->In - pPipe->Out);    
      
        /* first get the data from pPipe->out until the end of the buffer */
        Size = min(Length, pPipe->Lenth - (pPipe->Out & (pPipe->Lenth - 1)));
        memcpy(pOutputData, 
               pPipe->pBuffer + pPipe->Width * (pPipe->Out & (pPipe->Lenth - 1)),
               (size_t)Size * pPipe->Width);
      
        /* then get the rest (if any) from the beginning of the buffer */
        memcpy(pOutputData + Size * pPipe->Width, pPipe->pBuffer,
               (size_t)(Length - Size) * pPipe->Width);
      
        pPipe->Out += Length;
        
        atom_operate_unlock(Key);
        return Length;
    }
    
    pThread = OS_This;
    pPipe->pReadThread = pThread;
    
    /* 
     * 1) 将当前线程挂起
     */
    Priority = pThread->Priority;
    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {   
        OS_ActivePrio &= ~(1ul << Priority);
    }

    /*
     * 2) 设置超时唤醒条件
     */
    if (Timeout)
    {
        OS_timer_add(&pThread->Timer, Timeout);                             /* 添加唤醒定时器                       */
    }
    atom_operate_unlock(Key);

    /* 
     * 3) 切换线程
     */
    OS_thread_switch();                                                     /* 切换到其它线程                       */

    /*
     * 4.1) 删除超时唤醒条件
     */
    Key = atom_operate_lock();
    if (Timeout)
    {
        OS_timer_del(&pThread->Timer);                                      /* 删除唤醒定时器                       */    
    }
    pPipe->pReadThread = NULL;
    
    /*
     * 5) 从管道缓存区读出数据(满递增的方式)
     */
    if (pPipe->In == pPipe->Out)                                            /* 队列为空                             */
    {        
        atom_operate_unlock(Key);
        return ERR_TIMEOUT; 
    }

    Length = min(Length, pPipe->In - pPipe->Out);
  
    /* first get the data from pPipe->out until the end of the buffer */
    Size = min(Length, pPipe->Lenth - (pPipe->Out & (pPipe->Lenth - 1)));
    memcpy(pOutputData,
           pPipe->pBuffer + pPipe->Width * (pPipe->Out & (pPipe->Lenth - 1)),
           (size_t)Size * pPipe->Width);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(pOutputData + Size * pPipe->Width, pPipe->pBuffer,
           (size_t)(Length - Size) * pPipe->Width);
  
    pPipe->Out += Length;

    atom_operate_unlock(Key);
    return Length;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/


