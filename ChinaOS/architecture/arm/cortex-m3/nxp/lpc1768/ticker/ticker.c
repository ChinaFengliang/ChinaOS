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
** Descriptions:         ϵͳʱ��.
** Hardware platform:    lpc24xx
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011��3��6��  15:58:52
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
#include    <string.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <include/debug.h>
#include    <kernel/kernel.h>
#include    <library/bit/bit.h>
#include    <library/ioport.h>
#include    "./ticker.h"
#include    "../../lpc17xx.h"
#include    <syscall.h>
#include    <nvic/interrupt.h>

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ����� ----------------------------------------------------------------------------------------------------------*/
#define TimeCounter               (*(volatile INT32U *)0xE0004008)          /* ��ʱ��������                         */

/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/
static TIMER        *OS_pTimer;                                             /* ��ǰ��ʱ��                           */
extern INT32U        OS_ActivePrio;                                         /* ��������ȼ���־                     */
extern THREAD       *OS_ActiveProc[33];                                     /* ������̶߳���                       */
extern THREAD       *OS_This;                                               /* ��ǰ�߳�                             */


/*********************************************************************************************************************
** Function name:           get_left_time
** Descriptions:            ��õ�ǰ����ʣ��ʱ��
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
    return read_dword(T0MR0) - TimeCounter;                                 /* ����������ģʽ                       */
}

/*********************************************************************************************************************
** Function name:           set_time
** Descriptions:            ���ñ���ʱ��
** Input parameters:        Time: ����ʱ��
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
INLINE void set_time(INT32U Time)
{
    write_dword(T0TCR, 0x01 << 1);                                         /* ��λ��ʱ��                           */    
    write_dword(T0MR0, Time);                                              /* ������һ�α���ֵ                     */ 
    write_dword(T0TCR, 0x01);                                              /* ������ʱ��T1                         */
}

/*********************************************************************************************************************
** Function name:           OS_timer_add
** Descriptions:            ���ñ�����ʱ��
** Input parameters:        pTimer   : ��ʱ��
**                          Millisecond : ��ʱʱ��(��λ: ����)
** Output parameters:
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-26  15:42:17
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
STATUS OS_timer_add(TIMER * const pTimer, INT32U Millisecond)
{
    /* �����������ǵ��������:
     * �Զ�ʱ�������������޸�ʱ����Ҫʹ�ù��жϵķ�ʽ�ϸ�����Ա��ֶ�����������.
     * �Զ�ʱ����������һֱ��ǰ�Լӣ����Բ�ȡ�ݴ�ķ�ʽ���б���.
     * ���һ: ��ӽڵ���Ϣ, ������Ӳ������.
     * �����: ��ӽڵ���Ϣ, ������Ӳ������.
     * ʵ�ֻ���:
     * 1) �Ի���˫������ṹ����;
     * 2) ��ǰ�÷�ʽ���ڵ���Ϣ���뵽����;
     */
    TIMER       *pTmp = OS_pTimer;
    INT32U       LeftTime;                                                  /* ʣ��ʱ��                             */
    INT32U       Time;                                                      /* ��ʱʱ��                             */


   /*
    * ��Ŀ�궨ʱ������������� 
    */
    if (NULL == pTmp)                                                       /* ���Ψһ�Ķ�ʱ��                     */
    {
        pTimer->Time = Millisecond;
        pTimer->Next = pTimer;                                              /* ָ������                             */
        pTimer->Prev = pTimer;
        goto replace;
    }

    LeftTime = get_left_time();
    Time = pTmp->Time + Millisecond - LeftTime;
    pTimer->Time = Time;
    
    while (OS_pTimer != pTmp->Next)                                         /* ���Ҷ�λ                             */
    {
        INT32U       TimeFormer;
        INT32U       TimeLatter;
        
        TimeFormer = pTmp->Time;
        TimeLatter = pTmp->Next->Time;

        if ((Time - TimeFormer) <= (TimeLatter - TimeFormer))               /* �ж�ָ����ʱ���Ƿ�������֮��         */
        {
            break;
        }

        pTmp = pTmp->Next;
    }

    /* 
     * ������pTmp�ڵ��
     */
    pTimer->Next     = pTmp->Next;                                          
    pTimer->Prev     = pTmp;
    pTmp->Next->Prev = pTimer;
    pTmp->Next       = pTimer;

    
    if (LeftTime <= Millisecond)                                            /* ��Ŀ�궨ʱ���Ķ�ʱֵ�����ʱ��       */
    {
        return OK;;
    }

   /*
    * �л���ǰ��ʱ�� 
    */    
replace:
    OS_pTimer = pTimer;                                                     /* ��Ϊ��ǰ������                       */
    set_time(Millisecond);

    return OK;
}

/*********************************************************************************************************************
** Function name:           OS_timer_del
** Descriptions:            ���������ʱ��
** Input parameters:        pTimer: ��ʱ��
** Output parameters:
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
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
        return ERR;
    }

    if (pTimer == OS_pTimer)
    {
        /* ָ����ʱ�� == ��ǰ��ʱ��ʱ */
        if (pTimer == pTimer->Next)
        {   /* ��ǰ��ʱ��ΪΨһ��ʱ��ʱ */
            write_dword(T0TCR, 0x00);                                       /* �ض�ʱ���ж�                         */
            write_dword(T0TCR, 0x01 << 1);                                  /* ��λ��ʱ��                           */
            OS_pTimer = NULL;                                               /* ɾ��Ψһ�Ķ�ʱ��                     */
        }
        else
        {   /* ��ǰ��ʱ����ΪΨһ��ʱ��ʱ */
            INT32U       LeftTime;                                          /* ʣ��ʱ��                             */

            LeftTime  = get_left_time();
            OS_pTimer = pTimer->Next;
            set_time(OS_pTimer->Time - pTimer->Time + LeftTime);
        }
    }

    /* ָ����ʱ�� != ��ǰ��ʱ��ʱ */
    pTimer->Prev->Next = pTimer->Next;                                      /* ɾ��ָ����ʱ�������б�               */
    pTimer->Next->Prev = pTimer->Prev;
    memset(pTimer, NULL, sizeof(TIMER));

    return OK;
}

/*********************************************************************************************************************
** Function name:           systimer_exception
** Descriptions:            �ṩ��ʱ����,���ѵ��ڵ��̻߳���.
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
    INT32U          Time;                                                   /* ��ǰ��ʱֵ                           */
    INT32U          Priority;                                               /* �߳����ȼ�                           */
    TIMER          *pTimer;                                                 /* ��ʱ��ʱ��                           */
    

    /*
     * 1) ����жϱ�־.
     */
    write_dword(T0IR, 0xFF);                                                /* �����ʱ��0�����жϱ�־              */
    OS_thread_switch();                                                     /* �����߳��л�                         */

    /*
     * 2) �ݴ���.
     * ��ֹ��ɾ����ʱ���Ĺ�����(�ٽ����), ��ʱ���Ѳ����ж�.
     */
    pTimer = OS_pTimer;
    if (NULL == pTimer)
    {
        goto exit;
    }
    Time = pTimer->Time;
    
    do
    {
        THREAD         *pThread;                                            /* ��ʱ�������߳�                       */
        
        /*
         * 3) ����ʱ�������߳̾���.
         */
        pThread = (THREAD *)(((INT32U)pTimer) - (INT32U)&((THREAD*)0)->Timer);
        Priority = pThread->Priority;

        if (NULL == OS_ActiveProc[Priority])
        {   /* ��0���ڵ����ӵ�1���ڵ� */
            pThread->pNext = pThread;
            pThread->pPrev = pThread;
            OS_ActiveProc[Priority] = pThread;
            OS_ActivePrio |= 1ul << Priority;                               /* �������ȼ�������־                   */
        }
        else
        {   /* ���ڶ��еĺ��� */
            pThread->pPrev = OS_ActiveProc[Priority]->pPrev;
            pThread->pNext = OS_ActiveProc[Priority];
            OS_ActiveProc[Priority]->pPrev->pNext = pThread;
            OS_ActiveProc[Priority]->pPrev = pThread;
        }

        /*
         * 4) �л�����һ����ʱ��.
         */
        pTimer = pTimer->Next;                                              /* �л�����һ����ʱ��                   */
        
        if (Time != pTimer->Time)
        {
            set_time(pTimer->Time - pTimer->Prev->Time);
            OS_pTimer = pTimer;
            return;
        }

    } while (pTimer != OS_pTimer);                                          /* �������ζ�ʱ������                   */

    OS_pTimer = NULL;                                                       /* ָ�������                           */
    
exit:    
    write_dword(T0TCR, 0x00);                                              /* �ض�ʱ���ж�                         */
}

/*********************************************************************************************************************
** Function name:           systicker_setup
** Descriptions:            ��װϵͳ��ʱ��
** Input parameters:
** Output parameters:
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-26  15:46:29
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
int systicker_setup(void)
{
    /* Ҫ��:
     * 1��ʹ��ʱ������������1�����ʱ���źŽ��м���;
     * 2����ʱ���Ķ�ʱֵƥ��ʱ,�����ж�,��λ������,���ܼ�����;
     */
    dword_set_bits(PCONP, 1ul<<1);                                          /* ʹ�ܶ�ʱ��0                          */
    write_dword(T0TCR, 0x00);                                               /* ��ͣ��ʱ������                       */
    write_dword(T0IR, 0xFF);                                                /* �����ʱ��0�����жϱ�־              */
    write_dword(T0PR, 12499);                                               /* ��Ƶϵ��: 12499+1=12500��Ƶ          */
    write_dword(T0CTCR, 0x00);                                              /* ģʽѡ��: ��ʱ��ģʽ                 */
    write_dword(T0TCR, 0x01 << 1);                                          /* ��λ��ʱ��                           */
    write_dword(T0MCR, 0x07);                                               /* ��ʱ��ƥ��ʱ(�ж�&��λ&ֹͣ)         */
 
    OS_pTimer = NULL;
    irq_register(DEVICE_TIMER0, 15, systimer_exception);                    /* ע�ᶨʱ��0�жϷ�����              */
    
    return OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

