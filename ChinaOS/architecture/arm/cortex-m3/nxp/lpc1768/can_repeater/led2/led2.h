/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            led2.h
** Last version:         V1.00
** Descriptions:         报警器接口文件.
** Hardware platform:    lpc23xx
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Fengliang
** Created date:         2010年10月13日  14:17:50
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
#ifndef __LED_2_H_
#define __LED_2_H_

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <config.h>
#include    <include/types.h>
#include    "../led.h"

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/



/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/



/*********************************************************************************************************************
                                                    接口申明区
*********************************************************************************************************************/
#if DEVICE_LED_2_EN == 1
extern const CLASS_LED    led2;                                             /* 设备: 指示灯                         */
#endif


#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* end of __LED_2_H_                    */
/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

