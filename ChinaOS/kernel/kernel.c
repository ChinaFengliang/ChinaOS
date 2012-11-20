/******************************************************Copyright (c)**************************************************
**                                              ��������ֹһ�п��ܵĸ���Ե��
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------+
** File name:            Kernel.c
** Last version:         V1.00
** Descriptions:         ChinaOS����ϵͳ�����ļ�.
** Hardware platform:    ARM7��ARM9��Cortex-M0��Cortex-M3
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2010��8��25��  14:27:47
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
#define DEBUG_LOCAL_EN                              0                       /* ���ص��Կ���(Ĭ������): 0:��; 1:��   */
/*********************************************************************************************************************
                                                    ͷ�ļ���
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
#include    <device.h>

/*********************************************************************************************************************
                                                   �궨����
*********************************************************************************************************************/
/* ϵͳ������� ----------------------------------------------------------------------------------------------------*/
#define     THREAD_MAIN_PRIORITY                    1                       /* ���߳����ȼ�(��Χ:[0, 31])           */
#define     THREAD_MAIN_STACK_SIZE                  2040                    /* ���̳߳�ջ���(��λ:byte)            */
#define     THREAD_IDLE_STACK_SIZE                  256                     /* �����߳�ջ���(��λ:byte)            */

/*********************************************************************************************************************
                                                   ���Ͷ�����
*********************************************************************************************************************/
/* �����߳̿��Ʊ����� ----------------------------------------------------------------------------------------------*/
struct __idle_pcl
{
    REGISTER           *pStackTop;                                          /* �߳�ջ��ָ��(Ҫ��: �ṹ���һԪ��)   */
    REGISTER           *pStackBottom;                                       /* �߳�ջ��ָ��                         */
    INT32U              Priority;                                           /* �߳�����Ȩ                           */
    char               *pName;                                              /* �߳�����                             */
};
typedef     struct __idle_pcl                       IDLE_PCL;               /* �����߳̿��Ʊ�����                   */

/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/
INT32U      OS_ActivePrio;                                                  /* ��������ȼ�λ��־                   */
THREAD     *OS_ActiveProc[33];                                              /* ������̶߳���                       */
THREAD     *OS_This;                                                        /* ��ǰ���е��߳�                       */
THREAD     *OS_LiveList;                                                    /* �����߳�����                         */

static      THREAD      *OS_DeadList;                                       /* ��ʬ�߳�����                         */
static      IDLE_PCL     OS_Idle;                                           /* �����߳̿��Ʊ�                       */
static      REGISTER     OS_aIdleStack[THREAD_IDLE_STACK_SIZE];             /* �����߳�ջ                           */

/*********************************************************************************************************************
** Function name:           main
** Descriptions:            ���߳�
** Input parameters:
** Output parameters:
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
** Descriptions:            ��װ�ڴ������
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
** Descriptions:            ��ʼ���߳�ջ
** Input parameters:        pProcess     : �̺߳�����
**                          pOption      : ѡ��
**                          pStackBottom : ջ��
** Output parameters:       
** Returned value:          ��ǰջ��ָ��
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
** Function name:           thread_register
** Descriptions:            ע���߳�
** Input parameters:        pThread : �߳̿��Ʊ�
** Output parameters:       OS_LiveList : �߳�ע������
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
     * �������߳�ע�ᵽ���������β��
     */
    ppNode = &OS_LiveList;
    while (NULL != *ppNode)
    {
        ppNode = &((*ppNode)->pLive);
    }
    
    *ppNode = pThread;                                                      /* ���ӵ�β��                           */
}

/*********************************************************************************************************************
** Function name:           thread_unregister
** Descriptions:            ע���߳�
** Input parameters:        pThread : �߳̿��Ʊ�
** Output parameters:       OS_LiveList : �߳�ע������
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
     * ���̴߳Ӽ���������ɾ��
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
** Descriptions:            �̳߳���
** Input parameters:        Info     : �̷߳���ֵ
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
    THREAD          *pThread;                                               /* �߳̿��ƿ�                           */
    INT32U           Priority;
    int              Key;


    pThread = OS_This;
    if (NULL != pThread->pExit)
    {
        pThread->pExit(Info);
    }

    /* 
     * ɾ�����п��ƵǼǱ�
     */
    Priority = pThread->Priority;
    Key = atom_operate_lock();    
    thread_unregister(pThread);
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
     * ɾ���߳���Դ
     * 1) �̶߳�ʱ��;
     */
    OS_timer_del(&pThread->Timer);
    
    /*
     * ���뵽��ʬ�߳�����,�ȴ�ϵͳ�����߳���Դ.
     */
    if (NULL == OS_DeadList)
    {
        pThread->pNext = pThread;
        pThread->pPrev = pThread;
        OS_DeadList    = pThread;
    }
    else
    {   /* ���ڶ��еĺ��� */
        pThread->pPrev = OS_DeadList->pPrev;
        pThread->pNext = OS_DeadList;
        OS_DeadList->pPrev->pNext = pThread;
        OS_DeadList->pPrev = pThread;
    }
    
    atom_operate_unlock(Key);

   /* 
    * �л��������߳�
    */
    OS_thread_switch();
}

/*********************************************************************************************************************
** Function name:           kill
** Descriptions:            ɱ�������߳�(�����߳���)
** Input parameters:        Thread : �߳�ID��
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
     * �������Ƿ�ֻ��ɱ�������߳�, ����ɱ�����߳�.
     * ��Ҫɱ�����߳�,�����exit����.
     */
    THREAD     *pThread;                                                    /* �߳̿��ƿ�                           */
    INT32U      Priority;                                                   /* ���ȼ�                               */
    int         Key;


    /*
     * ��ȡָ���߳̿��Ʊ�
     */
    if (NULL == Thread)
    {
        return ERR_BAD_PARAM;
    }
    pThread = (THREAD*)Thread;

    /*
     * ִ���߳���ֹ�ص�����
     */
    if (NULL != pThread->pKill)
    {
        pThread->pKill();
    }

    /*
     * ע���߳�
     */
    Key = atom_operate_lock();
    thread_unregister(pThread);
    Priority = pThread->Priority;
    if (pThread == pThread->pNext)
    {
        OS_ActiveProc[Priority] = NULL;
        OS_ActivePrio &= ~(1ul << Priority);                                /* ���������־                         */
    }
    else
    {
        pThread->pPrev->pNext   = pThread->pNext;
        pThread->pNext->pPrev   = pThread->pPrev;
        OS_ActiveProc[Priority] = pThread->pNext;                           /* �����߳���ָ��                       */
    }
    OS_timer_del(&pThread->Timer);                                          /* ɾ���̶߳�ʱ��                       */
    atom_operate_unlock(Key);
    
    free(pThread);

    return OK;
}

/*********************************************************************************************************************
** Function name:           signal
** Descriptions:            ע���ź��¼�
** Input parameters:        SigNum     : �¼��ź�
**                          pFunction  : ������
** Output parameters:       
** Returned value:          ==OK    : �����ɹ�
**                          !=OK    : ����ʧ��(����������Ϣ)
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
            /* ���õ�ǰ�̵߳ķ��ػص����� */
            This->pExit = (void(*)(INT32S))pFunction;
            break;

        case SIG_KILL :
            /* */
            This->pKill = (void(*)(void))pFunction;
            break;

        default :
            return ERR;
    }

    return OK;
}


/*********************************************************************************************************************
** Function name:           shift_thread
** Descriptions:            �߳̽���
** Input parameters:        pStackTop : ��ǰ�̵߳�ջ��ָ��
** Output parameters:
** Returned value:          ��һ�������̵߳�ջ��ָ��
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

    OS_This->pStackTop = pStackTop;                                         /* ���浱ǰ�̵߳�ջ��ָ��               */

    Priority = bit_scan_forward(OS_ActivePrio);
    OS_This  = OS_ActiveProc[Priority];

    return OS_This->pStackTop;
}

/*********************************************************************************************************************
** Function name:           scheduler
** Descriptions:            ���Ⱦ���
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
     * ��ǰ�߳����ȼ���ֵ�Ǿ���������Сֵ, ����ǰ�߳����ȼ����.
     */
    if (bit_scan_forward(OS_ActivePrio) < OS_This->Priority)
    {
        OS_thread_switch();
    }
}

/*********************************************************************************************************************
** Function name:           labour
** Descriptions:            �����߳�
** Input parameters:        pName       : �߳�����
**                          pProcess    : �̺߳�����
**                          pOption     : ����
**                          StackDepth  : ջ���
**                          Priority    : ����Ȩ[0, 31]
** Output parameters:
** Returned value:          !=NULL      : �߳�ID
**                          ==NULL      : ����ʧ��(����������Ϣ)
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
    INT8U       *pStackBottom;                                              /* ջ�׵�ַ                             */
    THREAD      *pControl;                                                  /* �߳̿��Ʊ�                           */


    /* 
     * 0) ��������
     */
    if ((Priority & ~0x1F) ||
        (NULL == pProcess) ||
        (0 == StackDepth))
    {
        DBG_ERR("�����������");
        return NULL;
    }
    StackDepth = align_upside(StackDepth, sizeof(REGISTER));
    
    /*
     * 1) ��ʼ��ջ�ռ�
     */
    pStackBottom = (INT8U*)malloc(StackDepth + sizeof(THREAD));             /* ջ�ռ� + ���ƿ�ռ�                  */
    if (NULL == pStackBottom)
    {
        DBG_WARN("�ڴ治��\r\n");
        return NULL;
    }
    
    pControl = (THREAD*)pStackBottom;
    memset(pControl, NULL, StackDepth + sizeof(THREAD));                    /* ������ƿ�����                       */
    pStackBottom  += sizeof(THREAD);                                        /* 4�ֽڶ�������                        */
    
#   if OS_STACK_GROWTH == HIGH_TO_LOW
    pStackBottom += StackDepth - sizeof(REGISTER);
#   endif

    pControl->pStackBottom = (REGISTER*)pStackBottom;
    pControl->pStackTop    = stack_init(pProcess, pOption, pStackBottom);   /* ����ջ��ָ��                         */
    pControl->Priority     = Priority;
    pControl->pName        = pName;
    thread_register(pControl);
    
    /*
     * 2) ע���߳�
     * ���߳̿��Ʊ���뵽ָ���̶߳���
     */
    if (NULL == OS_ActiveProc[Priority])
    {   /* ��0���ڵ����ӵ�1���ڵ� */
        pControl->pNext = pControl;
        pControl->pPrev = pControl;
        OS_ActiveProc[Priority] = pControl;
    }
    else
    {   /* ���ڶ��еĺ��� */
        pControl->pPrev = OS_ActiveProc[Priority]->pPrev;
        pControl->pNext = OS_ActiveProc[Priority];
        OS_ActiveProc[Priority]->pPrev->pNext = pControl;
        OS_ActiveProc[Priority]->pPrev = pControl;
    }

    OS_ActivePrio |= 1ul << Priority;                                       /* �������ȼ�������־                   */
    
    /*
     * 3) ���Ⱦ���
     * ��������̵߳����ȼ����ڵ�ǰ�߳����ȼ�,�򷵻�����ǰ�߳�,����������������߳�.
     */
    if (Priority < OS_This->Priority)
    {
        OS_thread_switch();                                                 /* �����л��龰                         */
    }

    return (INT32S)pControl;
}

/*********************************************************************************************************************
** Function name:           getid
** Descriptions:            ��õ�ǰ�߳�ID
** Input parameters:
** Output parameters:
** Returned value:          �̱߳�ʶ��
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
** Descriptions:            �߳�����
** Input parameters:        pName : �߳�����
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
** Descriptions:            �ı��߳����ȼ�
** Input parameters:        id       : �߳�ID
**                          Priority : ���ȼ�
** Output parameters:
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
** Descriptions:            �����߳�
** Input parameters:        option  : (����)
** Output parameters:       none
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
    /* ����
     * 1) ϵͳ��ʱ�����ڴ�����;
     * 2) ϵͳ����ʱ����shell����
     */
    
    /* 
     * 1) ��װӲ/�����.
     *    Ϊ���豸��װʱ���������ڴ�,������ִ��mallocor��ʼ��,��ִ���豸��װ.
     */
    if (OK != mallocor_setup())
    {
        DBG_ERR("�ڴ��������װʧ��\r\n");
    }
    if (OK != console_setup())
    {
        DBG_ERR("����̨��װʧ��\r\n");
    }
    systicker_setup();
    device_setup();
    
    labour("main", main, NULL, 2040, THREAD_MAIN_PRIORITY);

    while (1)
    {
        /*
         * 2) �ͷ��߳̿ռ�
         */
        if (NULL != OS_DeadList)
        {
            THREAD *pThread = OS_DeadList;

            /* 3) �������н�� */
            if (pThread == pThread->pNext)
            {
                OS_DeadList = NULL;
            }
            else
            {
                pThread->pPrev->pNext = pThread->pNext;
                pThread->pNext->pPrev = pThread->pPrev;
                OS_DeadList = pThread->pNext;
            }
            /* 4) �ͷ��߳̿ռ� */
            free(pThread);
        }
        
        /*
         * 5) �ṩ����̨����
         */
        command();
    }
}

/*********************************************************************************************************************
** Function name:           sleep
** Descriptions:            ˯��
** input parameters:        Time: ��ʱʱ��(��λ: ����)
** output parameters:       
** Returned value:          == OK: ��������
**                          != OK: �쳣����
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
    INT32U      Priority;                                                   /* ���ȼ�                               */
    int         Key;


    if (0 == Time)
    {
        return OK;
    }


   /* 
    * 1) ����ǰ�̹߳���,ɾ�����п��ƵǼǱ�.
    */
    Key = atom_operate_lock();
    Priority = OS_This->Priority;
    if (OS_This == OS_This->pNext)
    {   /* Ψһ�ڵ� */
        OS_ActiveProc[Priority] = NULL;
        OS_ActivePrio &= ~(1ul << Priority);                                /* ���������־                         */
    }
    else
    {   /* ��Ψһ�ڵ� */
        OS_This->pPrev->pNext   = OS_This->pNext;
        OS_This->pNext->pPrev   = OS_This->pPrev;
        OS_ActiveProc[Priority] = OS_This->pNext;                           /* �����߳���ָ��                       */
    }
    
   /* 
    * 2) ��ӻ��Ѷ�ʱ��
    */
    OS_timer_add(&OS_This->Timer, Time);
    atom_operate_unlock(Key);   
   
   /* 
    * 3) �л��߳�
    */
    OS_thread_switch();                                                     /* �л��������߳�                       */

    OS_timer_del(&OS_This->Timer);                                          
   
    return OK;
}

/*********************************************************************************************************************
** Function name:           wakeup
** Descriptions:            ����
** Input parameters:        Thread      : �߳�ID
** Output parameters:
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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
     * ������ָ���߳�ʱ, ���س�����Ϣ;
     */

    return ERR;
}

#if 0
/*********************************************************************************************************************
** Function name:           check_need_to_sched
** Descriptions:            ����Ƿ���Ҫ�л��߳�
** Input parameters:
** Output parameters:
** Returned value:          ==0 : ����Ҫ�л�
**                          ==1 : ��Ҫ�л�
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-1-4  15:37:14
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
BOOL check_need_to_sched(void)
{
    /*
     * ������ص�ǰ�߳�����Ҫ�л�,��������л�.
     *
     * ��: �����ж��жϷ�����Ƿ�Ӧ���л�?
     * ��: OS_This == �����㷨��ѡ���߳̿��ƿ�.
     */


    INT32S          Priority;
    THREAD         *pThread;

    Priority = bit_scan_forward(OS_ActivePrio);
    pThread = OS_ActiveProc[Priority];
    
    return (pThread != OS_This);
}
#endif

/*********************************************************************************************************************
** Function name:           startup
** Descriptions:            ����ChinaOS����ϵͳ
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

    
    /*
     * 1) ��λ������Ϣ
     */
    OS_ActivePrio = 0;                                                      /* ����¼�������                       */
    OS_DeadList   = NULL;
    memset(OS_ActiveProc, NULL, sizeof(OS_ActiveProc));                     /* ������ƿ�����                       */
    
    /*
     * 2) ���������߳�
     */
    pStackBottom  = OS_aIdleStack;
#if OS_STACK_GROWTH == HIGH_TO_LOW
    pStackBottom += THREAD_IDLE_STACK_SIZE - 1;                             /* ������ջ��                           */
#endif
    OS_Idle.pStackBottom = pStackBottom;
    OS_Idle.pStackTop = stack_init(idle, NULL, pStackBottom);
    OS_Idle.Priority  = 32;
    OS_Idle.pName     = "idle";
    OS_This = OS_ActiveProc[32] = (THREAD *)&OS_Idle;

    thread_register(OS_This);
    
    OS_activate_system();                                                   /* idle�߳̽���ϵͳ����                 */
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

