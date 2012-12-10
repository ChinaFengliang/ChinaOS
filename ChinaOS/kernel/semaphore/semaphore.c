/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            semaphore.c
** Last version:         V1.00
** Descriptions:         ��Ϣ��ͬ�������ļ�.
** Hardware platform:    
** SoftWare platform:
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011��2��25��  14:6:28
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
#include    <kernel/semaphore/semaphore.h>
#include    <library/malloc/malloc.h>

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ����� ----------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/


/*********************************************************************************************************************
** Function name:           semaphore_new
** Descriptions:            �½�һ����̬�ļ����ź���
** Input parameters:        Amount : �����ź�����ʼֵ
** Output parameters:       
** Returned value:          ==NULL : ����ʧ��
**                          !=NULL : �����ɹ�(������Ϣ����)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-2-25  14:9:48
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
SEMAPHORE semaphore_new(INT32U Amount)
{
    SEMAPHORE   Semaphore = NULL;

    
    if (NULL != (Semaphore = (SEMAPHORE)malloc(sizeof(struct semaphore))))
    {
        Semaphore->Counter = Amount;
        INIT_LIST_HEAD(&Semaphore->WaitHead);
    }
    
    return Semaphore;
}

/*********************************************************************************************************************
** Function name:           semaphore_free
** Descriptions:            �ͷ�һ����̬�ź���
** Input parameters:        Semaphore : �ź���
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
int semaphore_free(SEMAPHORE Semaphore)
{
    free(Semaphore);
    return OK;
}

/*********************************************************************************************************************
** Function name:           semaphore_setup
** Descriptions:            ������̬�ź���
** Input parameters:        Semaphore : �ź�����ַ
**                          Amount    : ��ʼ����
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-2-25  14:9:48
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int semaphore_create(SEMAPHORE Semaphore, INT32U Amount)
{
    if (NULL == Semaphore)
    {
        return ERR_BAD_PARAM;
    }

    Semaphore->Counter = Amount;
    INIT_LIST_HEAD(&Semaphore->WaitHead);
    
    return OK;
}

/*********************************************************************************************************************
** Function name:           mail_fetch
** Descriptions:            ��ȡ�ź���(��������ʽ). 
** Input parameters:        Semaphore : �ź�����ַ
** Output parameters:       
** Returned value:          ==OK : �����ɹ����ź�������,��ȡ�ź����ɹ�.
**                          !=OK : ����ʧ��. �ź���Ϊ��,��ȡ�ź���ʧ��.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-10  18:20:52
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int semaphore_fetch(SEMAPHORE Semaphore)
{
    int              Key;

    
    if (NULL == Semaphore)
    {
        return ERR_BAD_PARAM;
    }

    Key = atom_operate_lock();
    if (Semaphore->Counter)
    {
        Semaphore->Counter--;
        atom_operate_unlock(Key);
        return OK;
    }
    atom_operate_unlock(Key);
    
    return ERR;
}

/*********************************************************************************************************************
** Function name:           semaphore_wait
** Descriptions:            �ȴ��ź���(������ʽ)
** Input parameters:        Semaphore : �ź���
**                          Timeout   : ��ʱʱ��(��λ: ����)
**                                       ==0: ���õȴ�.
**                                       !=0: �ȴ�ʱ��.
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-2-25  14:20:38
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:            
*********************************************************************************************************************/
int semaphore_wait(SEMAPHORE Semaphore, INT32U Timeout)
{
    /*
     * ��: ��ν������̵߳ȴ�һ���ź���?
     * ��: �����߳̿��ƿ���˫����ṹ�����еȴ��߳���������.
     */
    THREAD          *pThread;
    INT32U           Priority;                                              /* ���ȼ�                               */
    int              Key;
    
    
    /* 0)
     * ��������
     */
    if (NULL == Semaphore)
    {
        return ERR_BAD_PARAM;
    }

    Key = atom_operate_lock();
    if (Semaphore->Counter)
    {
        Semaphore->Counter--;
        atom_operate_unlock(Key);
        return OK;
    }

    /*  
     * 1) ����ǰ�̹߳���
     */
    pThread = OS_This;
    Priority = pThread->Priority;
    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {   
        OS_ActivePrio &= ~(1ul << Priority);
    }

    /* 2)
     * ��ӻ�������.
     * 2.1) �ź��������¼�;
     * 2.2) �ȴ���ʱ�¼�.
     */
     
    /* 2.1)
     * �����ź����ȴ�����
     */
    list_add(&pThread->Node, &Semaphore->WaitHead);

    /* 2.2) 
     * ��ӻ��Ѷ�ʱ��.
     */
    if (Timeout)
    {
        OS_timer_add(&pThread->Timer, Timeout);                             /* ��ӻ��Ѷ�ʱ��                       */
    }
    atom_operate_unlock(Key);


    /* 3)
     * �л��߳�.
     */
    OS_thread_switch();


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
     * 4.2) �˳��ź����ȴ�����.
     */    
    list_del(&pThread->Node);
    
    /*
     * 5) ��ȡ�ź���
     */   
    if (0 == Semaphore->Counter)
    {
        atom_operate_unlock(Key);
        return ERR_TIMEOUT; 
    }
    Semaphore->Counter--;    
    atom_operate_unlock(Key);

    return OK;    
}

/*********************************************************************************************************************
** Function name:           semaphore_post
** Descriptions:            Ͷ���ź���
** Input parameters:        Semaphore : �ź�����ַ
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-2-25  14:22:16
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int semaphore_post(SEMAPHORE Semaphore)
{
    THREAD          *pThread;
    INT32S           Priority;
    int              Key;
    
    
    /* 0)
     * ��������
     */
    if (NULL == Semaphore)
    {
        return ERR_BAD_PARAM;
    }

    /*
     * 1) �����ź���
     */
    Key = atom_operate_lock();
    Semaphore->Counter++;
    if (list_empty(&Semaphore->WaitHead))
    {
        atom_operate_unlock(Key);
        return OK;
    }

    /* 
     * 2) �߳̾���
     *    ���߳̿��Ʊ���뵽ָ���̶߳���
     */
    pThread = container_of(&Semaphore->WaitHead.next->next, THREAD, Node);
    Priority = pThread->Priority;
    list_add(&pThread->Node, &OS_ActiveProc[Priority]);
    OS_ActivePrio |= 1ul << Priority;
    atom_operate_unlock(Key);

    /* 
     * 3) �л��߳�
     */
    scheduler();                                                            /* �̵߳��Ⱦ���                         */    
    
    
    return OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

