/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:           sysio.c
**
** Last version:        V1.00
** Descriptions:        系统输入输出文件.
** Hardware platform:   
** SoftWare platform:
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


/*********************************************************************************************************************
                                                   头文件区
*********************************************************************************************************************/
#include    <stdarg.h>
#include    <stdio.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <device.h>


/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/
#define     standio             uart0

/*********************************************************************************************************************
** Function name:           getchar
** Descriptions:            输入字符
** Input parameters:        
** Output parameters:       
** Returned value:          输入字符
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-12-9  10:58:4
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
char scanc(void)
{
    char        Char;
    
    standio.recv(&Char, 1);
    return Char;
}

/*********************************************************************************************************************
** Function name:           getchar
** Descriptions:            输入字符
** Input parameters:        
** Output parameters:       
** Returned value:          输入字符
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-12-9  10:58:4
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INT32S printc(char Char)
{
   return standio.send(&Char, 1);
}

/*********************************************************************************************************************
** Function name:           putchar
** Descriptions:            输出字符串
** Input parameters:        Char    : 输出字符
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-12-9  10:53:33
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
STATUS prints(const char *pString)
{
    if (NULL == pString)
    {
        return ERR_BAD_PARAM;
    }
    
    while ('\0' != *pString)
    {
        printc(*pString++);
    }
    
    return OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/
