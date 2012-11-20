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
** Descriptions:         �ʼ�ͨ�Ż���ʵ���ļ�.
** Hardware platform:    
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011��2��23��  13:59:27
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
                                                    ͷ�ļ���
*********************************************************************************************************************/
#include    <syscall.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <kernel/kernel.h>
#include    <kernel/mailbox/mailbox.h>
#include    <library/malloc/malloc.h>

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ����� ----------------------------------------------------------------------------------------------------------*/
#define     MAILBOX_EMPTY                   0ul                             /* �ʼ����                             */
#define     MAILBOX_FULL                    1ul                             /* �ʼ�����                             */
/* ����״̬ --------------------------------------------------------------------------------------------------------*/
#define     MAILBOX_VALID                   0ul                             /* ��Ч����                             */
#define     MAILBOX_TRASH                   1ul                             /* ��������                             */

/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/
extern INT32U        OS_InterruptNesting;                                   /* �ж�Ƕ�׼�����                       */
extern INT32U        OS_ActivePrio;                                         /* ��������ȼ���־                     */
extern THREAD       *OS_ActiveProc[33];                                     /* ������̶߳���                       */
extern THREAD       *OS_This;                                               /* ��ǰ���е��߳�                       */

/*********************************************************************************************************************
** Function name:           mailbox_new
** Descriptions:            �½���Ϣ����
** Input parameters:        Size : �����С
** Output parameters:       
** Returned value:          ==NULL  : ����ʧ��
**                          !=NULL  : �����ɹ�(������Ϣ����)
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
        pMailbox->pThread = NULL;        
    }
    
exit:
    return pMailbox;
}

/*********************************************************************************************************************
** Function name:           mailbox_free
** Descriptions:            �ͷ���Ϣ����
** Input parameters:        Mailbox : ��Ϣ����
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
     * ���� : ��A�߳̽���ȴ��������Ա��������Ϣʱ, B�߳��ͷű���Ϣ����(�����洢�ռ�). 
     *        �ɴ����mail_wait�������Ѻ���ʷǷ��ڴ�.
     * ���� : 1) mailbox_freeֻ������������ֹ��������;
     *        2) �����еȴ��̻߳���;
     *        2) �������߳��˳��ȴ���, ���ղ��ͷű���Ϣ����.
     */
    free(Mailbox);
    return OK;
}

/*********************************************************************************************************************
** Function name:           mailbox_create
** Descriptions:            �����ʼ���
** Input parameters:        Mailbox : ��Ϣ����
**                          Size    : �����С
** Output parameters:       
** Returned value:          ==OK  : �����ɹ�
**                          !=OK  : ����ʧ��(����������Ϣ)
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
    pMailbox->pThread = NULL;
    pMailbox->Status  = VALID;
    
    return OK;
}

/*********************************************************************************************************************
** Function name:           mail_fetch
** Descriptions:            ��������ʽ��ȡ�ʼ�. 
** Input parameters:        pMailbox : �ʼ����ָ��
** Output parameters:       pMessage : �ʼ�ָ��
** Returned value:          ==OK : �����ɹ�������ǿ�,��ȡ�ʼ��ɹ�.
**                          !=OK : ����ʧ��. ����Ϊ��,��ȡ�ʼ�ʧ��.
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
    struct mailbox_t    *pMailbox;                                          /* ����ָ��                             */
    int                  Key;

    /*
     * �Ӷ��ж����ʼ�(�������ķ�ʽ)
     */
    pMailbox = (struct mailbox_t *)Mailbox;
    Key = atom_operate_lock();
    if (pMailbox->Read != pMailbox->Write)                                  /* ���в�Ϊ��                           */
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
** Descriptions:            �ȴ��ʼ�
** Input parameters:        pMailbox : �ʼ����ָ��
**                          Timeout  : ��ʱʱ��(��λ: ����)
**                                      ==0: ���õȴ�.
**                                      !=0: �ȴ�ʱ��.
** Output parameters:       pMessage : �ʼ�ָ��
** Returned value:          == OK : �����ɹ�
**                          != OK : ����ʧ��(����������Ϣ)
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
    struct mailbox_t    *pMailbox;                                          /* ����ָ��                             */
    THREAD              *pThread;                                           /* �߳�                                 */    
    INT32U               Priority;                                          /* ���ȼ�                               */    
    int                  Key;


    if ((NULL == Mailbox) || (NULL == pMessage))                            /* ��������                             */
    {
        return ERR_BAD_PARAM;
    }

    /*
     * �Ӷ��ж����ʼ�(�������ķ�ʽ)
     */
    pMailbox = (struct mailbox_t *)Mailbox;
    Key = atom_operate_lock();
    if (pMailbox->Read != pMailbox->Write)                                  /* ���в�Ϊ��                           */
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
     * 1) ����ǰ�̹߳���
     */
    pThread = OS_This;
    Priority = pThread->Priority;
    if (pThread == pThread->pNext)
    {   /* Ψһ�ڵ� */
        OS_ActiveProc[Priority] = NULL;
        OS_ActivePrio &= ~(1ul << Priority);                                /* ���������־                         */
    }
    else
    {   /* ��Ψһ�ڵ� */
        pThread->pPrev->pNext   = pThread->pNext;
        pThread->pNext->pPrev   = pThread->pPrev;
        OS_ActiveProc[Priority] = pThread->pNext;                           /* �����߳���ָ��                       */
    }
 
    /* 
     * 2) ��ӻ�������
     */
    
    /* 
     * 2.1) �����ʼ��ȴ�����
     */
    if (NULL == pMailbox->pThread)
    {   /* ��0���ڵ����ӵ�1���ڵ� */
        pThread->pNext    = pThread;
        pThread->pPrev    = pThread;
        pMailbox->pThread = pThread;
    }
    else
    {   /* ���ڶ��еĺ��� */
        pThread->pPrev = pMailbox->pThread->pPrev;
        pThread->pNext = pMailbox->pThread;
        pMailbox->pThread->pPrev->pNext = pThread;
        pMailbox->pThread->pPrev = pThread;
    }
    
    /*
     * 2.2) ���ó�ʱ��������
     */
    if (Timeout)
    {
        OS_timer_add(&pThread->Timer, Timeout);                             /* ��ӻ��Ѷ�ʱ��                       */
    }
    atom_operate_unlock(Key);
    
    /* 
     * 3) �л��߳�
     */
    OS_thread_switch();                                                     /* �л��������߳�                       */

    /*
     * 4) ɾ����������
     */
    
    /*
     * 4.1) ɾ����ʱ��������
     */
    Key = atom_operate_lock();
    if (Timeout)
    {
        OS_timer_del(&pThread->Timer);                                      /* ɾ�����Ѷ�ʱ��                       */    
    }
    /*
     * 4.2) �˳��ʼ��ȴ�����
     */
    if (pThread == pThread->pNext)
    {
        /* ��ǰ�߳�ΪΨһ�ȴ��߳�ʱ */
        pMailbox->pThread = NULL;
    }
    else
    {
        /* ���ʼ����ڶ���ȴ��߳�ʱ */
        pThread->pPrev->pNext = pThread->pNext;
        pThread->pNext->pPrev = pThread->pPrev;
    }
    
    /*
     * 5) ��ȡ�ʼ�
     */    
    if (pMailbox->Read == pMailbox->Write)                                  /* ����Ϊ��                             */
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
** Descriptions:            �����ʼ�
** Input parameters:        pMailbox : �ʼ����ָ��
**                          Message  : �ʼ�����
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
     * �龰: �������ж��л᲻�������?
     */
    struct mailbox_t    *pMailbox;                                          /* ����ָ��                             */
    THREAD              *Thread;                                            /* �߳̿��ƿ�                           */    
    INT32U               Priority;                                          /* ���ȼ�                               */
    INT32U               Index;                                             /* д������                             */
    int                  Key;


    /*
     * 1) �����д���ʼ�(�������ķ�ʽ)
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
    
    if (Index == pMailbox->Read)                                            /* ���������                           */
    {
        return ERR;
    }
    pMailbox->Write       = Index;
    pMailbox->Mail[Index] = Message;

    if (NULL == (Thread = pMailbox->pThread))                               /* û���̵߳ȴ�                         */
    {
        goto exit;
    }
    
    /* 
     * 2) �߳̾���
     *    ���߳̿��Ʊ���뵽ָ���̶߳���.
     */    
    Priority = Thread->Priority;     
    Key = atom_operate_lock();
    if (NULL == OS_ActiveProc[Priority])
    {   /* ��0���ڵ����ӵ�1���ڵ� */
        Thread->pNext = Thread;
        Thread->pPrev = Thread;
        OS_ActiveProc[Priority] = Thread;
        OS_ActivePrio |= 1ul << Priority;                                   /* �������ȼ�������־                   */    
    }
    else
    {   /* ���ڶ��еĺ��� */
        Thread->pPrev = OS_ActiveProc[Priority]->pPrev;
        Thread->pNext = OS_ActiveProc[Priority];
        OS_ActiveProc[Priority]->pPrev->pNext = Thread;
        OS_ActiveProc[Priority]->pPrev = Thread;
    }
    atom_operate_unlock(Key);

    /* 
     * 3) �л��߳�
     */
    scheduler();

exit:
    return OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

