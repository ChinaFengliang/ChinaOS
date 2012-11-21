/******************************************************Copyright (c)**************************************************
**                                              ��������ֹһ�п��ܵĸ���Ե��
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:               console.c
** Last version:            V1.00
** Descriptions:            ����̨�ļ�.
** Hardware platform:
** SoftWare platform:       ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created date:            2010��12��9��  10:29:35
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
                                                    ͷ�ļ���
*********************************************************************************************************************/
#include    <include/macro.h>
#include    <include/types.h>
#include    <library/malloc/malloc.h>
#include    <library/sysio/sysio.h>
#include    <library/printk.h>
#include    <string.h>

/*********************************************************************************************************************
                                                    �궨����
*********************************************************************************************************************/
/* ����� ----------------------------------------------------------------------------------------------------------*/
#define CMD_LINE_SIZE                               50                      /* ����������ַ���Ŀ                   */


/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/
struct __service
{
    char     *pServiceName;                                                 /* ��������                             */
    INT32S  (*pFunction)(char *Option);                                     /* ������                             */
};
typedef struct __service             SHELL_SERVICE;                         /* shell��������                        */

/*********************************************************************************************************************
                                                  ȫ�ֱ���������
*********************************************************************************************************************/
extern int                  aConsoleServices$$Base;                         /* �ϲ��ڵĿ�ʼ��ַ                     */
extern int                  aConsoleServices$$Limit;                        /* �ϲ��ڵ�ĩβ�ĺ�����ֽڵĵ�ַ       */
extern const char           aCmdPrompt[];                   				/* ������ʾ��                           */
static char                 aCmdBuffer[CMD_LINE_SIZE];                      /* �������                           */
/* ״̬��Ϣ --------------------------------------------------------------------------------------------------------*/
static const char          *aStatus[] =                                     /* ����״̬����                         */
{
    "�����ɹ�",
    "����ʧ��",
    "Ӳ������",
    "�������",
    "�Ƿ�����",
    "������ʱ",
    "�豸æµ",
    "�ڴ治��",
    "�Ƿ���ʶ"
};

/*********************************************************************************************************************
** Function name:           service
** Descriptions:            ��ȡ������ں���
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
     * ��ѯ�����б�, ���÷������.
     * 1) ���������б��Ի�ȡĿ�����ָ��;
     * 2) ָ��ָ��������.
     */

    
    int             CmdSize;                                                /* �����                             */
    int             Status;                                                 /* ����״̬                             */
    int           (*pService)(char *Option) = NULL;                         /* ������                             */
    SERVICE        *pItem;

    /*
     *  ���������ַ�����������CmdSize��.
     *  ����: ������"cmd_option"
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
     * ��ȡ������ں���������pService
     */
    for (pItem = (SERVICE*)&aConsoleServices$$Base; 
         pItem < (SERVICE*)&aConsoleServices$$Limit; pItem++)
    {
        if (CmdSize != strlen(pItem->pName))
        {
            continue;
        }
        if (0 == memcmp(pCmdName, pItem->pName, CmdSize))                    /* �������ƱȽ�                         */
        {
            pService = pItem->pFunction;
            break;
        }
    }

    /*
     * �����������
     */
    if (NULL != pService)
    {
        Status = (*pService)(pCmdName + CmdSize + 1);                       /* ִ�з�����                         */
        printk("%s����ִ��״̬: %d, %s\r\n\n",
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
** Returned value:          ==NULL : ����ʧ��
**                          !=NULL : �����ɹ�
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
    /* ֧������ǰ�˿ո��ַ�;
     * ֧�ֿ�����;
     * 1) �����ʽ: ���������� + ���з�.
     * 2) �˺����ɿ����̵߳���.
     */

    
    INT32U          CharCounter;                                            /* ��Ч�ַ�������                       */
    char            Char;                                                   /* �ַ�                                 */


    CharCounter = 0;
    aCmdBuffer[CMD_LINE_SIZE-1] = '\0';
    do
    {
        Char = scanc();
        
        
        /*
         * ��ͬ��ʽ�ļ��̵��˸������ͬ
         * a)  0x7F
         * b)  0x08
         */                              
        if ((0x7F==Char) || (0x08==Char))                                   /* �˸����                             */
        {
            if (CharCounter)
            {
                /*
                 * ����Ļ���������һ���ַ��İ취:
                 * 1) ��� �˸��('\b');
                 * 2) ��� �ո��(' ');
                 * 3) ��� �˸��('\b').
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
            if (0x20 == Char)                                               /* �ո����                             */
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
                if (0x0D == Char)                                           /* �س�����                             */
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
     * ɾ������������Ŀո��ַ�
     */
    aCmdBuffer[CharCounter] = '\0';

    /*
     * ��ʱ���������ַ�����ʽ: aCmdBuffer: cmd_a_b
     */

    service(aCmdBuffer);
end:
    prints(aCmdPrompt);                                                     /* �����ʾ��                           */
}

/*********************************************************************************************************************
** Function name:           console_setup
** Descriptions:            ��װ����̨
** Input parameters:        
** Output parameters:       
** Returned value:          ==OK : �����ɹ�
**                          !=OK : ����ʧ��(����������Ϣ)
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

