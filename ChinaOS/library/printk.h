/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            printk.h
** Last version:         V1.00
** Descriptions:         文件.
** Hardware platform:
** SoftWare platform:
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2011年11月12日  23:45:3
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
#ifndef __PRINTK_H_
#define __PRINTK_H_

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */

/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/


/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
** Function name:           printk
** Descriptions:            内核调试信息打印函数
** input parameters:        Format   : 格式化字符串
**                          ...      : 扩展参数
** output parameters:       none
** Returned value:          输出字符串长度
** Created by:
** Created Date:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************************/
int printk (const char *Format, ...);


#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */

#endif                                                                      /* end of __PRINTK_H_                 */
/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

