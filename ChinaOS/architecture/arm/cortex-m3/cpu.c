/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            cpu.c
** Last version:         V1.00
** Descriptions:         cpu初始化文件.
** Hardware platform:    cortex-m3
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011年6月27日  23:2:3
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

/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <include/macro.h>
#include    <include/types.h>
#include    <library/ioport.h>


/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/
#define     AIRCR                           		0xE000ED0C

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
extern void exit(INT32S Info);
extern int vic_setup(void);
extern void system_clock_init(void);

/*********************************************************************************************************************
** Function name:           cpu_init
** Descriptions:            初始化处理器
** Input parameters:
** Output parameters:
** Returned value:
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-9-20  16:11:46
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
void cpu_init(void)
{
    /* 
     * 1) 配置系统时钟
     */
    system_clock_init();

    /*
     * 2) 重定向中断向量表
     */

    /*
     * 3) 初始化中断控制器
     */  
    vic_setup();
}

/*********************************************************************************************************************
** Function name:           restart
** Descriptions:            复位系统
** Input parameters:        pOption : (保留)
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-11-29  21:13:48
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int restart(char *pOption)
{
    int             Register;
    
    Register = read_dword(AIRCR);
    Register &= 0x0000ff00;
    Register |= (0x05FA << 16) | (1ul << 2);
    write_dword(AIRCR, Register); 
    return OK;
}
EXPORT_COMMAND("restart", "复位系统", restart);


/*********************************************************************************************************************
** Function name:           stack_init
** Descriptions:            初始化线程栈
** Input parameters:        pThread      : 线程函数名
**                          pOption      : 选项
**                          pStackBottom : 栈底
** Output parameters:       
** Returned value:          当前栈顶指针
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-8-26  9:22:9
** Test recorde:
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde:
*********************************************************************************************************************/
REGISTER *stack_init(int (*pThread)(void *), void *pOption, void *pStackBottom)
{
    /*
     * cortex-m3堆栈模型: 向下生长的满栈 
     */
    REGISTER          *pStackTop = (REGISTER *)pStackBottom;

    
    /* 硬件自动保存寄存器组 */
    *pStackTop-- = (int)0x01000000;                                         /* xPSR                                 */
    *pStackTop-- = (int)pThread;                                            /* PC                                   */
    *pStackTop-- = (int)exit;                                               /* R14 / LR                             */
    *pStackTop-- = (int)0;                                                  /* R12                                  */
    *pStackTop-- = (int)0;                                                  /* R3                                   */
    *pStackTop-- = (int)0;                                                  /* R2                                   */
    *pStackTop-- = (int)0;                                                  /* R1                                   */
    *pStackTop-- = (int)pOption;                                            /* R0                                   */

    /* 软件手动保存寄存器组 */
    *pStackTop-- = (int)0;                                                  /* R11                                  */
    *pStackTop-- = (int)0;                                                  /* R10                                  */
    *pStackTop-- = (int)0;                                                  /* R9                                   */
    *pStackTop-- = (int)0;                                                  /* R8                                   */
    *pStackTop-- = (int)0;                                                  /* R7                                   */
    *pStackTop-- = (int)0;                                                  /* R6                                   */
    *pStackTop-- = (int)0;                                                  /* R5                                   */
    *pStackTop   = (int)0;                                                  /* R4                                   */

    return pStackTop;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

