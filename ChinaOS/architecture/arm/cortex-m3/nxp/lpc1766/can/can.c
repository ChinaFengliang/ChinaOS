/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            can.c
** Last version:         V1.00
** Descriptions:         can总线控制器驱动文件.
** Hardware platform:    lpc1768
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2011年12月13日  17:40:27
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
#include    <string.h>
#include    <include/macro.h>
#include    <include/types.h>
#include    <library/ioport.h>
#include    <nvic/interrupt.h>
#include    <kernel/pipe/pipe.h>
#include    "./can.h"
#include    "./can0.h"
#include    "./can1.h"
#include    "./can_filter.h"

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 调试模式 --------------------------------------------------------------------------------------------------------*/
#define     SELF_TEST_MODE       1                                          /* 自发自收模式                         */
#define     RESET                0                                          /* can控制器模式: 复位模式              */
#define     NORMAL               1                                          /* can控制器模式: 正常模式              */
/* 寄存器偏移地址 --------------------------------------------------------------------------------------------------*/
#define     MOD               0x00                                          /* 控制CAN控制器的操作模式              */
#define     CMR               0x04                                          /* 影响CAN控制器状态的命令位            */
#define     GSR               0x08                                          /* 全局控制状态和错误计数器             */
#define     ICR               0x0C                                          /* 中断状态,仲裁丢失捕获,错误代码捕获   */
#define     IER               0x10                                          /* 中断使能                             */
#define     BTR               0x14                                          /* 总线时序                             */
#define     EWL               0x18                                          /* 错误警报界限                         */
#define     SR                0x1C                                          /* 状态寄存器                           */

#define     RFS               0x20                                          /* 接收帧状态                           */
#define     RID               0x24                                          /* 接收到的标识符                       */
#define     RDA               0x28                                          /* 接收到的数据字节1~4                  */
#define     RDB               0x2C                                          /* 接收到的数据字节5~8                  */

#define     TFI1              0x30                                          /* 发送帧信息(Tx缓冲器1)                */
#define     TID1              0x34                                          /* 发送标识符(Tx缓冲器1)                */
#define     TDA1              0x38                                          /* 发送数据字节1~4(Tx缓冲器1)           */
#define     TDB1              0x3C                                          /* 发送数据字节5~8(Tx缓冲器1)           */

#define     TFI2              0x40                                          /* 发送帧信息(Tx缓冲器2)                */
#define     TID2              0x44                                          /* 发送标识符(Tx缓冲器2)                */
#define     TDA2              0x48                                          /* 发送数据字节1~4(Tx缓冲器2)           */
#define     TDB2              0x4C                                          /* 发送数据字节5~8(Tx缓冲器2)           */

#define     TFI3              0x50                                          /* 发送帧信息(Tx缓冲器3)                */
#define     TID3              0x54                                          /* 发送标识符(Tx缓冲器3)                */
#define     TDA3              0x58                                          /* 发送数据字节1~4(Tx缓冲器3)           */
#define     TDB3              0x5C                                          /* 发送数据字节5~8(Tx缓冲器3)           */

/* 中央寄存器地址 --------------------------------------------------------------------------------------------------*/
#define     CANTXSR           0x40040000                                    /* CAN中央发送状态寄存器                */
#define     CANRXSR           0x40040004                                    /* CAN中央接收状态寄存器                */
#define     CANMSR            0x40040008                                    /* CAN中央杂项状态寄存器                */

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
#if  (DEVICE_CAN0_ENABLE == 1) 
extern CAN_FEATURE can0_feature;
#endif

#if  (DEVICE_CAN1_ENABLE == 1)     
extern CAN_FEATURE can1_feature;
#endif

/*********************************************************************************************************************
** Function name:           can_ctrl_mode
** Descriptions:            设置can控制器模式
** Input parameters:        pThis : 设备特征清单
**                          Mode  : can控制器模式
**                                  0 : 复位模式
**                                  1 : 工作模式
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-16  15:17:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void can_ctrl_mode(CAN_FEATURE *pThis, int Mode)
{
    INT32U          BaseAddress;                                            /* 寄存器基址                           */

    BaseAddress = pThis->BaseAddress;
    if (0 == Mode)                                                          /* 复位模式                             */
    {
        dword_set_bits(BaseAddress + MOD, 0x01);
    }
    else                                                                    /* 工作模式                             */
    {
        INT32U      Status;

        dword_clear_bits(BaseAddress + MOD, 0x01);
        do
        {
            Status = read_dword(BaseAddress + MOD);
        }while (Status & 0x01);
    }
}

/*********************************************************************************************************************
** Function name:           can_handle
** Descriptions:            中断事件处理
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-1-7  18:15:38
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void can_handle(CAN_FEATURE *pThis)
{
    int             Status;                                                 /* 寄存器状态值                         */
    INT32U          BaseAddress;                                            /* 寄存器基址                           */    
    INT32U          aMessage[4];


    BaseAddress = pThis->BaseAddress;
    Status = read_dword(BaseAddress + ICR);

    /*
     * 接收中断
     */
    if ((1ul << 0) & Status)
    {
        memcpy(aMessage, (void *)(BaseAddress + RFS), 16);
        write_dword(BaseAddress + CMR, 1ul << 2);                           /* 释放接收寄存器                       */
        pipe_write(pThis->pRecvPipe, aMessage, 1);
    }

    /*
     * 发送中断
     */
    if ((1ul << 1) & Status)
    {
        mail_post(pThis->SendMailBox, 0);
    }
}

/*********************************************************************************************************************
** Function name:           can_exception
** Descriptions:            can控制器中断服务
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-16  15:17:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void can_exception(void)
{
#if  (DEVICE_CAN0_ENABLE == 1)    
    can_handle(&can0_feature);
#endif

#if  (DEVICE_CAN1_ENABLE == 1)
    can_handle(&can1_feature);
#endif
}

/*********************************************************************************************************************
** Function name:           can_setup
** Descriptions:            can类设备安装
** Input parameters:        pThis   : 设备特征清单
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:13:54
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_setup(CAN_FEATURE *pThis)
{
    INT32U          BaseAddress;                                            /* 寄存器基址                           */
    MAILBOX         Mailbox;                                                /* 消息邮箱                             */
    void           *pRecvPipe;                                              /* 接收管道                             */


    can_filter_setup();

    BaseAddress = pThis->BaseAddress;

    /*
     * 初始化设备时钟
     */
    can_ctrl_mode(pThis, 0);
#if 0    
    dword_clr_bits(BaseAddress + MOD, (0 << 1) |                            /* 只听模式:   0:禁能; 1:使能           */        
                                      (1 << 2) |                            /* 自测模式:   0:禁能; 1:使能           */
                                      (0 << 3) |                            /* 发送优先级: 0:CAN ID; 1:消息优先级   */
                                      (0 << 4) |                            /* 睡眠模式:   0:正常模式; 1:睡眠模式   */
                                      (0 << 5) |                            /* 接收极性:   0:正常模式; 1:反向模式   */
                                      (0 << 7));                            /* 测试模式:   0:禁能; 1:使能           */
#else
    dword_set_bits(BaseAddress + MOD, (1ul << 2));                          /* 自测模式:   0:禁能; 1:使能           */
#endif
    /*
     * 配置设备控制器
     */
    write_dword(BaseAddress + IER, (1ul << 0) |                             /* 接收中断                        使能 */
                                   (1ul << 1) |                             /* 发送缓冲区1中断                 使能 */
                                   (1ul << 2) |                             /* 错误警告中断                    使能 */
                                   (1ul << 3) |                             /* 数据溢出                        使能 */
                                   (0ul << 4) |                             /* 唤醒中断                        使能 */
                                   (0ul << 5) |                             /* 消极错误中断                    使能 */
                                   (0ul << 6) |                             /* 仲裁竞争失败                    使能 */
                                   (0ul << 7) |                             /* 总线错误中断                    使能 */
                                   (0ul << 8) |                             /* ID就绪中断                      使能 */
                                   (0ul << 9) |                             /* 发送缓冲区2中断                 使能 */
                                   (0ul << 10));                            /* 发送缓冲区3中断                 使能 */
    
    write_dword(BaseAddress + GSR, 0x00);                                   /* 清除接收错误计数器和发送错误计数器   */
    write_dword(BaseAddress + BTR, (0x04        <<  0) |                    /* 控制器预分频                         */
                                   (pThis->SJW  << 14) |                    /* 再同步补偿宽度                       */
                                   (pThis->PBS1 << 16) |                    /* 相伴缓冲段1                          */
                                   (pThis->PBS2 << 20) |                    /* 相伴缓冲段2                          */
                                   (0ul         << 23));                    /* 采样点方式: 0:1点采样; 1:3点采样     */

    /*
     * 中断服务中，将接收到的消息报文写入到接收管道.
     */
    pRecvPipe = pipe_new(16, 8);
    if (NULL == pRecvPipe)
    {
        return ERR_SOFTWARE;
    }
    pThis->pRecvPipe = pRecvPipe;

    /*
     * 硬件存在三个发送缓存区, 但只使用其中一个发送缓存区, 
     * 所以创建一个深度为一的消息邮箱.
     */
    Mailbox = mailbox_new(1);
    if (NULL == Mailbox)
    {
        pipe_free(pThis->pRecvPipe);
        return ERR_NO_MEMERY;
    }
    pThis->SendMailBox = Mailbox;
    
    /*
     * 设置设备中断服务
     */
    irq_register(pThis->Irq, pThis->Priority, can_exception);
    can_ctrl_mode(pThis, 1);                                                /* 进入正常工作模式                     */

    return OK;
}

/*********************************************************************************************************************
** Function name:           can_send
** Descriptions:            can类设备发送消息报文
** Input parameters:        pThis    : 设备特征清单
**                          pMessage : 消息报文
**                          Timeout  : 超时时间(单位: 毫秒)
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:18:29
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_send(const CAN_FEATURE *pThis, CAN_MESSAGE *pMessage, int Timeout)
{
    INT32U          BaseAddress;                                            /* 寄存器基址                           */    
    int             Status;
    MAIL            Mail;


    /* 0)
     * 参数过滤
     */
    if (NULL == pMessage)
    {
        return ERR_BAD_PARAM;
    }
    if (~0x0F & pMessage->Size)                                             /* 一个消息报文最多包含8字节内容        */
    {
        return ERR_SOFTWARE;
    }
    
    /* 1)
     * 向总线发送<消息报文>
     */
    BaseAddress = pThis->BaseAddress;
    write_dword(BaseAddress + MOD, 0);                                      /* 自发自收                             */

    write_dword(BaseAddress + TFI1, (pMessage->Size << 16) |
                                    (0xC0000000 & pMessage->ID));
    write_dword(BaseAddress + TID1, pMessage->ID & 0x3FFFFFFF);
    write_dword(BaseAddress + TDA1, *(INT32U*)(&pMessage->aData[0]));
    write_dword(BaseAddress + TDB1, *(INT32U*)(&pMessage->aData[4]));
#if (SELF_TEST_MODE == 0)
    write_dword(BaseAddress + CMR,  0x01);                                  /* 命令: 开始发送                       */
#else
    write_dword(BaseAddress + CMR,  1ul << 4);                              /* 命令: 自发自收                       */
#endif    

    /* 2)
     * 等待<消息报文>发送完成
     */
    Status = mail_wait(pThis->SendMailBox, &Mail, Timeout);
    return Status;
}

/*********************************************************************************************************************
** Function name:           can_recv
** Descriptions:            can类设备接收消息报文
** Input parameters:        pThis    : 设备特征清单
**                          pMessage : 消息报文
**                          Timeout  : 超时时间(单位: 毫秒)
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:19:4
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_recv(const CAN_FEATURE *pThis, CAN_MESSAGE *pMessage, int Timeout)
{
    INT32U          aMessage[4];
    int             Status;
    

    Status = pipe_read(pThis->pRecvPipe, aMessage, 1, Timeout);
    if (Status < 0)
    {
        return ERR;
    }

    /*
     * 将控制器中接收到的报文转换成通用格式
     */
    pMessage->Size = 0x0F & (aMessage[0] >> 16);
    aMessage[0] &= 3ul << 30;
    pMessage->ID = aMessage[0] | aMessage[1];
    memcpy(pMessage->aData, &aMessage[2], 16);
    
    return OK;
}

/*********************************************************************************************************************
** Function name:           can_cleanup
** Descriptions:            can类设备卸载
** Input parameters:        pThis   : 设备特征清单
** Output parameters:       
** Returned value:          ==OK : 操作成功
**                          !=OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2011-12-13  17:15:12
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int can_cleanup(const CAN_FEATURE *pThis)
{
    pipe_free(pThis->pRecvPipe);
    mailbox_free(pThis->SendMailBox);
    irq_unregister(pThis->Irq);
    
    return OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

