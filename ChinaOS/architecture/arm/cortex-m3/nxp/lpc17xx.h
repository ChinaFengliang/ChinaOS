/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            lpc17xx.h
** Last version:         V1.00
** Descriptions:         寄存器地址定义文件.
** Hardware platform:    lpc17xx
** SoftWare platform:
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011年7月9日  11:30:29
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
#ifndef __LPC17XX_H
#define __LPC17XX_H

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
**  System control block 系统控制模块
*********************************************************************************************************************/
#define MEMMAP                  (0x400FC040)
#define EXTINT                  (0x400FC140)
#define EXTMODE                 (0x400FC148)
#define EXTPOLAR                (0x400FC14C)
#define RSID                    (0x400FC180)
#define SCS                     (0x400FC1A0)
#define CLKSRCSEL               (0x400FC10C)
#define PLL0CON                 (0x400FC080)
#define PLL0CFG                 (0x400FC084)
#define PLL0STAT                (0x400FC088)
#define PLL0FEED                (0x400FC08C)
#define PLL1CON                 (0x400FC0A0)
#define PLL1CFG                 (0x400FC0A4)
#define PLL1STAT                (0x400FC0A8)
#define PLL1FEED                (0x400FC0AC)
#define CCLKCFG                 (0x400FC104)
#define USBCLKCFG               (0x400FC108)
#define IRCTRIM                 (0x400FC1A4)
#define PCLKSEL0                (0x400FC1A8)
#define PCLKSEL1                (0x400FC1AC)
#define PCON                    (0x400FC0C0)
#define PCONP                   (0x400FC0C4)
#define CLKOUTCFG               (0x400FC1C8)

/*********************************************************************************************************************
**  Flash Accelerator Flash加速器
*********************************************************************************************************************/
#define FLASHCFG                (0x400FC000)

/*********************************************************************************************************************
**  NVIC嵌套向量中断控制器
*********************************************************************************************************************/
#define NVIC                    (0xE000E004)
#define STCTRL                  (0xE000E010)
#define STRELOAD                (0xE000E014)
#define STCURR                  (0xE000E018)
#define STCALIB                 (0xE000E01C)
#define SETENA0                 (0xE000E100)
#define SETENA1                 (0xE000E104)
#define CLRENA0                 (0xE000E180)
#define CLRENA1                 (0xE000E184)
#define SETPEND0                (0xE000E200)
#define SETPEND1                (0xE000E204)
#define CLRPEND0                (0xE000E280)
#define CLRPEND1                (0xE000E284)
#define ACTIVE0                 (0xE000E300)
#define ACTIVE1                 (0xE000E304)
#define IP0                     (0xE000E400)
#define IP1                     (0xE000E404)
#define IP2                     (0xE000E408)
#define IP3                     (0xE000E40C)
#define IP4                     (0xE000E410)
#define IP5                     (0xE000E414)
#define IP6                     (0xE000E418)
#define IP7                     (0xE000E41C)
#define IP8                     (0xE000E420)
#define IP9                     (0xE000E424)
#define IP10                    (0xE000E428)
#define IP11                    (0xE000E42C)
#define IP12                    (0xE000E430)
#define IP13                    (0xE000E434)
#define IP14                    (0xE000E438)
#define IP15                    (0xE000E43C)
#define CPUIDBR                 (0xE000ED00)
#define ICSR                    (0xE000ED04)
#define VTOR                    (0xE000ED08)
#define AITCR                   (0xE000ED0C)
#define SCR                     (0xE000ED10)
#define CCR                     (0xE000ED14)
#define SHPR0                   (0xE000ED18)
#define SHPR1                   (0xE000ED1C)
#define SHPR2                   (0xE000ED20)
#define SHCSR                   (0xE000ED24)
#define CFSR                    (0xE000ED28)
#define HFSR                    (0xE000ED2C)
#define DFSR                    (0xE000ED30)
#define MMFAR                   (0xE000ED34)
#define BFAR                    (0xE000ED38)
#define STIR                    (0xE000EF00)

/*********************************************************************************************************************
**  Pin connect block 引脚连接模块
*********************************************************************************************************************/
#define PINSEL0                 (0x4002C000)
#define PINSEL1                 (0x4002C004)
#define PINSEL2                 (0x4002C008)
#define PINSEL3                 (0x4002C00C)
#define PINSEL4                 (0x4002C010)
#define PINSEL5                 (0x4002C014)
#define PINSEL6                 (0x4002C018)
#define PINSEL7                 (0x4002C01C)
#define PINSEL8                 (0x4002C020)
#define PINSEL9                 (0x4002C024)
#define PINSEL10                (0x4002C028)
#define PINMODE0                (0x4002C040)
#define PINMODE1                (0x4002C044)
#define PINMODE2                (0x4002C048)
#define PINMODE3                (0x4002C04C)
#define PINMODE4                (0x4002C050)
#define PINMODE5                (0x4002C054)
#define PINMODE6                (0x4002C058)
#define PINMODE7                (0x4002C05C)
#define PINMODE8                (0x4002C060)
#define PINMODE9                (0x4002C064)
#define PINMODE_OD0             (0x4002C068)
#define PINMODE_OD1             (0x4002C06C)
#define PINMODE_OD2             (0x4002C070)
#define PINMODE_OD3             (0x4002C074)
#define PINMODE_OD4             (0x4002C078)
#define I2CPADCFG               (0x4002C07C)

/*********************************************************************************************************************
**  GPIO 通用输入输出端口
*********************************************************************************************************************/
#define FIO0DIR                 (0x2009c000)
#define FIO0DIR0                (0x2009c000)
#define FIO0DIR1                (0x2009c001)
#define FIO0DIR2                (0x2009c002)
#define FIO0DIR3                (0x2009c003)
#define FIO0DIRL                (0x2009c000)
#define FIO0DIRU                (0x2009c002)

#define FIO0MASK                (0x2009c010)
#define FIO0MASK0               (0x2009c010)
#define FIO0MASK1               (0x2009c011)
#define FIO0MASK2               (0x2009c012)
#define FIO0MASK3               (0x2009c013)
#define FIO0MASKL               (0x2009c010)
#define FIO0MASKU               (0x2009c012)

#define FIO0PIN                 (0x2009c014)
#define FIO0PIN0                (0x2009c014)
#define FIO0PIN1                (0x2009c015)
#define FIO0PIN2                (0x2009c016)
#define FIO0PIN3                (0x2009c017)
#define FIO0PINL                (0x2009c014)
#define FIO0PINU                (0x2009c016)

#define FIO0SET                 (0x2009c018)
#define FIO0SET0                (0x2009c018)
#define FIO0SET1                (0x2009c019)
#define FIO0SET2                (0x2009c01A)
#define FIO0SET3                (0x2009c01B)
#define FIO0SETL                (0x2009c018)
#define FIO0SETU                (0x2009c01A)

#define FIO0CLR                 (0x2009c01c)
#define FIO0CLR0                (0x2009c01c)
#define FIO0CLR1                (0x2009c01D)
#define FIO0CLR2                (0x2009c01E)
#define FIO0CLR3                (0x2009c01F)
#define FIO0CLRL                (0x2009c01c)
#define FIO0CLRU                (0x2009c01E)

#define FIO1DIR                 (0x2009c020)
#define FIO1DIR0                (0x2009c020)
#define FIO1DIR1                (0x2009c021)
#define FIO1DIR2                (0x2009c022)
#define FIO1DIR3                (0x2009c023)
#define FIO1DIRL                (0x2009c020)
#define FIO1DIRU                (0x2009c022)

#define FIO1MASK                (0x2009c030)
#define FIO1MASK0               (0x2009c030)
#define FIO1MASK1               (0x2009c031)
#define FIO1MASK2               (0x2009c032)
#define FIO1MASK3               (0x2009c033)
#define FIO1MASKL               (0x2009c030)
#define FIO1MASKU               (0x2009c032)

#define FIO1PIN                 (0x2009c034)
#define FIO1PIN0                (0x2009c034)
#define FIO1PIN1                (0x2009c035)
#define FIO1PIN2                (0x2009c036)
#define FIO1PIN3                (0x2009c037)
#define FIO1PINL                (0x2009c034)
#define FIO1PINU                (0x2009c036)

#define FIO1SET                 (0x2009c038)
#define FIO1SET0                (0x2009c038)
#define FIO1SET1                (0x2009c039)
#define FIO1SET2                (0x2009c03A)
#define FIO1SET3                (0x2009c03B)
#define FIO1SETL                (0x2009c038)
#define FIO1SETU                (0x2009c03A)

#define FIO1CLR                 (0x2009c03c)
#define FIO1CLR0                (0x2009c03c)
#define FIO1CLR1                (0x2009c03D)
#define FIO1CLR2                (0x2009c03E)
#define FIO1CLR3                (0x2009c03F)
#define FIO1CLRL                (0x2009c03c)
#define FIO1CLRU                (0x2009c03E)

#define FIO2DIR                 (0x2009c040)
#define FIO2DIR0                (0x2009c040)
#define FIO2DIR1                (0x2009c041)
#define FIO2DIR2                (0x2009c042)
#define FIO2DIR3                (0x2009c043)
#define FIO2DIRL                (0x2009c040)
#define FIO2DIRU                (0x2009c042)

#define FIO2MASK                (0x2009c050)
#define FIO2MASK0               (0x2009c050)
#define FIO2MASK1               (0x2009c051)
#define FIO2MASK2               (0x2009c052)
#define FIO2MASK3               (0x2009c053)
#define FIO2MASKL               (0x2009c050)
#define FIO2MASKU               (0x2009c052)

#define FIO2PIN                 (0x2009c054)
#define FIO2PIN0                (0x2009c054)
#define FIO2PIN1                (0x2009c055)
#define FIO2PIN2                (0x2009c056)
#define FIO2PIN3                (0x2009c057)
#define FIO2PINL                (0x2009c054)
#define FIO2PINU                (0x2009c056)

#define FIO2SET                 (0x2009c058)
#define FIO2SET0                (0x2009c058)
#define FIO2SET1                (0x2009c059)
#define FIO2SET2                (0x2009c05A)
#define FIO2SET3                (0x2009c05B)
#define FIO2SETL                (0x2009c058)
#define FIO2SETU                (0x2009c05A)

#define FIO2CLR                 (0x2009c05c)
#define FIO2CLR0                (0x2009c05c)
#define FIO2CLR1                (0x2009c05D)
#define FIO2CLR2                (0x2009c05E)
#define FIO2CLR3                (0x2009c05F)
#define FIO2CLRL                (0x2009c05c)
#define FIO2CLRU                (0x2009c05E)

#define FIO3DIR                 (0x2009c060)
#define FIO3DIR0                (0x2009c060)
#define FIO3DIR1                (0x2009c061)
#define FIO3DIR2                (0x2009c062)
#define FIO3DIR3                (0x2009c063)
#define FIO3DIRL                (0x2009c060)
#define FIO3DIRU                (0x2009c062)
                          
#define FIO3MASK                (0x2009c070)
#define FIO3MASK0               (0x2009c070)
#define FIO3MASK1               (0x2009c071)
#define FIO3MASK2               (0x2009c072)
#define FIO3MASK3               (0x2009c073)
#define FIO3MASKL               (0x2009c070)
#define FIO3MASKU               (0x2009c072)

#define FIO3PIN                 (0x2009c074)
#define FIO3PIN0                (0x2009c074)
#define FIO3PIN1                (0x2009c075)
#define FIO3PIN2                (0x2009c076)
#define FIO3PIN3                (0x2009c077)
#define FIO3PINL                (0x2009c074)
#define FIO3PINU                (0x2009c076)

#define FIO3SET                 (0x2009c078)
#define FIO3SET0                (0x2009c078)
#define FIO3SET1                (0x2009c079)
#define FIO3SET2                (0x2009c07A)
#define FIO3SET3                (0x2009c07B)
#define FIO3SETL                (0x2009c078)
#define FIO3SETU                (0x2009c07A)

#define FIO3CLR                 (0x2009c07c)
#define FIO3CLR0                (0x2009c07c)
#define FIO3CLR1                (0x2009c07D)
#define FIO3CLR2                (0x2009c07E)
#define FIO3CLR3                (0x2009c07F)
#define FIO3CLRL                (0x2009c07c)
#define FIO3CLRU                (0x2009c07E)

#define FIO4DIR                 (0x2009c080)
#define FIO4DIR0                (0x2009c080)
#define FIO4DIR1                (0x2009c081)
#define FIO4DIR2                (0x2009c082)
#define FIO4DIR3                (0x2009c083)
#define FIO4DIRL                (0x2009c080)
#define FIO4DIRU                (0x2009c082)

#define FIO4MASK                (0x2009c090)
#define FIO4MASK0               (0x2009c090)
#define FIO4MASK1               (0x2009c091)
#define FIO4MASK2               (0x2009c092)
#define FIO4MASK3               (0x2009c093)
#define FIO4MASKL               (0x2009c090)
#define FIO4MASKU               (0x2009c092)

#define FIO4PIN                 (0x2009c094)
#define FIO4PIN0                (0x2009c094)
#define FIO4PIN1                (0x2009c095)
#define FIO4PIN2                (0x2009c096)
#define FIO4PIN3                (0x2009c097)
#define FIO4PINL                (0x2009c094)
#define FIO4PINU                (0x2009c096)

#define FIO4SET                 (0x2009c098)
#define FIO4SET0                (0x2009c098)
#define FIO4SET1                (0x2009c099)
#define FIO4SET2                (0x2009c09A)
#define FIO4SET3                (0x2009c09B)
#define FIO4SETL                (0x2009c098)
#define FIO4SETU                (0x2009c09A)

#define FIO4CLR                 (0x2009c09c)
#define FIO4CLR0                (0x2009c09c)
#define FIO4CLR1                (0x2009c09D)
#define FIO4CLR2                (0x2009c09E)
#define FIO4CLR3                (0x2009c09F)
#define FIO4CLRL                (0x2009c09c)
#define FIO4CLRU                (0x2009c09E)

#define IO0IntEnr               (0x40028090)
#define IO0IntEnf               (0x40028094)
#define IO0IntStatr             (0x40028084)
#define IO0IntStatf             (0x40028088)
#define IO0IntClr               (0x4002808C)
#define IO2IntEnr               (0x400280B0)
#define IO2IntEnF               (0x400280B4)
#define IO2IntStatr             (0x400280A4)
#define IO2IntStatF             (0x400280A8)
#define IO2IntClr               (0x400280AC)
#define IOIntStatus             (0x40028080)

/*********************************************************************************************************************
**  ETHERNET
*********************************************************************************************************************/
#define MAC_MAC1                (0x50000000)
#define MAC_MAC2                (0x50000004)
#define MAC_IPGT                (0x50000008)
#define MAC_IPGR                (0x5000000C)
#define MAC_CLRT                (0x50000010)
#define MAC_MAXF                (0x50000014)
#define MAC_SUPP                (0x50000018)
#define MAC_TEST                (0x5000001C)
#define MAC_MCFG                (0x50000020)
#define MAC_MCMD                (0x50000024)
#define MAC_MADR                (0x50000028)
#define MAC_MWTD                (0x5000002C)
#define MAC_MRDD                (0x50000030)
#define MAC_MIND                (0x50000034)
#define MAC_SA0                 (0x50000040)
#define MAC_SA1                 (0x50000044)
#define MAC_SA2                 (0x50000048)
#define MAC_COMMAND             (0x50000100)
#define MAC_Status              (0x50000104)
#define MAC_RXDESCRIPTOR        (0x50000108)
#define MAC_RXSTATUS            (0x5000010C)
#define MAC_RXDESCRIPTORNUM     (0x50000110)
#define MAC_RXPRODUCEINDEX      (0x50000114)
#define MAC_RXCONSUMEINDEX      (0x50000118)
#define MAC_TXDESCRIPTOR        (0x5000011C)
#define MAC_TXSTATUS            (0x50000120)
#define MAC_TXDESCRIPTORNUM     (0x50000124)
#define MAC_TXPRODUCEINDEX      (0x50000128)
#define MAC_TXCONSUMEINDEX      (0x5000012C)
#define MAC_TSV0                (0x50000158)
#define MAC_TSV1                (0x5000015C)
#define MAC_RSV                 (0x50000160)
#define MAC_FlowControlCounter  (0x50000170)
#define MAC_FlowControlStatus   (0x50000174)
#define MAC_RXFILTERCTRL        (0x50000200)
#define MAC_RxFilterWoLStatus   (0x50000204)
#define MAC_RxFilterWoLClear    (0x50000208)
#define MAC_HashFilterL         (0x50000210)
#define MAC_HashFilterH         (0x50000214)
#define MAC_INTSTATUS           (0x50000FE0)
#define MAC_INTENABLE           (0x50000FE4)
#define MAC_INTCLEAR            (0x50000FE8)
#define MAC_IntSet              (0x50000FEC)
#define MAC_PowerDown           (0x50000FF4)

/*********************************************************************************************************************
**  USB
*********************************************************************************************************************/
#define USBClkCtrl              (0x5000CFF4)
#define USBClkSt                (0x5000CFF8)
#define USBIntSt                (0x400FC1C0)
#define USBDevIntSt             (0x5000C200)
#define USBDevIntEn             (0x5000C204)
#define USBDevIntClr            (0x5000C208)
#define USBDevIntSet            (0x5000C20C)
#define USBDevIntPri            (0x5000C22C)
#define USBEpIntSt              (0x5000C230)
#define USBEpIntEn              (0x5000C234)
#define USBEpIntClr             (0x5000C238)
#define USBEpIntSet             (0x5000C23C)
#define USBEpIntPri             (0x5000C240)
#define USBReEp                 (0x5000C244)
#define USBEpInd                (0x5000C248)
#define USBMaxPSize             (0x5000C24C)
#define USBRxData               (0x5000C218)
#define USBRxPLen               (0x5000C220)
#define USBTxData               (0x5000C21C)
#define USBTxPLen               (0x5000C224)
#define USBCtrl                 (0x5000C228)
#define USBCmdCode              (0x5000C210)
#define USBCmdData              (0x5000C214)
#define USBDMARSt               (0x5000C250)
#define USBDMARClr              (0x5000C254)
#define USBDMARSet              (0x5000C258)
#define USBUDCAH                (0x5000C280)
#define USBEpDMASt              (0x5000C284)
#define USBEpDMAEn              (0x5000C288)
#define USBEpDMADis             (0x5000C28C)
#define USBDMAIntSt             (0x5000C290)
#define USBDMAIntEn             (0x5000C294)
#define USBEoTIntSt             (0x5000C2A0)
#define USBEoTIntClr            (0x5000C2A4)
#define USBEoTIntSet            (0x5000C2A8)
#define USBNDDRIntSt            (0x5000C2AC)
#define USBNDDRIntClr           (0x5000C2B0)
#define USBNDDRIntSet           (0x5000C2B4)
#define USBSysErrIntSt          (0x5000C2B8)
#define USBSysErrIntClr         (0x5000C2BC)
#define USBSysErrIntSet         (0x5000C2C0)

/*********************************************************************************************************************
**  USB HOST (OHCI) CONTROLLER
*********************************************************************************************************************/
#define HcRevision              (0x5000C000)
#define HcControl               (0x5000C004)
#define HcCommandStatus         (0x5000C008)
#define HcInterruptStatus       (0x5000C00C)
#define HcInterruptEnable       (0x5000C010)
#define HcInterruptDisable      (0x5000C014)
#define HcHCCA                  (0x5000C018)
#define HcPeriodCurrentED       (0x5000C01C)
#define HcControlHeadED         (0x5000C020)
#define HcControlCurrentED      (0x5000C024)
#define HcBulkHeadED            (0x5000C028)
#define HcBulkCurrentED         (0x5000C02C)
#define HcDoneHead              (0x5000C030)
#define HcFmInterval            (0x5000C034)
#define HcFmRemaining           (0x5000C038)
#define HcFmNumber              (0x5000C03C)
#define HcPeriodicStart         (0x5000C040)
#define HcLSThreshold           (0x5000C044)
#define HcRhDescriptorA         (0x5000C048)
#define HcRhDescriptorB         (0x5000C04C)
#define HcRhStatus              (0x5000C050)
#define HcRhPortStatus1         (0x5000C054)
#define HcRhPortStatus2         (0x5000C058)
#define Module_ID               (0x5000C0FC)

/*********************************************************************************************************************
**  USB OTG Controller
*********************************************************************************************************************/
#define OTGIntSt                (0x5000C100)
#define OTGIntEn                (0x5000C104)
#define OTGIntSet               (0x5000C108)
#define OTGIntClr               (0x5000C10C)
#define OTGStCtrl               (0x5000C110)
#define OTGTmr                  (0x5000C114)
#define I2C_RX                  (0x5000C300)
#define I2C_TX                  I2C_RX
#define I2C_STS                 (0x5000C304)
#define I2C_CTL                 (0x5000C308)
#define I2C_CLKHI               (0x5000C30C)
#define I2C_CLKLO               (0x5000C310)
#define OTGClkCtrl              USBClkCtrl
#define OTGClkSt                USBClkSt

/*********************************************************************************************************************
**  UART0
*********************************************************************************************************************/
#define U0RBRTHR                (0x4000C000)                                /* U0DLL, U0RBR and U0THR               */
#define U0DLL                   U0RBRTHR                                    /* share the same address               */
#define U0RBR                   U0RBRTHR
#define U0THR                   U0RBRTHR

#define U0IER                   (0x4000C004)                                /* U0DLM and U0IER                      */
#define U0DLM                   U0IER                                       /* share the same address               */

#define U0FCR                   (0x4000C008)                                /* U0FCR and U0IIR                      */
#define U0IIR                   U0FCR                                       /* share the same address               */

#define U0LCR                   (0x4000C00C)
#define U0LSR                   (0x4000C014)
#define U0SCR                   (0x4000C01C)
#define U0ACR                   (0x4000C020)
#define U0FDR                   (0x4000C028)
#define U0TER                   (0x4000C030)
#define U0FIFOLVL               (0x4000C058)

/*********************************************************************************************************************
**  UART1
*********************************************************************************************************************/
#define U1RBRTHR                (0x40010000)                                /* U1DLL, U1RBR and U1THR               */
#define U1DLL                   U1RBRTHR                                    /* share the same address               */
#define U1RBR                   U1RBRTHR
#define U1THR                   U1RBRTHR

#define U1IER                   (0x40010004)                                /* U1DLM and U1IER                      */
#define U1DLM                   U1IER                                       /* share the same address               */


#define U1FCR                   (0x40010008)                                /* U1FCR and U1IIR                      */
#define U1IIR                   U1FCR                                       /* share the same address               */

#define U1LCR                   (0x4001000C)
#define U1MCR                   (0x40010010)
#define U1LSR                   (0x40010014)
#define U1MSR                   (0x40010018)
#define U1SCR                   (0x4001001C)
#define U1ACR                   (0x40010020)
#define U1FDR                   (0x40010028)
#define U1TER                   (0x40010030)
#define U1RS485CTRL             (0x4001004C)
#define U1ADRMATCH              (0x40010050)
#define U1RS485DLY              (0x40010054)
#define U1FIFOLVL               (0x40010058)

/*********************************************************************************************************************
**  UART2
*********************************************************************************************************************/
#define U2RBRTHR                (0x40098000)                                /* U2DLL, U2RBR and U2THR               */
#define U2DLL                   U2RBRTHR                                    /* share the same address               */
#define U2RBR                   U2RBRTHR
#define U2THR                   U2RBRTHR

#define U2IER                   (0x40098004)                                /* U2DLM and U2IER                      */
#define U2DLM                   U2IER                                       /* share the same address               */

#define U2FCR                   (0x40098008)                                /* U2FCR and U2IIR                      */
#define U2IIR                   U2FCR                                       /* share the same address               */

#define U2LCR                   (0x4009800C)
#define U2LSR                   (0x40098014)
#define U2SCR                   (0x4009801C)
#define U2ACR                   (0x40098020)
#define U2FDR                   (0x40098028)
#define U2TER                   (0x40098030)
#define U2FIFOLVL               (0x40098058)

/*********************************************************************************************************************
**  UART3
*********************************************************************************************************************/
#define U3RBRTHR                (0x4009C000)                                /* U3DLL, U3RBR and U3THR               */
#define U3DLL                   U3RBRTHR                                    /* share the same address               */
#define U3RBR                   U3RBRTHR
#define U3THR                   U3RBRTHR

#define U3IER                   (0x4009C004)                                /* U3DLM and U3IER                      */
#define U3DLM                   U3IER                                       /* share the same address               */
    
#define U3FCR                   (0x4009C008)                                /* U3FCR and U3IIR                      */
#define U3IIR                   U3FCR                                       /* share the same address               */

#define U3LCR                   (0x4009C00C)
#define U3LSR                   (0x4009C014)
#define U3SCR                   (0x4009C01C)
#define U3ACR                   (0x4009C020)
#define U3ICR                   (0x4009C024)
#define U3FDR                   (0x4009C028)
#define U3TER                   (0x4009C030)
#define U3FIFOLVL               (0x4009C058)

/*********************************************************************************************************************
**  CAN控制器
*********************************************************************************************************************/
#define AFMR                    (0x4003C000)
#define SFF_SA                  (0x4003C004)
#define SFF_GRP_SA              (0x4003C008)
#define EFF_SA                  (0x4003C00C)
#define EFF_GRP_SA              (0x4003C010)
#define ENDOFTABLE              (0x4003C014)
#define LUTERRAD                (0x4003C018)
#define LUTERR                  (0x4003C01C)
#define FCANIE                  (0x4003C020)
#define FCANIC0                 (0x4003C024)
#define FCANIC1                 (0x4003C028)
#define CANTXSR                 (0x40040000)
#define CANRXSR                 (0x40040004)
#define CANMSR                  (0x40040008)

#define CAN1MOD                 (0x40044000)
#define CAN1CMR                 (0x40044004)
#define CAN1GSR                 (0x40044008)
#define CAN1ICR                 (0x4004400C)
#define CAN1IER                 (0x40044010)
#define CAN1BTR                 (0x40044014)
#define CAN1EWL                 (0x40044018)
#define CAN1SR                  (0x4004401C)
#define CAN1RFS                 (0x40044020)
#define CAN1RID                 (0x40044024)
#define CAN1RDA                 (0x40044028)
#define CAN1RDB                 (0x4004402C)
#define CAN1TFI1                (0x40044030)
#define CAN1TID1                (0x40044034)
#define CAN1TDA1                (0x40044038)
#define CAN1TDB1                (0x4004403C)
#define CAN1TFI2                (0x40044040)
#define CAN1TID2                (0x40044044)
#define CAN1TDA2                (0x40044048)
#define CAN1TDB2                (0x4004404C)
#define CAN1TFI3                (0x40044050)
#define CAN1TID3                (0x40044054)
#define CAN1TDA3                (0x40044058)
#define CAN1TDB3                (0x4004405C)

#define CAN2MOD                 (0x40048000)
#define CAN2CMR                 (0x40048004)
#define CAN2GSR                 (0x40048008)
#define CAN2ICR                 (0x4004800C)
#define CAN2IER                 (0x40048010)
#define CAN2BTR                 (0x40048014)
#define CAN2EWL                 (0x40048018)
#define CAN2SR                  (0x4004801C)
#define CAN2RFS                 (0x40048020)
#define CAN2RID                 (0x40048024)
#define CAN2RDA                 (0x40048028)
#define CAN2RDB                 (0x4004802C)
#define CAN2TFI1                (0x40048030)
#define CAN2TID1                (0x40048034)
#define CAN2TDA1                (0x40048038)
#define CAN2TDB1                (0x4004803C)
#define CAN2TFI2                (0x40048040)
#define CAN2TID2                (0x40048044)
#define CAN2TDA2                (0x40048048)
#define CAN2TDB2                (0x4004804C)
#define CAN2TFI3                (0x40048050)
#define AN2TID3                 (0x40048054)
#define CAN2TDA3                (0x40048058)
#define CAN2TDB3                (0x4004805C)

/*********************************************************************************************************************
**  SPI
*********************************************************************************************************************/
#define S0SPCR                  (0x40020000)
#define S0SPSR                  (0x40020004)
#define S0SPDR                  (0x40020008)
#define S0SPCCR                 (0x4002000C)
#define SPTCR                   (0x40020010)
#define SPTSR                   (0x40020014)
#define S0SPINT                 (0x4002001C)

/*********************************************************************************************************************
**  SSP0
*********************************************************************************************************************/
#define SSP0CR0                 (0x40088000)
#define SSP0CR1                 (0x40088004)
#define SSP0DR                  (0x40088008)
#define SSP0SR                  (0x4008800C)
#define SSP0CPSR                (0x40088010)
#define SSP0IMSC                (0x40088014)
#define SSP0RIS                 (0x40088018)
#define SSP0MIS                 (0x4008801C)
#define SSP0ICR                 (0x40088020)
#define SSP0DMACR               (0x40088024)

/*********************************************************************************************************************
**  SSP1
*********************************************************************************************************************/
#define SSP1CR0                 (0x40030000)
#define SSP1CR1                 (0x40030004)
#define SSP1DR                  (0x40030008)
#define SSP1SR                  (0x4003000C)
#define SSP1CPSR                (0x40030010)
#define SSP1IMSC                (0x40030014)
#define SSP1RIS                 (0x40030018)
#define SSP1MIS                 (0x4003001C)
#define SSP1ICR                 (0x40030020)
#define SSP1DMACR               (0x40030024)

/*********************************************************************************************************************
**  I2C0
*********************************************************************************************************************/
#define I2C0CONSET              (0x4001C000)
#define I2C0STAT                (0x4001C004)
#define I2C0DAT                 (0x4001C008)
#define I2C0ADR                 (0x4001C00C)
#define I2C0SCLH                (0x4001C010)
#define I2C0SCLL                (0x4001C014)
#define I2C0CONCLR              (0x4001C018)
#define I2C0MMCTRL              (0x4001C01C)
#define I2C0ADR1                (0x4001C020)
#define I2C0ADR2                (0x4001C024)
#define I2C0ADR3                (0x4001C028)
#define I2C0DATABUFFER          (0x4001C02C)
#define I2C0MASK0               (0x4001C030)
#define I2C0MASK1               (0x4001C034)
#define I2C0MASK2               (0x4001C038)
#define I2C0MASK3               (0x4001C03C)

/*********************************************************************************************************************
**  I2C1
*********************************************************************************************************************/
#define I2C1CONSET              (0x4005C000)
#define I2C1STAT                (0x4005C004)
#define I2C1DAT                 (0x4005C008)
#define I2C1ADR                 (0x4005C00C)
#define I2C1SCLH                (0x4005C010)
#define I2C1SCLL                (0x4005C014)
#define I2C1CONCLR              (0x4005C018)
#define I2C1MMCTRL              (0x4005C01C)
#define I2C1ADR1                (0x4005C020)
#define I2C1ADR2                (0x4005C024)
#define I2C1ADR3                (0x4005C028)
#define I2C1DATABUFFER          (0x4005C02C)
#define I2C1MASK0               (0x4005C030)
#define I2C1MASK1               (0x4005C034)
#define I2C1MASK2               (0x4005C038)
#define I2C1MASK3               (0x4005C03C)

/*********************************************************************************************************************
**  I2C2
*********************************************************************************************************************/
#define I2C2CONSET              (0x400A0000)
#define I2C2STAT                (0x400A0004)
#define I2C2DAT                 (0x400A0008)
#define I2C2ADR                 (0x400A000C)
#define I2C2SCLH                (0x400A0010)
#define I2C2SCLL                (0x400A0014)
#define I2C2CONCLR              (0x400A0018)
#define I2C2MMCTRL              (0x400A001C)
#define I2C2ADR1                (0x400A0020)
#define I2C2ADR2                (0x400A0024)
#define I2C2ADR3                (0x400A0028)
#define I2C2DATABUFFER          (0x400A002C)
#define I2C2MASK0               (0x400A0030)
#define I2C2MASK1               (0x400A0034)
#define I2C2MASK2               (0x400A0038)
#define I2C2MASK3               (0x400A003C)

/*********************************************************************************************************************
**  I2S
*********************************************************************************************************************/
#define I2SDAO                  (0x400A8000)
#define I2SDAI                  (0x400A8004)
#define I2STXFIFO               (0x400A8008)
#define I2SRXFIFO               (0x400A800C)
#define I2SSTATE                (0x400A8010)
#define I2SDMA1                 (0x400A8014)
#define I2SDMA2                 (0x400A8018)
#define I2SIRQ                  (0x400A801C)
#define I2STXRATE               (0x400A8020)
#define I2SRXRATE               (0x400A8024)
#define I2STXBITRATE            (0x400A8028)
#define I2SRXBITRATE            (0x400A802C)
#define I2STXMODE               (0x400A8030)
#define I2SRXMODE               (0x400A8034)

/*********************************************************************************************************************
**  TIMER0
*********************************************************************************************************************/
#define T0IR                    (0x40004000)
#define T0TCR                   (0x40004004)
#define T0TC                    (0x40004008)
#define T0PR                    (0x4000400C)
#define T0PC                    (0x40004010)
#define T0MCR                   (0x40004014)
#define T0MR0                   (0x40004018)
#define T0MR1                   (0x4000401C)
#define T0MR2                   (0x40004020)
#define T0MR3                   (0x40004024)
#define T0CCR                   (0x40004028)
#define T0CR0                   (0x4000402C)
#define T0CR1                   (0x40004030)
#define T0EMR                   (0x4000403C)
#define T0CTCR                  (0x40004070)

/*********************************************************************************************************************
**  TIMER1
*********************************************************************************************************************/
#define T1IR                    (0x40008000)
#define T1TCR                   (0x40008004)
#define T1TC                    (0x40008008)
#define T1PR                    (0x4000800C)
#define T1PC                    (0x40008010)
#define T1MCR                   (0x40008014)
#define T1MR0                   (0x40008018)
#define T1MR1                   (0x4000801C)
#define T1MR2                   (0x40008020)
#define T1MR3                   (0x40008024)
#define T1CCR                   (0x40008028)
#define T1CR0                   (0x4000802C)
#define T1CR1                   (0x40008030)
#define T1EMR                   (0x4000803C)
#define T1CTCR                  (0x40008070)

/*********************************************************************************************************************
**  TIMER2
*********************************************************************************************************************/
#define T2IR                    (0x40090000)
#define T2TCR                   (0x40090004)
#define T2TC                    (0x40090008)
#define T2PR                    (0x4009000C)
#define T2PC                    (0x40090010)
#define T2MCR                   (0x40090014)
#define T2MR0                   (0x40090018)
#define T2MR1                   (0x4009001C)
#define T2MR2                   (0x40090020)
#define T2MR3                   (0x40090024)
#define T2CCR                   (0x40090028)
#define T2CR0                   (0x4009002C)
#define T2CR1                   (0x40090030)
#define T2EMR                   (0x4009003C)
#define T2CTCR                  (0x40090070)

/*********************************************************************************************************************
**  TIMER3
*********************************************************************************************************************/
#define T3IR                    (0x40094000)
#define T3TCR                   (0x40094004)
#define T3TC                    (0x40094008)
#define T3PR                    (0x4009400C)
#define T3PC                    (0x40094010)
#define T3MCR                   (0x40094014)
#define T3MR0                   (0x40094018)
#define T3MR1                   (0x4009401C)
#define T3MR2                   (0x40094020)
#define T3MR3                   (0x40094024)
#define T3CCR                   (0x40094028)
#define T3CR0                   (0x4009402C)
#define T3CR1                   (0x40094030)
#define T3EMR                   (0x4009403C)
#define T3CTCR                  (0x40094070)
                        
/*********************************************************************************************************************
**  Repetitive Interrupt Timer 可重复性中断控制器
*********************************************************************************************************************/
#define RICOMPVAL               (0x400B0000)
#define RIMASK                  (0x400B0004)
#define RICTRL                  (0x400B0008)
#define RICOUNTER               (0x400B000C)
                         
/*********************************************************************************************************************
**  PWM1
*********************************************************************************************************************/
#define PWM1IR                  (0x40018000)
#define PWM1TCR                 (0x40018004)
#define PWM1TC                  (0x40018008)
#define PWM1PR                  (0x4001800C)
#define PWM1PC                  (0x40018010)
#define PWM1MCR                 (0x40018014)
#define PWM1MR0                 (0x40018018)
#define PWM1MR1                 (0x4001801C)
#define PWM1MR2                 (0x40018020)
#define PWM1MR3                 (0x40018024)
#define PWM1CCR                 (0x40018028)
#define PWM1CR0                 (0x4001802C)
#define PWM1CR1                 (0x40018030)
#define PWM1MR4                 (0x40018040)
#define PWM1MR5                 (0x40018044)
#define PWM1MR6                 (0x40018048)
#define PWM1PCR                 (0x4001804C)
#define PWM1LER                 (0x40018050)
#define PWM1CTCR                (0x40018070)

/*********************************************************************************************************************
**  Motor control PWM
*********************************************************************************************************************/
#define MCCON                   (0x400B8000)
#define MCCON_SET               (0x400B8004)
#define MCCON_CLR               (0x400B8008)
#define MCCAPCON                (0x400B800C)
#define MCCAPCON_SET            (0x400B8010)
#define MCCAPCON_CLR            (0x400B8014)
#define MCTC0                   (0x400B8018)
#define MCTC1                   (0x400B801C)
#define MCTC2                   (0x400B8020)
#define MCLIM0                  (0x400B8024)
#define MCLIM1                  (0x400B8028)
#define MCLIM2                  (0x400B802C)
#define MCMAT0                  (0x400B8030)
#define MCMAT1                  (0x400B8034)
#define MCMAT2                  (0x400B8038)
#define MCDT                    (0x400B803C)
#define MCCP                    (0x400B8040)
#define MCCR0                   (0x400B8044)
#define MCCR1                   (0x400B8048)
#define CCR2                    (0x400B804C)
#define MCINTEN                 (0x400B8050)
#define MCINTEN_SET             (0x400B8054)
#define MCINTEN_CLR             (0x400B8058)
#define MCCNTCON                (0x400B805C)
#define MCCNTCON_SET            (0x400B8060)
#define MCCNTCON_CLR            (0x400B8064)
#define MCINTFLAG               (0x400B8068)
#define MCINTFLAG_SET           (0x400B806C)
#define MCINTFLAG_CLR           (0x400B8070)
#define MCCAP_CLR               (0x400B8074)

/*********************************************************************************************************************
**  Quadrature Encoder Interface
*********************************************************************************************************************/
#define QEICON                  (0x400BC000)
#define QEISTAT                 (0x400BC004)
#define QEICONF                 (0x400BC008)
#define QEIPOS                  (0x400BC00C)
#define QEIMAXPSOS              (0x400BC010)
#define CMPOS0                  (0x400BC014)
#define CMPOS1                  (0x400BC018)
#define CMPOS2                  (0x400BC01C)
#define INXCNT                  (0x400BC020)
#define INXCMP                  (0x400BC024)
#define QEILOAD                 (0x400BC028)
#define QEITIME                 (0x400BC02C)
#define QEIVEL                  (0x400BC030)
#define QEICAP                  (0x400BC034)
#define VELCOMP                 (0x400BC038)
#define FILTER                  (0x400BC03C)
#define QEIIES                  (0x400BCFDC)
#define QEIIEC                  (0x400BCFD8)
#define QEIINTSTAT              (0x400BCFE0)
#define QEIIE                   (0x400BCFE4)
#define QEICLR                  (0x400BCFE8)
#define QEISET                  (0x400BCFEC)

/*********************************************************************************************************************
**  RTC
*********************************************************************************************************************/
#define RTCILR                  (0x40024000)
#define RTCCCR                  (0x40024008)
#define RTCCIIR                 (0x4002400C)
#define RTCAMR                  (0x40024010)
#define RTCCTIME0               (0x40024014)
#define RTCCTIME1               (0x40024018)
#define RTCCTIME2               (0x4002401C)
#define RTCSEC                  (0x40024020)
#define RTCMIN                  (0x40024024)
#define RTCHOUR                 (0x40024028)
#define RTCDOM                  (0x4002402C)
#define RTCDOW                  (0x40024030)
#define RTCDOY                  (0x40024034)
#define RTCMONTH                (0x40024038)
#define RTCYEAR                 (0x4002403C)
#define RTCCALIBRATION          (0x40024040)
#define RTCGPREG0               (0x40024044)
#define RTCGPREG1               (0x40024048)
#define RTCGPREG2               (0x4002404C)
#define RTCGPREG3               (0x40024050)
#define RTCGPREG4               (0x40024054)
#define RTCALSEC                (0x40024060)
#define RTCALMIN                (0x40024064)
#define RTCALHOUR               (0x40024068)
#define RTCALDOM                (0x4002406C)
#define RTCALDOW                (0x40024070)
#define RTCALDOY                (0x40024074)
#define RTCALMON                (0x40024078)
#define RTCALYEAR               (0x4002407C)
#define RTC_AUX                 (0x4002405c)
#define RTC_AUXEN               (0x40024058)

/*********************************************************************************************************************
**  Watchdog
*********************************************************************************************************************/
#define WDMOD                   (0x40000000)
#define WDTC                    (0x40000004)
#define WDFEED                  (0x40000008)
#define WDTV                    (0x4000000C)
#define WDCLKSEL                (0x40000010)

/*********************************************************************************************************************
**  A/D Converters
*********************************************************************************************************************/
#define AD0CR                   (0x40034000)
#define AD0GDR                  (0x40034004)
#define ADINTEN                 (0x4003400C)
#define ADDR0                   (0x40034010)
#define ADDR1                   (0x40034014)
#define ADDR2                   (0x40034018)
#define ADDR3                   (0x4003401C)
#define ADDR4                   (0x40034020)
#define ADDR5                   (0x40034024)
#define ADDR6                   (0x40034028)
#define ADDR7                   (0x4003402C)
#define ADSTAT                  (0x40034030)
#define ADTRM                   (0x40034034)

/*********************************************************************************************************************
**  D/A Converter
*********************************************************************************************************************/
#define DACR                    (0x4008C000)
#define DACCTRL                 (0x4008C004)
#define DACCNTVAL               (0x4008C008)

/*********************************************************************************************************************
**  GPDMA
*********************************************************************************************************************/
#define DMACIntStat             (0x50004000)
#define DMACIntTCStat           (0x50004004)
#define DMACIntTCClear          (0x50004008)
#define DMACIntErrStat          (0x5000400C)
#define DMACIntErrClr           (0x50004010)
#define DMACRawIntTCStat        (0x50004014)
#define DMACRawIntErrStat       (0x50004018)
#define DMACEnbldChns           (0x5000401C)
#define DMACSoftBReq            (0x50004020)
#define DMACSoftSReq            (0x50004024)
#define DMACSoftLBReq           (0x50004028)
#define DMACSoftLSReq           (0x5000402C)
#define DMACConfig              (0x50004030)
#define DMACSync                (0x50004034)
#define DMACC0SrcAddr           (0x50004100)
#define DMACC0DestAddr          (0x50004104)
#define DMACC0LLI               (0x50004108)
#define DMACC0Control           (0x5000410C)
#define DMACC0Config            (0x50004110)
#define DMACC1SrcAddr           (0x50004120)
#define DMACC1DestAddr          (0x50004124)
#define DMACC1LLI               (0x50004128)
#define DMACC1Control           (0x5000412C)
#define DMACC1Config            (0x50004130)
#define DMACC2SrcAddr           (0x50004140)
#define DMACC2DestAddr          (0x50004144)
#define DMACC2LLI               (0x50004148)
#define DMACC2Control           (0x5000414C)
#define DMACC2Config            (0x50004150)
#define DMACC3SrcAddr           (0x50004160)
#define DMACC3DestAddr          (0x50004164)
#define MACC3LLI                (0x50004168)
#define DMACC3Control           (0x5000416C)
#define DMACC3Config            (0x50004170)
#define DMACC4SrcAddr           (0x50004180)
#define DMACC4DestAddr          (0x50004184)
#define DMACC4LLI               (0x50004188)
#define DMACC4Control           (0x5000418C)
#define DMACC4Config            (0x50004190)
#define DMACC5SrcAddr           (0x500041A0)
#define MACC5DestAddr           (0x500041A4)
#define DMACC5LLI               (0x500041A8)
#define DMACC5Control           (0x500041AC)
#define DMACC5Config            (0x500041B0)
#define DMACC6SrcAddr           (0x500041C0)
#define DMACC6DestAddr          (0x500041C4)
#define DMACC6LLI               (0x500041C8)
#define DMACC6Control           (0x500041CC)
#define DMACC6Config            (0x500041D0)
#define DMACC7SrcAddr           (0x500041E0)
#define DMACC7DestAddr          (0x500041E4)
#define DMACC7LLI               (0x500041E8)
#define DMACC7Control           (0x500041EC)
#define DMACC7Config            (0x500041F0)

/*********************************************************************************************************************
**  GPDMA Controller peripheral devices lines GPDMA 外围设备控制通道号
*********************************************************************************************************************/
#define GPDMA_SSP0TX            0                                           /* SPI0 Tx                              */
#define GPDMA_SSP0RX            1                                           /* SPI0 Rx                              */
#define GPDMA_SSP1TX            2                                           /* SPI1 Tx                              */
#define GPDMA_SSP1RX            3                                           /* SPI1 Rx                              */
#define GPDMA_ADC               4                                           /* ADC                                  */
#define GPDMA_I2S0              5                                           /* I2S Channel 0                        */
#define GPDMA_I2S1              6                                           /* I2S Channel 1                        */
#define GPDMA_DAC               7                                           /* ADC                                  */
#define GPDMA_U0TX_MAT0_0       8                                           /* UART 0 Tx/MAT0.0                     */
#define GPDMA_U0RX_MAT0_1       9                                           /* UART 0 Rx/MAT0.1                     */
#define GPDMA_U1TX_MAT1_0      10                                           /* UART 1 Tx/MAT1.0                     */
#define GPDMA_U1RX_MAT1_1      11                                           /* UART 1 Rx/MAT1.1                     */
#define GPDMA_U2TX_MAT2_0      12                                           /* UART 2 Tx/MAT2.0                     */
#define GPDMA_U2RX_MAT2_1      13                                           /* UART 2 Rx/MAT2.1                     */
#define GPDMA_U3TX_MAT3_0      14                                           /* UART 3 Tx/MAT3.0                     */
#define GPDMA_U3RX_MAT3_1      15                                           /* UART 3 Rx/MAT3.1                     */

/*********************************************************************************************************************
**  NVIC Interrupt channels  NVIC中断控制通道号
*********************************************************************************************************************/
#define MAIN_STACK             0                                            /* Main Stack                           */
#define RESETI                 1                                            /* Reset                                                          */
#define NMII                   2                                            /* Non-maskable Interrupt               */
#define HFI                    3                                            /* Hard Fault                                                     */
#define MMI                    4                                            /* Memory Management                    */
#define BFI                    5                                            /* Bus Fault                                                      */
#define UFI                    6                                            /* Usage Fault                          */
#define SVCI                  11                                            /* SVCall                               */
#define DMI                   12                                            /* Debug Monitor                        */
#define PSI                   14                                            /* PendSV                               */
#define STI                   15                                            /* SysTick                              */
#define NVIC_WDT              16                                            /* Watchdog Interrupt (WDINT)           */
#define NVIC_TIMER0           17                                            /* Match 0-1(MR0, MR1),                 */
                                                                            /* Capture 0-1(CR0, CR1)                */
#define NVIC_TIMER1           18                                            /* Match 0 - 2 (MR0, MR1, MR2),         */
                                                                            /* Capture 0 - 1 (CR0, CR1)             */
#define NVIC_TIMER2           19                                            /* Match 0-3, Capture 0-1               */
#define NVIC_TIMER3           20                                            /* Match 0-3, Capture 0-1               */
#define NVIC_UART0            21                                            /* UART0                                */
#define NVIC_UART1            22                                            /* UART1                                */
#define NVIC_UART2            23                                            /* UART2                                */
#define NVIC_UART3            24                                            /* UART3                                */
#define NVIC_PWM1             25                                            /* Match 0 - 6 of PWM1,                 */
                                                                            /* Capture 0-1 of PWM1                  */
#define NVIC_I2C0             26                                            /* SI (state change)                    */
#define NVIC_I2C1             27                                            /* SI (state change)                    */
#define NVIC_I2C2             28                                            /* SI (state change)                    */
#define NVIC_SPI              29                                            /* SPI Interrupt Flag (SPIF),           */
                                                                            /* Mode Fault (MODF)                    */
#define NVIC_SSP0             30                                            /* SSP0                                 */
#define NVIC_SSP1             31                                            /* SSP1                                 */
#define NVIC_PLL0             32                                            /* PLL0 Lock                            */
#define NVIC_RTC              33                                            /* Counter Increment (RTCCIF),          */
                                                                            /* Alarm (RTCALF)                       */
#define NVIC_EINT0            34                                            /* External Interrupt 0 (EINT0)         */
#define NVIC_EINT1            35                                            /* External Interrupt 1 (EINT1)         */
#define NVIC_EINT2            36                                            /* External Interrupt 2 (EINT2)         */
#define NVIC_EINT3            37                                            /* External Interrupt 3 (EINT3)         */
#define NVIC_ADC              38                                            /* A/D Converter                        */
#define NVIC_BOD              39                                            /* Brown Out detect                     */
#define NVIC_USB              40                                            /* USB                                  */
#define NVIC_CAN              41                                            /* CAN Common, CAN 0 Tx,                */
                                                                            /* CAN 0 Rx, CAN 1 Tx, CAN 1 Rx         */
#define NVIC_GP_DMA           42                                            /* IntStatus of DMA channel 0,          */
                                                                            /* IntStatus of DMA channel 1           */
#define NVIC_I2S              43                                            /* irq, dmareq1, dmareq2                */
#define NVIC_ETHR             44                                            /* Ethernet                             */
#define NVIC_RIT              45                                            /* Repetitive Interrupt Timer           */
#define NVIC_MC               46                                            /* Motor Control PWM                    */
#define NVIC_QE               47                                            /* Quadrature Encoder                   */
#define NVIC_PLL1             48                                            /* PLL1 Lock                            */

/*********************************************************************************************************************
**  NVIC Interrupt Priority   NVIC中断优先级
*********************************************************************************************************************/
#define PRIO_ZERO            (0x00)                                         /* 优先级为0                            */
#define PRIO_ONE             (0x01 << 3)                                    /* 优先级为1                            */
#define PRIO_TWO             (0x02 << 3)                                    /* 优先级为2                            */
#define PRIO_THREE           (0x03 << 3)                                    /* 优先级为3                            */
#define PRIO_FOUR            (0x04 << 3)                                    /* 优先级为4                            */
#define PRIO_FIVE            (0x05 << 3)                                    /* 优先级为5                            */
#define PRIO_SIX             (0x06 << 3)                                    /* 优先级为6                            */
#define PRIO_SEVEN           (0x07 << 3)                                    /* 优先级为7                            */
#define PRIO_EIGHT           (0x08 << 3)                                    /* 优先级为8                            */
#define PRIO_NINE            (0x09 << 3)                                    /* 优先级为9                            */
#define PRIO_TEN             (0x0A << 3)                                    /* 优先级为11                           */
#define PRIO_ELEVEN          (0x0B << 3)                                    /* 优先级为11                           */
#define PRIO_TWELVE          (0x0C << 3)                                    /* 优先级为12                           */
#define PRIO_THIRTEEN        (0x0D << 3)                                    /* 优先级为13                           */
#define PRIO_FOURTEEN        (0x0E << 3)                                    /* 优先级为14                           */
#define PRIO_FIFTEEN         (0x0F << 3)                                    /* 优先级为15                           */
#define PRIO_SIXTEEN         (0x10 << 3)                                    /* 优先级为16                           */
#define PRIO_SEVENTEEN       (0x11 << 3)                                    /* 优先级为17                           */
#define PRIO_EIGHTEEN        (0x12 << 3)                                    /* 优先级为18                           */
#define PRIO_NINETEEN        (0x13 << 3)                                    /* 优先级为19                           */
#define PRIO_TWENTY          (0x14 << 3)                                    /* 优先级为20                           */
#define PRIO_TWENTY_ONE      (0x15 << 3)                                    /* 优先级为21                           */
#define PRIO_TWENTY_TWO      (0x16 << 3)                                    /* 优先级为22                           */
#define PRIO_TWENTY_THREE    (0x17 << 3)                                    /* 优先级为23                           */
#define PRIO_TWENTY_FOUR     (0x18 << 3)                                    /* 优先级为24                           */
#define PRIO_TWENTY_FIVE     (0x19 << 3)                                    /* 优先级为25                           */
#define PRIO_TWENTY_SIX      (0x1A << 3)                                    /* 优先级为26                           */
#define PRIO_TWENTY_SEVEV    (0x1B << 3)                                    /* 优先级为27                           */
#define PRIO_TWENTY_EIGHT    (0x1C << 3)                                    /* 优先级为28                           */
#define PRIO_TWENTY_NINE     (0x1D << 3)                                    /* 优先级为29                           */
#define PRIO_THIRTY          (0x1E << 3)                                    /* 优先级为30                           */
#define PRIO_THIRTY_ONE      (0x1F << 3)                                    /* 优先级为31                           */


#endif                                                                      /* end of __LPC17XX_H                   */
/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

