/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            sysio.h
** Last version:         V1.00
** Descriptions:         系统输入/输出头文件.
** Hardware platform:
** SoftWare platform:
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Fengliang
** Created date:         2010年12月13日  18:59:55
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
#ifndef __SYSIO_H_
#define __SYSIO_H_

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <include/types.h>

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 系统I/O设备配置 -------------------------------------------------------------------------------------------------*/
#define     DEVICE_SYSIO_EN                         1                       /* 设备配置(0:禁能; 1:使能)             */

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/



#if (DEVICE_SYSIO_EN == 1)
char    scanc  (void);
INT32S  printc (char Char);
INT32S  prints (const char *string);
int     printf (const char *FmtStr, ...);

#endif                                                                      /* (DEVICE_SYSIO_EN == 1)               */

#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* end of __SYSIO_H_                    */
/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

