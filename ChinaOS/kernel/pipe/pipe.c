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
** Descriptions:         �����ܵ��ļ�.
** Hardware platform:    
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011��5��22��  0:34:19
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
#include    <syscall.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <kernel/kernel.h>
#include    <library/malloc/malloc.h>
#include    <library/math/math.h>
#include    "./pipe.h"

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ����� ----------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/
struct pipe_t
{    
    INT8U         *pBuffer;                                                 /* the buffer holding the data          */    
    INT32U         Lenth;                                                   /* �ܵ�����                             */
    INT32U         Width;                                                   /* �ܵ���� number of bytes per element */
    INT32U         In;                                                      /* ��������                             */    
    INT32U         Out;                                                     /* ������� data is extracted from off. */
    THREAD        *pReadThread;                                             /* �ܵ����ն��߳�                       */
};  

/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/


/*********************************************************************************************************************
** Function name:           pipe_new
** Descriptions:            �½��ܵ�
** Input parameters:        Width  : �ܵ����(Ԫ�ش�С)
**                          Length : �ܵ�����
** Output parameters:       
** Returned value:          ==NULL : ����ʧ��
**                          !=NULL : �����ɹ�(�����ܵ���Ϣ)
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
    struct pipe_t           *pPipe;                                         /* �ܵ�����ָ��                         */


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
** Descriptions:            �ͷŹܵ�
** Input parameters:        Pipe : �ܵ�����
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
** Descriptions:            ��ܵ�д������
** Input parameters:        Pipe      : �ܵ�����
**                          pElements : ���ݻ�����
**                          Length    : ����Ԫ�ظ���
** Output parameters:       
** Returned value:          ʵ��д�������Ԫ�ظ���
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-23  19:53:22
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:             ��pipe�Ѿ���ʱ��д��᲻�Ḳ��
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int pipe_write(void * Pipe, void * pElements, unsigned Length)
{
    THREAD              *Thread;                                            /* �߳̿��ƿ�                           */    
    INT32U               Size;                                              /* ���ݳ���                             */
    INT32U               Priority;                                          /* ���ȼ�                               */
    int                  Key;                                               /* �ٽ���                               */
    struct pipe_t       *pPipe = (struct pipe_t *)Pipe;                     /* �ܵ�                                 */
    char                *pInputData;                                        /* ��������                             */


    if ((NULL == Pipe) || (NULL == pElements))
    {
        return ERR_BAD_PARAM;
    }

    /*
     * 1) ��������
     */
    pInputData = (char *)pElements;
    
    /*
     * pPipe->In - pPipe->Out : ��ʾ����������ʹ�õĿռ��С;
     * pPipe->Lenth - (pPipe->In - pPipe->Out)��ʾ����δʹ�õĿռ�.
     */
    Length = min(Length, pPipe->Lenth - pPipe->In + pPipe->Out);            /* ȡ����֮С����ʾʵ��Ҫ�������ֽ���   */
  
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

    if (NULL == (Thread = pPipe->pReadThread))                              /* û���̵߳ȴ�                         */
    {
        goto exit;
    }

    /* 
     * 2) �߳̾���
     *    ���ܵ��ȴ��������߳̿��Ʊ���뵽ָ���̶߳���.
     */    
    Priority = Thread->Priority;     
    Key = atom_operate_lock();
    list_add(&Thread->Node, &OS_ActiveProc[Priority]);
    OS_ActivePrio |= 1ul << Priority;
    atom_operate_unlock(Key);

    /* 
     * 3) �л��߳�
     */
    scheduler();

exit:
    return Length;    
}
  
/*********************************************************************************************************************
** Function name:           pipe_read
** Descriptions:            �ӹܵ�������
** Input parameters:        Pipe      : �ܵ�����
**                          pElements : ���ݻ�����
**                          Length    : ����Ԫ�ظ���
** Output parameters:       
** Returned value:          ʵ�ʶ���������Ԫ�ظ���
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
    THREAD              *pThread;                                           /* �߳�                                 */    
    INT32U               Priority;                                          /* ���ȼ�                               */    
    int                  Key;
    char                *pOutputData;                                       /* �������                             */
    struct pipe_t       *pPipe = (struct pipe_t *)Pipe;   


    if ((NULL == Pipe) || (NULL == pElements))
    {
        return ERR_BAD_PARAM;
    }
    
    /*
     * �ӹܵ���������������(�������ķ�ʽ)
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
     * 1) ����ǰ�̹߳���
     */
    Priority = pThread->Priority;
    list_del(&pThread->Node);
    if (list_empty(&OS_ActiveProc[Priority]))
    {   
        OS_ActivePrio &= ~(1ul << Priority);
    }

    /*
     * 2) ���ó�ʱ��������
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
     * 4.1) ɾ����ʱ��������
     */
    Key = atom_operate_lock();
    if (Timeout)
    {
        OS_timer_del(&pThread->Timer);                                      /* ɾ�����Ѷ�ʱ��                       */    
    }
    pPipe->pReadThread = NULL;
    
    /*
     * 5) �ӹܵ���������������(�������ķ�ʽ)
     */
    if (pPipe->In == pPipe->Out)                                            /* ����Ϊ��                             */
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


