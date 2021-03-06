/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            uart0.h
** Last version:         V1.00
** Descriptions:         系统之数据类型头文件.
** Hardware platform:    
** SoftWare platform:
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011年2月19日  21:24:27
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
#ifndef __UART0_H
#define __UART0_H

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <include/macro.h>
#include    <include/types.h>
#include    "../uart.h"

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/


/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    接口申明区
*********************************************************************************************************************/
extern const CLASS_UART        uart0;                                       /* 设备: uart0                          */


#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* end of __UART0_H                     */

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

