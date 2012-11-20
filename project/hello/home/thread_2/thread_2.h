/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            thread_2.h
** Last version:         V1.00
** Descriptions:         线程头文件.
** Hardware platform:
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Fengliang
** Created date:         2010年11月16日  13:41:34
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
#ifndef __THREAD_2_H_
#define __THREAD_2_H_

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    "../include/ChinaOS.h"


/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
** Function name:           thread_2
** Descriptions:            线程2
** Input parameters:        option  : 服务选项
** Output parameters:
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-27  11:18:25
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
extern int thread_2(void *option);



#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* end of __THREAD_2_H_                 */
/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

