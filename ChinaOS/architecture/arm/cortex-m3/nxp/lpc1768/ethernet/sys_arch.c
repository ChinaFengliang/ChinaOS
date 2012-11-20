/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            sys_arch.c
** Last version:         V1.00
** Descriptions:         lwip系统接口层文件.
** Hardware platform:    
** SoftWare platform:    ChinaOS & lwip
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2012年2月8日  14:55:13
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
#include    <lwip/debug.h>
#include    <lwip/sys.h>
#include    <lwip/opt.h>
#include    <lwip/stats.h>
#include    <arch/sys_arch.h>
#include    <syscall.h>
#include    <nxp/lpc1768/timer/millisecond.h>

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
int errno;


/*********************************************************************************************************************
** Function name:           sys_init
** Descriptions:            must be called before anthing else.
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  16:18:51
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void sys_init(void)
{

}

/*********************************************************************************************************************
** Function name:           sys_sem_new
** Descriptions:            Creates and returns a new semaphore. The "count" argument specifies  the initial state
**                          of the semaphore.
** Input parameters:        count
** Output parameters:       
** Returned value:          !=ERR_OK : 操作失败
**                          ==ERR_OK : 操作成功(包含消息邮箱)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  14:58:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
err_t sys_sem_new(sys_sem_t* pSemaphore, u8_t count)
{
    if (NULL == pSemaphore)
    {
        return ERR_ARG;
    }
    *pSemaphore = semaphore_new(count);
    if (NULL == *pSemaphore)
    {
        return ERR_MEM;
    }
    
    return ERR_OK;
}

/*********************************************************************************************************************
** Function name:           sys_sem_valid
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==1 : 合法信号量
**                          ==0 : 非法信号量
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-29  18:16:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int sys_sem_valid(sys_sem_t *pSemaphore)
{
    if (NULL != pSemaphore)
    {
        if (NULL != *pSemaphore)
        {
            return 1;
        }
    }
    
    return 0;
}

/*********************************************************************************************************************
** Function name:           sys_sem_set_invalid
** Descriptions:            Set a semaphore invalid so that sys_sem_valid returns 0
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-29  18:18:54
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void sys_sem_set_invalid(sys_sem_t *pSemaphore)
{
    if (NULL != pSemaphore)
    {
        *pSemaphore = NULL;
    }
}

/*********************************************************************************************************************
** Function name:           sys_sem_free
** Descriptions:            Deallocates a semaphore.
** Input parameters:        pSemaphore : 信号量
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  14:59:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void sys_sem_free(sys_sem_t *pSemaphore)
{
    semaphore_free(*pSemaphore);
}

/*********************************************************************************************************************
** Function name:           sys_sem_signal
** Descriptions:            Signals a semaphore.
** Input parameters:        Semaphore : 信号量
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  14:59:31
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void sys_sem_signal(sys_sem_t *pSemaphore)
{
    semaphore_post(*pSemaphore);
}

/*********************************************************************************************************************
** Function name:           sys_arch_sem_wait
** Descriptions:            Blocks the thread while waiting for the semaphore to be  signaled. If the "timeout" 
**                          argument is non-zero, the thread should  only be blocked for the specified time (measured
**                          in  milliseconds). If the "timeout" argument is zero, the thread should be  blocked until
**                          the semaphore is signalled.
** Input parameters:        Semaphore : 信号量
**                          Timeout   : 等待时间(单位: 毫秒)
**                                      == 0: 无限期等待
**                                      != 0: 等待时间
** Output parameters:       
** Returned value:          If the timeout argument is non-zero, the return value is the number of  milliseconds spent
**                          waiting for the semaphore to be signaled. If the  semaphore wasn't signaled within the
**                          specified time, the return value is  SYS_ARCH_TIMEOUT. If the thread didn't have to wait
**                          for the semaphore  (i.e., it was already signaled), the function may return zero.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:0:6
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
u32_t sys_arch_sem_wait(sys_sem_t *pSemaphore, u32_t Timeout)
{
    INT32U       TimeStart, TimeEnd;

    
    TimeStart = mtimer.gettick();
    if (OS_ERR_TIMEOUT == semaphore_wait(*pSemaphore, Timeout))
    {
        return SYS_ARCH_TIMEOUT;
    }
    TimeEnd= mtimer.gettick();
    
    return TimeEnd - TimeStart;
}

/*********************************************************************************************************************
** Function name:           sys_mbox_new
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:3:12
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
err_t sys_mbox_new(sys_mbox_t *pMailbox, int size)
{
	if (NULL == pMailbox)
	{
		return ERR_ARG;
	}
	
    *pMailbox = mailbox_new(size);
    if (NULL == *pMailbox)
    {
		return ERR_MEM;
    }
    
    return ERR_OK;
}

/*********************************************************************************************************************
** Function name:           sys_mbox_valid
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==1 : 合法信号量
**                          ==0 : 非法信号量
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-29  18:20:43
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int sys_mbox_valid(sys_mbox_t *pMailbox)
{
    if (NULL != pMailbox)
    {
        if (NULL != *pMailbox)
        {
			return 1;
        }
    }
    return 0;
}

/*********************************************************************************************************************
** Function name:           sys_mbox_set_invalid
** Descriptions:            Set an mbox invalid so that sys_mbox_valid returns 0
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-29  18:21:29
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void sys_mbox_set_invalid(sys_mbox_t *pMailbox)
{
    if (NULL != pMailbox)
    {
        *pMailbox = NULL;
    }
}

/*********************************************************************************************************************
** Function name:           sys_mbox_free
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:4:31
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void sys_mbox_free(sys_mbox_t *mbox)
{
    mailbox_free(*mbox);
}

/*********************************************************************************************************************
** Function name:           sys_mbox_post
** Descriptions:            Posts the "msg" to the mailbox. This function have to block until  the "msg" is really
**                          posted.
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:4:45
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    int             Status;

    
    do
    {
        Status = mail_post(*mbox, msg);
    } while (OS_OK != Status);
}

/*********************************************************************************************************************
** Function name:           sys_mbox_trypost
** Descriptions:            Try to post the "msg" to the mailbox. Returns ERR_MEM if this one  is full, else, ERR_OK if the "msg" is posted.
** Input parameters:        
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:4:56
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)    
{
    if (OS_OK == mail_post(*mbox, msg))
    {
        return ERR_OK;
    }
    else
    {
        return ERR_MEM;
    }
}

/*********************************************************************************************************************
** Function name:           sys_arch_mbox_fetch
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a  timeout.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:5:0
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
    MAIL            Mail;
    INT32U          TimeStart, TimeEnd;


    if (NULL == msg)
    {
        msg = &Mail;
    }
    
    TimeStart = mtimer.gettick();
    if (OS_ERR_TIMEOUT == mail_wait(*mbox, msg, timeout))
    {
        return SYS_ARCH_TIMEOUT;
    }
    TimeEnd= mtimer.gettick();

    return TimeEnd - TimeStart;
}

/*********************************************************************************************************************
** Function name:           sys_arch_mbox_tryfetch
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:5:5
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)    
{
    if (OS_OK == mail_fetch(*mbox, msg))
    {
        return 0;
    }
    else
    {
        return SYS_MBOX_EMPTY;
    }
}

/*********************************************************************************************************************
** Function name:           sys_arch_timeouts
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-9  0:41:56
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
struct sys_timeouts *sys_arch_timeouts(void)
{
    THREAD              *pThread;

    pThread = (THREAD *)getid();
    
    return (struct sys_timeouts *)&(pThread->pAttachment);
}

/*********************************************************************************************************************
** Function name:           sys_thread_new
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:5:10
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
sys_thread_t sys_thread_new(const char *name, void (* thread)(void *arg), void *arg, int stacksize, int prio)
{
    return labour((char *)name, (int (*)(void *))thread, arg, stacksize, prio);
}

/*********************************************************************************************************************
** Function name:           sys_arch_protect
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:5:19
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
sys_prot_t sys_arch_protect(void)
{
    return atom_operate_lock();
}

/*********************************************************************************************************************
** Function name:           sys_arch_unprotect
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-8  15:5:25
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void sys_arch_unprotect(sys_prot_t pval)
{
    atom_operate_unlock(pval);
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

