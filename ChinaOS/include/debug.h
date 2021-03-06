/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:           debug.h
**
** Last version:        V2.00
** Descriptions:        系统之调试组件.
**--------------------------------------------------------------------------------------------------------------------
** Created by:          Fengliang
** Created date:        2009-10-07
** Version:             V2.00
** Descriptions:        The original version
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
**
*********************************************************************************************************************/
#ifndef __DEBUG__H
#define __DEBUG__H

#ifdef __cplusplus
extern "C" {
#endif                                                                      /* __cplusplus                          */
/*********************************************************************************************************************
                                                    配置定义区
*********************************************************************************************************************/
#define DEBUG_GLOBAL_EN                             1                       /* 全局调试开关: 0:关; 1:开             */

/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <library/printk.h>

/*********************************************************************************************************************
以下为调试固件代码，用户无须关心且不可随意改变。
*********************************************************************************************************************/
#define __str(x)            #x
#define __astr(x)           __str(x)                                        /* 为兼容不同预处理方式                 */


#ifndef DEBUG_LOCAL_EN                                                      /* 本地调试开关(默认配置): 0:关; 1:开   */
#define DEBUG_LOCAL_EN      0                                               /* 默认配置                             */
#endif

#if ( DEBUG_LOCAL_EN && DEBUG_GLOBAL_EN )
#   define DBG_INFO(...)       printk(__VA_ARGS__)
#   define DBG_WARN(...)       printk("File: "__FILE__"\r\nLine: "__astr(__LINE__)"\r\n");\
                               printk("Warn: "__VA_ARGS__);
#   define DBG_ERR(...)        printk("File: "__FILE__"\r\nLine: "__astr(__LINE__)"\r\n");\
                               printk("Error: "__VA_ARGS__);\
                               while(1)
#else
#   define DBG_INFO(...)                                                    /* 打印正常调试信息                     */
#   define DBG_WARN(...)                                                    /* 打印警告调试信息                     */
#   define DBG_ERR(...)                                                     /* 打印出错调试信息                     */
#endif

#define _DBG_INFO(...)                                                      /* 单行注销                             */
#define _DBG_WARN(...)                                                      /* 单行注销                             */
#define _DBG_ERR(...)                                                       /* 单行注销                             */

#ifdef __cplusplus
}
#endif                                                                      /* __cplusplus                          */
#endif                                                                      /* __DEBUG__H                           */
/*********************************************************************************************************************
                                                   END OF FILE
*********************************************************************************************************************/

