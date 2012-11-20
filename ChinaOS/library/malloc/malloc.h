/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            malloc.h
** Last version:         V1.00
** Descriptions:         �ڴ������ͷ�ļ�.
** Hardware platform:    
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011��3��20��  17:59:57
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
#ifndef __MALLOC_H
#define __MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                    ͷ�ļ���
*********************************************************************************************************************/
#include <include/macro.h>
#include <include/types.h>

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ����� ----------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
** Function name:           malloc
** Descriptions:            �����ڴ�ռ�
** Input parameters:        Size : �����ڴ�ռ��С(��λ: Byte)
** Output parameters:       
** Returned value:          ==NULL : ����ʧ��
**                          !=NULL : �����ɹ�(�ڴ�ռ��ַ)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-20  18:1:49
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void* malloc(INT32U Size);

/*********************************************************************************************************************
** Function name:           free
** Descriptions:            �ͷ��ڴ�ռ�
** Input parameters:        pMemory : �ڴ�ռ��ַ
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-3-20  18:3:31
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void free(void * pMemory);


#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* end of __MALLOC_H                    */

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/
