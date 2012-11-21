/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:               console.c
** Last version:            V1.00
** Descriptions:            控制台文件.
** Hardware platform:
** SoftWare platform:       ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created date:            2010年12月9日  10:29:35
** Version:                 V1.00
** Descriptions:            The original version
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
#include    <library/malloc/malloc.h>
#include    <library/sysio/sysio.h>
#include    <library/printk.h>
#include    <string.h>

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/
#define CMD_LINE_SIZE                               50                      /* 命令行最大字符数目                   */


/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/
struct __service
{
    char     *pServiceName;                                                 /* 服务名称                             */
    INT32S  (*pFunction)(char *Option);                                     /* 服务函数                             */
};
typedef struct __service             SHELL_SERVICE;                         /* shell服务类型                        */

/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
extern int                  aConsoleServices$$Base;                         /* 合并节的开始地址                     */
extern int                  aConsoleServices$$Limit;                        /* 合并节的末尾的后面的字节的地址       */
extern const char           aCmdPrompt[];                   				/* 命令提示符                           */
static char                 aCmdBuffer[CMD_LINE_SIZE];                      /* 命令缓存区                           */
/* 状态信息 --------------------------------------------------------------------------------------------------------*/
static const char          *aStatus[] =                                     /* 命令状态描述                         */
{
    "操作成功",
    "操作失败",
    "硬件错误",
    "软件错误",
    "非法参数",
    "操作超时",
    "设备忙碌",
    "内存不足",
    "非法标识"
};

/*********************************************************************************************************************
** Function name:           service
** Descriptions:            获取服务入口函数
** Input parameters:        pCmdName
** Output parameters:       
** Returned value:          ==NULL : 
**                          !=NULL : 
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-12-10  15:3:33
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void service(char *pCmdName)
{
    /* 
     * 查询服务列表, 设置服务入口.
     * 1) 查收命令列表以获取目标服务指针;
     * 2) 指行指定服务函数.
     */

    
    int             CmdSize;                                                /* 命令长度                             */
    int             Status;                                                 /* 服务状态                             */
    int           (*pService)(char *Option) = NULL;                         /* 服务函数                             */
    SERVICE        *pItem;

    /*
     *  计算命令字符数量保存至CmdSize中.
     *  例如: 命令行"cmd_option"
     */
    for (CmdSize = 0; pCmdName[CmdSize] != '\0'; CmdSize++)
    {
        if (' ' == pCmdName[CmdSize])
        {
            pCmdName[CmdSize] = '\0';
            break;
        }
    }

    /*
     * 获取服务入口函数保存至pService
     */
    for (pItem = (SERVICE*)&aConsoleServices$$Base; 
         pItem < (SERVICE*)&aConsoleServices$$Limit; pItem++)
    {
        if (CmdSize != strlen(pItem->pName))
        {
            continue;
        }
        if (0 == memcmp(pCmdName, pItem->pName, CmdSize))                    /* 服务名称比较                         */
        {
            pService = pItem->pFunction;
            break;
        }
    }

    /*
     * 服务参数问题
     */
    if (NULL != pService)
    {
        Status = (*pService)(pCmdName + CmdSize + 1);                       /* 执行服务函数                         */
        printk("%s命令执行状态: %d, %s\r\n\n",
               pCmdName, Status, aStatus[-Status]);
    }
    else
    {
        prints("This command is not supported!\r\n");
    }
}

/*********************************************************************************************************************
** Function name:           command
** Descriptions:            
** Input parameters:        
** Output parameters:       
** Returned value:          ==NULL : 操作失败
**                          !=NULL : 操作成功
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2010-12-11  9:29:25
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void command(void)
{
    /* 支持消除前端空格字符;
     * 支持空命令;
     * 1) 命令格式: 命令描述符 + 回行符.
     * 2) 此函数由空闲线程调用.
     */

    
    INT32U          CharCounter;                                            /* 有效字符计数器                       */
    char            Char;                                                   /* 字符                                 */


    CharCounter = 0;
    aCmdBuffer[CMD_LINE_SIZE-1] = '\0';
    do
    {
        Char = scanc();
        
        
        /*
         * 不同制式的键盘的退格键符不同
         * a)  0x7F
         * b)  0x08
         */                              
        if ((0x7F==Char) || (0x08==Char))                                   /* 退格键符                             */
        {
            if (CharCounter)
            {
                /*
                 * 在屏幕上消除最后一个字符的办法:
                 * 1) 输出 退格符('\b');
                 * 2) 输出 空格符(' ');
                 * 3) 输出 退格符('\b').
                 */
                #if 0 
                printc('\b');
                printc(' ');
                printc('\b');
                #else
                printc(Char);
                #endif
                
                CharCounter--;
            }
            continue;
        }
        else 
        {
            if (0x20 == Char)                                               /* 空格键符                             */
            {
                if (0 == CharCounter)
                {
                    continue;
                }
                if (' ' == aCmdBuffer[CharCounter-1])
                {
                    continue;
                }
            }
            else
            {
                if (0x0D == Char)                                           /* 回车键符                             */
                {
                    printc('\r');                   
                    printc('\n');

                    if(CharCounter)
                    {
                        break;
                    }
                    goto end;
                }
            }
            printc(Char);
            aCmdBuffer[CharCounter++] = Char;
        }
    }while (CharCounter < (CMD_LINE_SIZE -1));

    /*
     * 删除最后可能输入的空格字符
     */
    aCmdBuffer[CharCounter] = '\0';

    /*
     * 此时输入命令字符串格式: aCmdBuffer: cmd_a_b
     */

    service(aCmdBuffer);
end:
    prints(aCmdPrompt);                                                     /* 输出提示符                           */
}

/*********************************************************************************************************************
** Function name:           console_setup
** Descriptions:            安装控制台
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-10-30  18:43:15
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int console_setup(void)
{
#if 0
    _syscall_table_begin  = (CONSOLE_SERVICE*)&aConsoleServices$$Base;
    _syscall_table_end = (CONSOLE_SERVICE*)&aConsoleServices$$Limit;

    /* create or set shell structure */
    shell = (struct finsh_shell*)malloc(sizeof(struct finsh_shell));
    if (shell == NULL)
    {
        return OS_ERR_NO_MEMERY;
    }
    memset(shell, 0, sizeof(struct finsh_shell));
    shell->echo_mode = 1;                                                   
    finsh_init(&shell->parser);

#endif    
    return OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

