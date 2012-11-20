/******************************************************Copyright (c)**************************************************
**                                              ��������ֹһ�п��ܵĸ���Ե��
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            can.c
** Last version:         V1.00
** Descriptions:         can���߿����������ļ�.
** Hardware platform:    lpc1768
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2011��12��13��  17:40:27
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
#include    <library/ioport.h>
#include    <nvic/interrupt.h>
#include    <kernel/pipe/pipe.h>
#include    "./can.h"
#include    "./can0.h"
#include    "./can1.h"
#include    "./can_filter.h"

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ����ģʽ --------------------------------------------------------------------------------------------------------*/
#define     SELF_TEST_MODE       1                                          /* �Է�����ģʽ                         */
#define     RESET                0                                          /* can������ģʽ: ��λģʽ              */
#define     NORMAL               1                                          /* can������ģʽ: ����ģʽ              */
/* �Ĵ���ƫ�Ƶ�ַ --------------------------------------------------------------------------------------------------*/
#define     MOD               0x00                                          /* ����CAN�������Ĳ���ģʽ              */
#define     CMR               0x04                                          /* Ӱ��CAN������״̬������λ            */
#define     GSR               0x08                                          /* ȫ�ֿ���״̬�ʹ��������             */
#define     ICR               0x0C                                          /* �ж�״̬,�ٲö�ʧ����,������벶��   */
#define     IER               0x10                                          /* �ж�ʹ��                             */
#define     BTR               0x14                                          /* ����ʱ��                             */
#define     EWL               0x18                                          /* ���󾯱�����                         */
#define     SR                0x1C                                          /* ״̬�Ĵ���                           */

#define     RFS               0x20                                          /* ����֡״̬                           */
#define     RID               0x24                                          /* ���յ��ı�ʶ��                       */
#define     RDA               0x28                                          /* ���յ��������ֽ�1~4                  */
#define     RDB               0x2C                                          /* ���յ��������ֽ�5~8                  */

#define     TFI1              0x30                                          /* ����֡��Ϣ(Tx������1)                */
#define     TID1              0x34                                          /* ���ͱ�ʶ��(Tx������1)                */
#define     TDA1              0x38                                          /* ���������ֽ�1~4(Tx������1)           */
#define     TDB1              0x3C                                          /* ���������ֽ�5~8(Tx������1)           */

#define     TFI2              0x40                                          /* ����֡��Ϣ(Tx������2)                */
#define     TID2              0x44                                          /* ���ͱ�ʶ��(Tx������2)                */
#define     TDA2              0x48                                          /* ���������ֽ�1~4(Tx������2)           */
#define     TDB2              0x4C                                          /* ���������ֽ�5~8(Tx������2)           */

#define     TFI3              0x50                                          /* ����֡��Ϣ(Tx������3)                */
#define     TID3              0x54                                          /* ���ͱ�ʶ��(Tx������3)                */
#define     TDA3              0x58                                          /* ���������ֽ�1~4(Tx������3)           */
#define     TDB3              0x5C                                          /* ���������ֽ�5~8(Tx������3)           */

/* ����Ĵ�����ַ --------------------------------------------------------------------------------------------------*/
#define     CANTXSR           0x40040000                                    /* CAN���뷢��״̬�Ĵ���                */
#define     CANRXSR           0x40040004                                    /* CAN�������״̬�Ĵ���                */
#define     CANMSR            0x40040008                                    /* CAN��������״̬�Ĵ���                */

/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/
#if  (DEVICE_CAN0_ENABLE == 1) 
extern CAN_FEATURE can0_feature;
#endif

#if  (DEVICE_CAN1_ENABLE == 1)     
extern CAN_FEATURE can1_feature;
#endif

/*********************************************************************************************************************
** Function name:           can_ctrl_mode
** Descriptions:            ����can������ģʽ
** Input parameters:        pThis : �豸�����嵥
**                          Mode  : can������ģʽ
**                                  0 : ��λģʽ
**                                  1 : ����ģʽ
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-16  15:17:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void can_ctrl_mode(CAN_FEATURE *pThis, int Mode)
{
    INT32U          BaseAddress;                                            /* �Ĵ�����ַ                           */

    BaseAddress = pThis->BaseAddress;
    if (0 == Mode)                                                          /* ��λģʽ                             */
    {
        dword_set_bits(BaseAddress + MOD, 0x01);
    }
    else                                                                    /* ����ģʽ                             */
    {
        INT32U      Status;

        dword_clear_bits(BaseAddress + MOD, 0x01);
        do
        {
            Status = read_dword(BaseAddress + MOD);
        }while (Status & 0x01);
    }
}

/*********************************************************************************************************************
** Function name:           can_handle
** Descriptions:            �ж��¼�����
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-1-7  18:15:38
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void can_handle(CAN_FEATURE *pThis)
{
    int             Status;                                                 /* �Ĵ���״ֵ̬                         */
    INT32U          BaseAddress;                                            /* �Ĵ�����ַ                           */    
    INT32U          aMessage[4];


    BaseAddress = pThis->BaseAddress;
    Status = read_dword(BaseAddress + ICR);

    /*
     * �����ж�
     */
    if ((1ul << 0) & Status)
    {
        memcpy(aMessage, (void *)(BaseAddress + RFS), 16);
        write_dword(BaseAddress + CMR, 1ul << 2);                           /* �ͷŽ��ռĴ���                       */
        pipe_write(pThis->pRecvPipe, aMessage, 1);
    }

    /*
     * �����ж�
     */
    if ((1ul << 1) & Status)
    {
        mail_post(pThis->SendMailBox, 0);
    }
}

/*********************************************************************************************************************
** Function name:           can_exception
** Descriptions:            can�������жϷ���
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-16  15:17:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void can_exception(void)
{
#if  (DEVICE_CAN0_ENABLE == 1)    
    can_handle(&can0_feature);
#endif

#if  (DEVICE_CAN1_ENABLE == 1)
    can_handle(&can1_feature);
#endif
}

/*********************************************************************************************************************
** Function name:           can_setup
** Descriptions:            can���豸��װ
** Input parameters:        pThis   : �豸�����嵥
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:13:54
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_setup(CAN_FEATURE *pThis)
{
    INT32U          BaseAddress;                                            /* �Ĵ�����ַ                           */
    MAILBOX         Mailbox;                                                /* ��Ϣ����                             */
    void           *pRecvPipe;                                              /* ���չܵ�                             */


    can_filter_setup();

    BaseAddress = pThis->BaseAddress;

    /*
     * ��ʼ���豸ʱ��
     */
    can_ctrl_mode(pThis, 0);
#if 0    
    dword_clr_bits(BaseAddress + MOD, (0 << 1) |                            /* ֻ��ģʽ:   0:����; 1:ʹ��           */        
                                      (1 << 2) |                            /* �Բ�ģʽ:   0:����; 1:ʹ��           */
                                      (0 << 3) |                            /* �������ȼ�: 0:CAN ID; 1:��Ϣ���ȼ�   */
                                      (0 << 4) |                            /* ˯��ģʽ:   0:����ģʽ; 1:˯��ģʽ   */
                                      (0 << 5) |                            /* ���ռ���:   0:����ģʽ; 1:����ģʽ   */
                                      (0 << 7));                            /* ����ģʽ:   0:����; 1:ʹ��           */
#else
    dword_set_bits(BaseAddress + MOD, (1ul << 2));                          /* �Բ�ģʽ:   0:����; 1:ʹ��           */
#endif
    /*
     * �����豸������
     */
    write_dword(BaseAddress + IER, (1ul << 0) |                             /* �����ж�                        ʹ�� */
                                   (1ul << 1) |                             /* ���ͻ�����1�ж�                 ʹ�� */
                                   (1ul << 2) |                             /* ���󾯸��ж�                    ʹ�� */
                                   (1ul << 3) |                             /* �������                        ʹ�� */
                                   (0ul << 4) |                             /* �����ж�                        ʹ�� */
                                   (0ul << 5) |                             /* ���������ж�                    ʹ�� */
                                   (0ul << 6) |                             /* �ٲþ���ʧ��                    ʹ�� */
                                   (0ul << 7) |                             /* ���ߴ����ж�                    ʹ�� */
                                   (0ul << 8) |                             /* ID�����ж�                      ʹ�� */
                                   (0ul << 9) |                             /* ���ͻ�����2�ж�                 ʹ�� */
                                   (0ul << 10));                            /* ���ͻ�����3�ж�                 ʹ�� */
    
    write_dword(BaseAddress + GSR, 0x00);                                   /* ������մ���������ͷ��ʹ��������   */
    write_dword(BaseAddress + BTR, (0x04        <<  0) |                    /* ������Ԥ��Ƶ                         */
                                   (pThis->SJW  << 14) |                    /* ��ͬ���������                       */
                                   (pThis->PBS1 << 16) |                    /* ��黺���1                          */
                                   (pThis->PBS2 << 20) |                    /* ��黺���2                          */
                                   (0ul         << 23));                    /* �����㷽ʽ: 0:1�����; 1:3�����     */

    /*
     * �жϷ����У������յ�����Ϣ����д�뵽���չܵ�.
     */
    pRecvPipe = pipe_new(16, 8);
    if (NULL == pRecvPipe)
    {
        return ERR_SOFTWARE;
    }
    pThis->pRecvPipe = pRecvPipe;

    /*
     * Ӳ�������������ͻ�����, ��ֻʹ������һ�����ͻ�����, 
     * ���Դ���һ�����Ϊһ����Ϣ����.
     */
    Mailbox = mailbox_new(1);
    if (NULL == Mailbox)
    {
        pipe_free(pThis->pRecvPipe);
        return ERR_NO_MEMERY;
    }
    pThis->SendMailBox = Mailbox;
    
    /*
     * �����豸�жϷ���
     */
    irq_register(pThis->Irq, pThis->Priority, can_exception);
    can_ctrl_mode(pThis, 1);                                                /* ������������ģʽ                     */

    return OK;
}

/*********************************************************************************************************************
** Function name:           can_send
** Descriptions:            can���豸������Ϣ����
** Input parameters:        pThis    : �豸�����嵥
**                          pMessage : ��Ϣ����
**                          Timeout  : ��ʱʱ��(��λ: ����)
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:18:29
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_send(const CAN_FEATURE *pThis, CAN_MESSAGE *pMessage, int Timeout)
{
    INT32U          BaseAddress;                                            /* �Ĵ�����ַ                           */    
    int             Status;
    MAIL            Mail;


    /* 0)
     * ��������
     */
    if (NULL == pMessage)
    {
        return ERR_BAD_PARAM;
    }
    if (~0x0F & pMessage->Size)                                             /* һ����Ϣ����������8�ֽ�����        */
    {
        return ERR_SOFTWARE;
    }
    
    /* 1)
     * �����߷���<��Ϣ����>
     */
    BaseAddress = pThis->BaseAddress;
    write_dword(BaseAddress + MOD, 0);                                      /* �Է�����                             */

    write_dword(BaseAddress + TFI1, (pMessage->Size << 16) |
                                    (0xC0000000 & pMessage->ID));
    write_dword(BaseAddress + TID1, pMessage->ID & 0x3FFFFFFF);
    write_dword(BaseAddress + TDA1, *(INT32U*)(&pMessage->aData[0]));
    write_dword(BaseAddress + TDB1, *(INT32U*)(&pMessage->aData[4]));
#if (SELF_TEST_MODE == 0)
    write_dword(BaseAddress + CMR,  0x01);                                  /* ����: ��ʼ����                       */
#else
    write_dword(BaseAddress + CMR,  1ul << 4);                              /* ����: �Է�����                       */
#endif    

    /* 2)
     * �ȴ�<��Ϣ����>�������
     */
    Status = mail_wait(pThis->SendMailBox, &Mail, Timeout);
    return Status;
}

/*********************************************************************************************************************
** Function name:           can_recv
** Descriptions:            can���豸������Ϣ����
** Input parameters:        pThis    : �豸�����嵥
**                          pMessage : ��Ϣ����
**                          Timeout  : ��ʱʱ��(��λ: ����)
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:19:4
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_recv(const CAN_FEATURE *pThis, CAN_MESSAGE *pMessage, int Timeout)
{
    INT32U          aMessage[4];
    int             Status;
    

    Status = pipe_read(pThis->pRecvPipe, aMessage, 1, Timeout);
    if (Status < 0)
    {
        return ERR;
    }

    /*
     * ���������н��յ��ı���ת����ͨ�ø�ʽ
     */
    pMessage->Size = 0x0F & (aMessage[0] >> 16);
    aMessage[0] &= 3ul << 30;
    pMessage->ID = aMessage[0] | aMessage[1];
    memcpy(pMessage->aData, &aMessage[2], 16);
    
    return OK;
}

/*********************************************************************************************************************
** Function name:           can_cleanup
** Descriptions:            can���豸ж��
** Input parameters:        pThis   : �豸�����嵥
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:15:12
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_cleanup(const CAN_FEATURE *pThis)
{
    pipe_free(pThis->pRecvPipe);
    mailbox_free(pThis->SendMailBox);
    irq_unregister(pThis->Irq);
    
    return OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

