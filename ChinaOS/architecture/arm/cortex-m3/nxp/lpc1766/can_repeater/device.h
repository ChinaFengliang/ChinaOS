/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:           device.h
**
** Last version:        V1.00
** Environment：
** Descriptions:        系统之数据类型头文件.
**--------------------------------------------------------------------------------------------------------------------
** Created by:          Fengliang
** Created date:        2009-10-07
** Version:             V1.00
** Descriptions:        The original version
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************************/
#ifndef __OS_DEVICE_H
#define __OS_DEVICE_H

/*********************************************************************************************************************
                                                   头文件区
*********************************************************************************************************************/
/* 板级设备列表 ----------------------------------------------------------------------------------------------------*/
#include    "./led2/led2.h"
#include    "../uart/uart0/uart0.h"
#include    "../can/can_filter.h"
#include    "../can/can0.h"
#include    "../can/can1.h"
#include    "../timer/millisecond.h"

/*********************************************************************************************************************
** Function name:           device_setup
** Descriptions:            安装系统默认设备
** Input parameters:        
** Output parameters:       操作结果信息
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-24  15:57:4
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
int device_setup(void);

#endif                                                                      /* end of __OS_DEVICE_H                 */
/*********************************************************************************************************************
                                                   END OF FILE
*********************************************************************************************************************/

