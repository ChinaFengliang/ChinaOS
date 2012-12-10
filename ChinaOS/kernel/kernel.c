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
#include    <library/link/list.h>
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
INT32U                  OS_ActivePrio;                                      /* ��������ȼ�λ��־                   */
struct list_head        OS_ActiveProc[33];                                  /* ������̶߳���                       */
THREAD                 *OS_This;                                            /* ��ǰ���е��߳�                       */
THREAD                 *OS_LiveList;                                        /* �����߳�����                         */

struct list_head        OS_DeadList;                                        /* ��ʬ�߳�����                         */
static THREAD           OS_Idle;                                            /* �����߳̿��Ʊ�                       */
static REGISTER         OS_aIdleStack[THREAD_IDLE_STACK_SIZE];              /* �����߳�ջ                           */

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
** Function name:           thread_wakeup
** Descriptions:            �߳�
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
        OS_ActivePrio &= ~(1ul << Priority);                                /* ���������־                         */
    }
}

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

    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {   
        OS_ActivePrio &= ~(1ul << Priority);
    }

    /*
     * ɾ���߳���Դ
     * 1) �̶߳�ʱ��;
     */
    OS_timer_del(&pThread->Timer);
    
    /*
     * ���뵽��ʬ�߳�����,�ȴ�ϵͳ�����߳���Դ.
     */
    list_add(&pThread->Node, &OS_DeadList);
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

    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {
        OS_ActivePrio &= ~(1ul << Priority);                                /* ���������־                         */
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
            /* ���ñ�ɱ��ص����� */
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
    OS_This  = container_of(&OS_ActiveProc[Priority].next->next, THREAD, Node);

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
    list_add(&pControl->Node, &OS_ActiveProc[Priority]);
    OS_ActivePrio |= 1ul << Priority;
    
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
        struct list_head        *pNode;
        /*
         * 2) �ͷ��߳̿ռ�
         */
        list_for_each(pNode, &OS_DeadList)
        {
            THREAD *pThread = container_of(&pNode->next, THREAD, Node);

            list_del(&pThread->Node);
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
    THREAD     *pThread;                                                    /* �߳̿��ƿ�                           */
    INT32U      Priority;                                                   /* ���ȼ�                               */
    int         Key;


    if (0 == Time)
    {
        return OK;
    }


   /* 
    * 1) ����ǰ�̹߳���,ɾ�����п��ƵǼǱ�.
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
    * 2) �л��߳�
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
    int                  i;
    
    
    /*
     * 1) ��λ������Ϣ
     */
    OS_ActivePrio = 0;
    INIT_LIST_HEAD(&OS_DeadList);
    for (i = 0; i < 33; i++)
    {
        INIT_LIST_HEAD(&OS_ActiveProc[i]);
    }
    
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

    list_add(&OS_Idle.Node, &OS_ActiveProc[32]);
    OS_This = &OS_Idle;

    thread_register(OS_This);
    
    OS_activate_system();                                                   /* idle�߳̽���ϵͳ����                 */
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

