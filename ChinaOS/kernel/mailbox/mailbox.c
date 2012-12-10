/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            mail.c
** Last version:         V1.00
** Descriptions:         邮件通信机制实现文件.
** Hardware platform:    
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011年2月23日  13:59:27
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
#include    <syscall.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <kernel/kernel.h>
#include    <kernel/mailbox/mailbox.h>
#include    <library/malloc/malloc.h>

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/
#define     MAILBOX_EMPTY                   0ul                             /* 邮件箱空                             */
#define     MAILBOX_FULL                    1ul                             /* 邮件箱满                             */
/* 邮箱状态 --------------------------------------------------------------------------------------------------------*/
#define     MAILBOX_VALID                   0ul                             /* 有效邮箱                             */
#define     MAILBOX_TRASH                   1ul                             /* 废弃邮箱                             */

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/


/*********************************************************************************************************************
** Function name:           mailbox_new
** Descriptions:            新建消息邮箱
** Input parameters:        Size : 邮箱大小
** Output parameters:       
** Returned value:          ==NULL  : 操作失败
**                          !=NULL  : 操作成功(包含消息邮箱)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-2-23  17:12:36
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
MAILBOX mailbox_new(INT32U Size)
{
    struct mailbox_t     *pMailbox = NULL;
    INT32U                Byte;
    
    if (0 == Size)
    {
        goto exit;
    }
    
    Byte = Size * sizeof(MAIL) + sizeof(struct mailbox_t);
    if (NULL != (pMailbox = (struct mailbox_t *)malloc(Byte)))
    {
        pMailbox->Size    = Size;
        pMailbox->Read    = 0;
        pMailbox->Write   = 0;
        INIT_LIST_HEAD(&pMailbox->WaitHead);
    }
    
exit:
    return pMailbox;
}

/*********************************************************************************************************************
** Function name:           mailbox_free
** Descriptions:            释放消息邮箱
** Input parameters:        Mailbox : 消息邮箱
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-4-13  2:33:9
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int mailbox_free(MAILBOX Mailbox)
{
    /*
     * 问题 : 当A线程进入等待接收来自本邮箱的消息时, B线程释放本消息邮箱(包括存储空间). 
     *        由此造成mail_wait函数唤醒后访问非法内存.
     * 方案 : 1) mailbox_free只发出本邮箱终止服务命令;
     *        2) 将所有等待线程唤醒;
     *        2) 在所有线程退出等待后, 最终才释放本消息邮箱.
     */
    free(Mailbox);
    return OK;
}

/*********************************************************************************************************************
** Function name:           mailbox_create
** Descriptions:            创建邮件箱
** Input parameters:        Mailbox : 消息邮箱
**                          Size    : 邮箱大小
** Output parameters:       
** Returned value:          ==OK  : 操作成功
**                          !=OK  : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-2-23  17:12:36
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int mailbox_create(MAILBOX Mailbox, INT32U Size)
{
    struct mailbox_t     *pMailbox;
    
    
    if (NULL == Mailbox)
    {
        return ERR_BAD_PARAM;
    }

    pMailbox = (struct mailbox_t *)Mailbox;
    pMailbox->Size    = Size;
    pMailbox->Read    = 0;
    pMailbox->Write   = 0;
    pMailbox->Status  = VALID;
    INIT_LIST_HEAD(&pMailbox->WaitHead);
    
    return OK;
}

/*********************************************************************************************************************
** Function name:           mail_fetch
** Descriptions:            非阻塞方式获取邮件. 
** Input parameters:        pMailbox : 邮件箱的指针
** Output parameters:       pMessage : 邮件指针
** Returned value:          ==OK : 操作成功。信箱非空,获取邮件成功.
**                          !=OK : 操作失败. 信箱为空,获取邮件失败.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-10  18:20:52
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int mail_fetch(MAILBOX Mailbox, MAIL *pMessage)
{
    struct mailbox_t    *pMailbox;                                          /* 信箱指针                             */
    int                  Key;

    /*
     * 从队列读出邮件(满递增的方式)
     */
    pMailbox = (struct mailbox_t *)Mailbox;
    
    Key = atom_operate_lock();
    if (pMailbox->Read != pMailbox->Write)                                  /* 队列不为空                           */
    {
        if (pMailbox->Read == pMailbox->Size)
        {
            pMailbox->Read = 0;
        }
        else
        {
            pMailbox->Read++;
        }
        *pMessage = pMailbox->Mail[pMailbox->Read];
        
        atom_operate_unlock(Key);
        return OK;
    }
    atom_operate_unlock(Key);

    return ERR;
}

/*********************************************************************************************************************
** Function name:           mail_wait
** Descriptions:            等待邮件
** Input parameters:        pMailbox : 邮件箱的指针
**                          Timeout  : 超时时间(单位: 毫秒)
**                                      ==0: 永久等待.
**                                      !=0: 等待时间.
** Output parameters:       pMessage : 邮件指针
** Returned value:          == OK : 操作成功
**                          != OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-20  16:8:24
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
int mail_wait(MAILBOX Mailbox, MAIL *pMessage, INT32U Timeout)
{
    struct mailbox_t    *pMailbox;                                          /* 信箱指针                             */
    THREAD              *pThread;                                           /* 线程                                 */    
    INT32U               Priority;                                          /* 优先级                               */    
    int                  Key;


    if ((NULL == Mailbox) || (NULL == pMessage))                            /* 参数过滤                             */
    {
        return ERR_BAD_PARAM;
    }

    /*
     * 从队列读出邮件(满递增的方式)
     */
    pMailbox = (struct mailbox_t *)Mailbox;
    Key = atom_operate_lock();
    if (pMailbox->Read != pMailbox->Write)                                  /* 队列不为空                           */
    {
        if (pMailbox->Read == pMailbox->Size)
        {
            pMailbox->Read = 0;
        }
        else
        {
            pMailbox->Read++;
        }
        *pMessage = pMailbox->Mail[pMailbox->Read];
        
        atom_operate_unlock(Key);
        return OK;
    }
    
    /* 
     * 1) 将当前线程挂起
     */
    pThread = OS_This;
    Priority = pThread->Priority;
    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {   
        OS_ActivePrio &= ~(1ul << Priority);
    }
    
    /* 
     * 2) 添加唤醒条件
     */
    list_add(&pThread->Node, &pMailbox->WaitHead);
    
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
     * 4) 删除唤醒条件
     */
    
    /*
     * 4.1) 删除超时唤醒条件
     */
    Key = atom_operate_lock();
    if (Timeout)
    {
        OS_timer_del(&pThread->Timer);                                      /* 删除唤醒定时器                       */    
    }
    /*
     * 4.2) 退出邮件等待队列
     */
    list_del(&pThread->Node);
    
    /*
     * 5) 收取邮件
     */    
    if (pMailbox->Read == pMailbox->Write)                                  /* 队列为空                             */
    {        
        atom_operate_unlock(Key);
        return ERR_TIMEOUT; 
    }
    if (pMailbox->Read == pMailbox->Size)
    {
        pMailbox->Read = 0;
    }
    else
    {
        pMailbox->Read++;
    }
    *pMessage = pMailbox->Mail[pMailbox->Read];
    atom_operate_unlock(Key);   
    
    return OK;
}

/*********************************************************************************************************************
** Function name:           mail_post
** Descriptions:            发送邮件
** Input parameters:        pMailbox : 邮件箱的指针
**                          Message  : 邮件内容
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-2-23  14:2:3
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int mail_post(MAILBOX Mailbox, MAIL Message)
{
    /*
     * 情景: 函数在中断中会不会出问题?
     */
    struct mailbox_t    *pMailbox;                                          /* 信箱指针                             */
    THREAD              *Thread;                                            /* 线程控制块                           */    
    INT32U               Priority;                                          /* 优先级                               */
    INT32U               Index;                                             /* 写入索引                             */
    int                  Key;


    /*
     * 1) 向队列写入邮件(满递增的方式)
     */
    pMailbox = (struct mailbox_t *)Mailbox;
    Index = pMailbox->Write;
    if (Index == pMailbox->Size)
    {
        Index = 0;
    }
    else
    {
        Index++;
    }
    
    if (Index == pMailbox->Read)                                            /* 如果队列满                           */
    {
        return ERR;
    }
    pMailbox->Write       = Index;
    pMailbox->Mail[Index] = Message;

    if (list_empty(&pMailbox->WaitHead))
    {
        goto exit;
    }
    
    /* 
     * 2) 线程就绪
     *    将线程控制表插入到指定线程队列.
     */
    Thread = container_of(&pMailbox->WaitHead.next->next, THREAD, Node);
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
    return OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

