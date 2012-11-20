/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            ethernet mac.c
** Last version:         V1.00
** Descriptions:         以太网控制器驱动文件.
** Hardware platform:    lpc1768
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2012年2月7日  22:50:46
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
#include    <include/OS_macro.h>
#include    <include/OS_types.h>
#include    <library/ioport.h>
#include    <nvic/interrupt.h>


/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 寄存器地址 ------------------------------------------------------------------------------------------------------*/
#define     MAC1                    (0x50000000)                            /* MAC配置寄存器1                       */
#define     MAC2                    (0x50000004)
#define     IPGT                    (0x50000008)
#define     IPGR                    (0x5000000C)

#define     CLRT                    (0x50000010)
#define     MAXF                    (0x50000014)
#define     SUPP                    (0x50000018)
#define     TEST                    (0x5000001C)

#define     MCFG                    (0x50000020)
#define     MCMD                    (0x50000024)
#define     MADR                    (0x50000028)
#define     MWTD                    (0x5000002C)

#define     MRDD                    (0x50000030)
#define     MIND                    (0x50000034)

#define     SA0                     (0x50000040)
#define     SA1                     (0x50000044)
#define     SA2                     (0x50000048)

#define     COMMAND                 (0x50000100)
#define     Status                  (0x50000104)
#define     RXDESCRIPTOR            (0x50000108)
#define     RXSTATUS                (0x5000010C)

#define     RXDESCRIPTORNUM         (0x50000110)
#define     RXPRODUCEINDEX          (0x50000114)
#define     RXCONSUMEINDEX          (0x50000118)
#define     TXDESCRIPTOR            (0x5000011C)

#define     TXSTATUS                (0x50000120)
#define     TXDESCRIPTORNUM         (0x50000124)
#define     TXPRODUCEINDEX          (0x50000128)
#define     TXCONSUMEINDEX          (0x5000012C)

#define     TSV0                    (0x50000158)
#define     TSV1                    (0x5000015C)

#define     RSV                     (0x50000160)

#define     FlowControlCounter      (0x50000170)
#define     FlowControlStatus       (0x50000174)

#define     RXFILTERCTRL            (0x50000200)
#define     RxFilterWoLStatus       (0x50000204)
#define     RxFilterWoLClear        (0x50000208)

#define     HashFilterL             (0x50000210)
#define     HashFilterH             (0x50000214)

#define     INTSTATUS               (0x50000FE0)
#define     INTENABLE               (0x50000FE4)
#define     INTCLEAR                (0x50000FE8)
#define     IntSet                  (0x50000FEC)

#define     PowerDown               (0x50000FF4)

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/


/*********************************************************************************************************************
** Function name:           ethernet_exception
** Descriptions:            以太网控制器中断服务
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-27  18:5:19
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
void ethernet_exception (void)
{
    ethernetif_input(NULL);
}

/*********************************************************************************************************************
** Function name:           ethernet_setup
** Descriptions:            安装以太网控制器
** Input parameters:        
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-27  18:12:42
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int ethernet_setup(void)
{
    dword_set_bits(PCONP, 1ul << 30);                                       /* power up the EMAC controller         */

    /* 配置以太网相关引脚功能 */
    write_dword(PINSEL2, 0x50150105);										/* port1[0,1,4,8,9,10,14,15]    		*/
    dword_modify(PINSEL3, (1ul<<3) | (1ul<<1),
        				  (1ul<<2) | (1ul<<0));                             /* port1[16,17]                         */

    /* 软件复位全部MAC内部模块. */
    write_word(MAC1, (1ul<<15) |											/* 复位MAC内除主机接口以外的所有模块    */
                     (1ul<<14) |											/* 复位发送功能中的随机数发生器			*/
                     (1ul<<11) |											/* 复位MAC控制子层/接收逻辑				*/
                     (1ul<<10) |											/* 复位以太网接收逻辑					*/
                     (1ul<< 9) |											/* 复位MAC控制子层/发送逻辑				*/
                     (1ul<< 8));											/* 复位发送功能的逻辑    				*/
    write_dword(COMMAND, (1ul<<5) |											/* 复位接收通道							*/
        				 (1ul<<4) |											/* 复位发送通道							*/
        				 (1ul<<3));											/* 复位所有的通道和主机寄存器			*/
    
	
    /* A short delay after reset. */
    sleep(2);     

    /*
     * 初始化MAC控制器寄存器.
     */
    write_word(MAC1, 1ul<<2);
    write_word(MAC2, (1ul<<7) |
                     (1ul<<5));
    write_word(MAXF, 1536);													/* 最大帧size: 1536						*/
	write_word(CLRT, 0x370F);
    write_word(IPGR, 0x12);
    
    write_word(MCFG, (1ul<<15) |											/* 复位MII管理硬件						*/
                     (6ul<<2));												/* 主机时钟20分频						*/
    sleep(2);
    write_word(MCFG, (6ul<<2));
    
    write_word(COMMAND, (1ul<<6) |
                        (1ul<<7) |
                        (1ul<<9));
    /*
     * 复位MII逻辑.
     */
    write_word(SUPP, (1ul<<8) |
                     (1ul<<11));
    sleep(2);
    write_word(SUPP, 1ul<<8);

    /*
     * put the PHY in reset mode.
     */
    write_phy(PHY_REG_BMCR, 0X8000);
    sleep(2);

    /*
     * wait for hardware reset to end.
     */
    for (i = 0; i < 0x100000; i++)
	{
        regv = read_phy(PHY_REG_BMCR);
        if (!(regv & 0x8000))
        {
            break;
        }
	}
    if (0x100000 <= i)
    {
        return OS_ERR_HARDWARE;
    }
    /*
     * 注册设备
     */
    irq_register(ENET_IRQn, 2, ethernet_exception);
    return OS_OK;
}

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

