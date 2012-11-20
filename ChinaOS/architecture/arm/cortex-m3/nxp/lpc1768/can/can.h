/******************************************************Copyright (c)**************************************************
**                                              ��������ֹһ�п��ܵĸ���Ե��
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            can.h
** Last version:         V1.00
** Descriptions:         can�ļ�.
** Hardware platform:    lpc1768
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2011��12��13��  17:26:21
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
#ifndef __CAN_H_
#define __CAN_H_

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                    ͷ�ļ���
*********************************************************************************************************************/
#include    <include/types.h>
#include    <kernel/mailbox/mailbox.h>
#include    <nvic/interrupt.h>

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ��Ϣ���� --------------------------------------------------------------------------------------------------------*/
#define     TYPE_DATA               (0ul << 30)                             /* ����֡                               */
#define     TYPE_REMOTE             (1ul << 30)                             /* ң��֡                               */

/* ��Ϣ��ʽ --------------------------------------------------------------------------------------------------------*/
#define     FORMAT_STANDARD         (0ul << 31)                             /* ��׼֡                               */
#define     FORMAT_EXTENDED         (1ul << 31)                             /* ��չ֡                               */

/* ���֡ ----------------------------------------------------------------------------------------------------------*/
#define     STANDARD_DATA           (FORMAT_STANDARD | TYPE_DATA)           /* ��׼����֡                           */
#define     STANDARD_REMOTE         (FORMAT_STANDARD | TYPE_REMOTE)         /* ��׼ң��֡                           */
#define     EXTENDED_DATA           (FORMAT_EXTENDED | TYPE_DATA)           /* ��չ����֡                           */
#define     EXTENDED_REMOTE         (FORMAT_EXTENDED | TYPE_REMOTE)         /* ��չң��֡                           */

/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �����嵥���� ----------------------------------------------------------------------------------------------------*/
struct __can_feature
{
    INT32U          BaseAddress;                                            /* �Ĵ�����ַ                           */
    int             Priority;                                               /* �ж����ȼ�                           */
    enum irq_t      Irq;                                                    /* �ж�ͨ����                           */
    void          (*pException)(void);                                      /* �жϷ���                             */

    MAILBOX         SendMailBox;                                            /* �����������                         */
    void           *pRecvPipe;                                              /* ���չܵ�                             */

    INT8U           SJW;                                                    /* ��ͬ��λ��                           */
    INT8U           PBS1;                                                   /* ��λ�����1                          */
    INT8U           PBS2;                                                   /* ��λ�����2                          */
};
typedef struct __can_feature                        CAN_FEATURE;            /* can�豸��������                      */

/* CAN��Ϣ�������� -------------------------------------------------------------------------------------------------*/
struct __can_message
{
  INT32U            ID;                                                     /* ��ʶ��                               */
  char              aData[8];                                               /* Data field                           */
  INT8U             Size;                                                   /* Length of data field in bytes        */
};
typedef struct __can_message                        CAN_MESSAGE;

/* �豸�������� ----------------------------------------------------------------------------------------------------*/
struct __class_can
{
    int          (*setup)(void);                                            /* ��װ�豸                             */
    int          (*cleanup)(void);                                          /* жװ�豸                             */
    int          (*send)(CAN_MESSAGE *pMessage, int Timeout);               /* ������Ϣ����                         */
    int          (*recv)(CAN_MESSAGE *pMessage, int Timeout);               /* ������Ϣ����                         */
    int          (*add_message)(INT32U MessageID);                          /* ���ӽ�����ϢID                       */
    int          (*remove_message)(INT32U MessageID);                       /* �Ƴ�������ϢID                       */
};
typedef struct __class_can                          CLASS_CAN;              /* can�豸����                          */

/*********************************************************************************************************************
** Function name:           can_setup
** Descriptions:            can���豸��װ
** Input parameters:        pThis    : �豸������
** Output parameters:       
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:13:54
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_setup(CAN_FEATURE *pThis);

/*********************************************************************************************************************
** Function name:           can_send
** Descriptions:            can���豸������Ϣ����
** Input parameters:        pThis    : �豸������
**                          pMessage : ��Ϣ����
**                          Timeout  : ��ʱʱ��(��λ: ����)
** Output parameters:       
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:18:29
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_send(const CAN_FEATURE *pThis, CAN_MESSAGE *pMessage, int Timeout);

/*********************************************************************************************************************
** Function name:           can_recv
** Descriptions:            can���豸������Ϣ����
** Input parameters:        pThis    : �豸�����嵥
**                          pMessage : ��Ϣ����
**                          Timeout  : ��ʱʱ��(��λ: ����)
** Output parameters:       
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:19:4
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_recv(const CAN_FEATURE *pThis, CAN_MESSAGE *pMessage, int Timeout);

/*********************************************************************************************************************
** Function name:           can_cleanup
** Descriptions:            can���豸ж��
** Input parameters:        pThis   : �豸������
** Output parameters:       
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:15:12
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_cleanup(const CAN_FEATURE *pThis);

/*********************************************************************************************************************
** Function name:           get_message_type
** Descriptions:            ��ȡ��Ϣ����
** Input parameters:        MessageID : ��Ϣ��ʶ��
** Output parameters:       
** Returned value:          ==0 : �Ƿ���Ϣ
**                          ==1 : 
**                          ==2 : 
**                          ==3 : 
**                          ==4 : 
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-19  1:28:59
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int get_message_type(INT32U MessageID);

    

#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* end of __CAN_H_                      */
/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/
