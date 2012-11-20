/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            uart2.h
** Last version:         V1.00
** Descriptions:         uart1ͷ�ļ�.
** Hardware platform:    
** SoftWare platform:
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011��2��19��  21:24:27
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
#ifndef __UART2_H
#define __UART2_H

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                    ͷ�ļ���
*********************************************************************************************************************/
#include    "../../../../../../../include/OS_macro.h"
#include    "../../../../../../../include/OS_types.h"
#include    "../uart.h"

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* uart2�豸���� ---------------------------------------------------------------------------------------------------*/
#define     DEVICE_UART2_EN							0						/* �豸����(0:����; 1:ʹ��)             */


/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    �ӿ�������
*********************************************************************************************************************/
#if (DEVICE_UART2_EN == 1)
extern const CLASS_UART        uart2;                                       /* �豸: uart2                          */
#endif


#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* end of __UART2_H                     */

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/
