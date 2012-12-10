/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            ethernet mac.c
** Last version:         V1.00
** Descriptions:         以太网控制器驱动文件.
** Hardware platform:    主控器件:lpc1768 & 物理收发器:KSZ8041
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
#define DEBUG_LOCAL_EN                              1                       /* 本地调试开关(默认配置): 0:关; 1:开   */
/*********************************************************************************************************************
                                                    头文件区
*********************************************************************************************************************/
#include    <include/macro.h>
#include    <include/types.h>
#include    <debug/debug.h>
#include    <library/ioport.h>
#include    <nvic/interrupt.h>
#include    <nxp/lpc1768/lpc17xx.h>

#include    "lwip/opt.h"
#include    "lwip/def.h"
#include    "lwip/mem.h"
#include    "lwip/pbuf.h"
#include    "lwip/sys.h"
#include    <lwip/stats.h>
#include    <lwip/snmp.h>
#include    "netif/etharp.h"
#include    "netif/ppp_oe.h"

#include    <string.h>
#include    <library/sysio/sysio.h>

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* MAC寄存器 -------------------------------------------------------------------------------------------------------*/
#define     MAC1                    (0x50000000)                            /* MAC配置寄存器1                       */
#define     MAC2                    (0x50000004)                            /* MAC配置寄存器2                       */
#define     IPGT                    (0x50000008)                            /* 连续两包的内部包间隙寄存器           */
#define     IPGR                    (0x5000000C)                            /* 非连续两包的内部包间隙寄存器         */

#define     CLRT                    (0x50000010)                            /* 冲突窗口/重试寄存器                  */
#define     MAXF                    (0x50000014)                            /* 最大帧寄存器                         */
#define     SUPP                    (0x50000018)                            /* PHY支持寄存器                        */
#define     TEST                    (0x5000001C)                            /* 测试寄存器                           */

#define     MCFG                    (0x50000020)                            /* MII Mgmt 配置寄存器                  */
#define     MCMD                    (0x50000024)                            /* MII Mgmt 命令寄存器                  */
#define     MADR                    (0x50000028)                            /* MII Mgmt 地址寄存器                  */
#define     MWTD                    (0x5000002C)                            /* MII Mgmt 写数据寄存器                */

#define     MRDD                    (0x50000030)                            /* MII Mgmt 读数据寄存器                */
#define     MIND                    (0x50000034)                            /* MII Mgmt 指示寄存器                  */

#define     SA0                     (0x50000040)                            /* 站地址0寄存器                        */
#define     SA1                     (0x50000044)                            /* 站地址1寄存器                        */
#define     SA2                     (0x50000048)                            /* 站地址2寄存器                        */

/* 控制寄存器 ------------------------------------------------------------------------------------------------------*/
#define     Command                 (0x50000100)                            /* 命令寄存器                           */
#define     Status                  (0x50000104)                            /* 状态寄存器                           */

#define     RxDescriptor            (0x50000108)                            /* 接收描述符基址寄存器                 */
#define     RxStatus                (0x5000010C)                            /* 接收状态基址寄存器                   */
#define     RxDescriptorNumber      (0x50000110)                            /* 接收描述符数目寄存器                 */
#define     RxProduceIndex          (0x50000114)                            /* 接收生产索引寄存器                   */
#define     RxConsumeIndex          (0x50000118)                            /* 接收消费索引寄存器                   */

#define     TxDescriptor            (0x5000011C)                            /* 发送描述符基址寄存器                 */
#define     TxStatus                (0x50000120)                            /* 发送状态基址寄存器                   */
#define     TxDescriptorNumber      (0x50000124)                            /* 发送描述符数目寄存器                 */
#define     TxProduceIndex          (0x50000128)                            /* 发送生产索引寄存器                   */
#define     TxConsumeIndex          (0x5000012C)                            /* 发送消费索引寄存器                   */

#define     TSV0                    (0x50000158)                            /* 发送状态向量0寄存器                  */
#define     TSV1                    (0x5000015C)                            /* 发送状态向量1寄存器                  */

#define     RSV                     (0x50000160)                            /* 接收状态向量寄存器                   */

#define     FlowControlCounter      (0x50000170)                            /* 流控制计数器寄存器                   */
#define     FlowControlStatus       (0x50000174)                            /* 流控制状态寄存器                     */

/* 接收过滤寄存器 --------------------------------------------------------------------------------------------------*/
#define     RxFilterCtrl            (0x50000200)                            /* 接收过滤器控制寄存器                 */
#define     RxFilterWoLStatus       (0x50000204)                            /* 接收过滤器WoL状态寄存器              */
#define     RxFilterWoLClear        (0x50000208)                            /* 接收过滤器WoL清零寄存器              */

#define     HashFilterL             (0x50000210)                            /* Hash过滤器表LSBs寄存器               */
#define     HashFilterH             (0x50000214)                            /* Hash过滤器表MSBs寄存器               */

/* 以太网模块控制寄存器 --------------------------------------------------------------------------------------------*/
#define     IntStatus               (0x50000FE0)                            /* 中断状态寄存器                       */
#define     IntEnable               (0x50000FE4)                            /* 中断使能寄存器                       */
#define     IntClear                (0x50000FE8)                            /* 中断禁能寄存器                       */
#define     IntSet                  (0x50000FEC)                            /* 中断置位寄存器                       */

#define     PowerDown               (0x50000FF4)                            /* 掉电寄存器                           */

/* 网络接口名称 ----------------------------------------------------------------------------------------------------*/
#define     IFNAME0                 'e'
#define     IFNAME1                 '0'

/* 宏操作 ----------------------------------------------------------------------------------------------------------*/
/* EMAC Memory Buffer configuration for 16K Ethernet RAM. */
#define     NUM_RX_FRAG             4                                       /* 接收段数目: 4*1536= 6.0kB            */
#define     NUM_TX_FRAG             3                                       /* 发送段数目: 3*1536= 4.6kB            */
#define     ETH_FRAG_SIZE           1536                                    /* 段缓存区大小: 1536 Bytes             */
#define     ETH_MAX_FLEN            1536                                    /* 最大的以太网帧大小                   */

/* EMAC variables located in 16K Ethernet SRAM */
#define     RX_DESC_BASE            0x2007C000                              /* 接收描述符数组基址                   */ 
#define     RX_STAT_BASE            (RX_DESC_BASE + NUM_RX_FRAG*8)          /* 接收状态数组基址                     */
#define     TX_DESC_BASE            (RX_STAT_BASE + NUM_RX_FRAG*8)          /* 发送描述符数组基址                   */
#define     TX_STAT_BASE            (TX_DESC_BASE + NUM_TX_FRAG*8)          /* 发送状态数组基址                     */
#define     RX_BUF_BASE             (TX_STAT_BASE + NUM_TX_FRAG*4)          /* 接收缓冲区数组基址                   */
#define     TX_BUF_BASE             (RX_BUF_BASE  + NUM_RX_FRAG*ETH_FRAG_SIZE) /* 发送缓冲区数组基址                */
/* 0x2007C000+4*8+4*8+3*8+3*4+4*1536+3*1536 = 0x2007C000+10852 = 0x2007EA64*/

/* RX and TX descriptor and status definitions. */
#define     RX_DESC_PACKET(i)       (*(INT32U *)(RX_DESC_BASE   + 8*i))     /* 第i个接收描述符--packet成员          */
#define     RX_DESC_CTRL(i)         (*(INT32U *)(RX_DESC_BASE+4 + 8*i))     /* 第i个接收描述符--control成员         */
#define     RX_STAT_INFO(i)         (*(INT32U *)(RX_STAT_BASE   + 8*i))     /* 第i个接收状态--StatusInfo成员        */
#define     RX_STAT_HASHCRC(i)      (*(INT32U *)(RX_STAT_BASE+4 + 8*i))     /* 第i个接收状态--HashCRC成员           */
#define     TX_DESC_PACKET(i)       (*(INT32U *)(TX_DESC_BASE   + 8*i))     /* 第i个发送描述符--packet成员          */
#define     TX_DESC_CTRL(i)         (*(INT32U *)(TX_DESC_BASE+4 + 8*i))     /* 第i个发送描述符--control成员         */
#define     TX_STAT_INFO(i)         (*(INT32U *)(TX_STAT_BASE   + 4*i))     /* 第i个发送状态--StatusInfo成员        */
#define     RX_BUF(i)               (RX_BUF_BASE + ETH_FRAG_SIZE*i)         /* 第i个接收缓冲区基址                  */
#define     TX_BUF(i)               (TX_BUF_BASE + ETH_FRAG_SIZE*i)         /* 第i个发送缓冲区基址                  */

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/
/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif
{
    struct eth_addr *ethaddr;
    /* Add whatever per-interface state that is needed here. */
};


/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/
static SEMAPHORE            Access;                                         /* 驱动访问信号量                       */
static SEMAPHORE            SendDone;                                       /* 发送完成信号量                       */
static SEMAPHORE            RecvDone;                                       /* 接收完成信号量                       */

/*********************************************************************************************************************
** Function name:           delay
** Descriptions:            短时间延时
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-1  17:22:59
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static void delay(unsigned int Time)
{
    int             i;

    for (i = 0; i < Time*256; i++)
    {
        ;
    }
}

/*********************************************************************************************************************
** Function name:           read_phy
** Descriptions:            读物理芯片寄存器
** Input parameters:        PhyAddress : 器件地址
**                          RegIndex   : 寄存器地址
** Output parameters:       pValue     : 待写入数据
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-1  13:8:34
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static int read_phy(INT32U PhyAddress, INT32U RegIndex, INT32U *pValue) 
{
    int          Timeout;


    write_dword(MADR, (PhyAddress<<8) | RegIndex);                          /* 设置PHY器件地址与寄存器地址          */
    write_dword(MCMD, 0x01);                                                /* 执行单次读命令                       */

    /* Wait until operation completed */
    for (Timeout = 0; Timeout < 50000; Timeout++)
    {
        if ((read_dword(MIND) & (1ul<<0)) == 0)                             /* MII Mgmt当前状态: 空闲               */
        {
            write_dword(MCMD, 0x00);                                        /* 停止单次读命令和连续读命令           */
            *pValue = read_dword(MRDD);
            return OS_OK;
        }
    }
    
    DBG_WARN("timeout in writing phy");
    return OS_ERR_HARDWARE;
}

/*********************************************************************************************************************
** Function name:           write_phy
** Descriptions:            写物理芯片寄存器
** Input parameters:        PhyAddress : 器件地址
**                          RegIndex   : 寄存器地址
**                          Value      : 待写入数据
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-1  12:47:22
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static int write_phy(INT32U PhyAddress, INT32U RegIndex, INT32U Value)
{
    int          Timeout;


    write_dword(MCMD, 0x00);                                                /* 停止单次读命令和连续读命令           */
    write_dword(MADR, (PhyAddress<<8) | RegIndex);                          /* 设置PHY器件地址与寄存器地址          */
    write_dword(MWTD, Value);                                               /* 设置待写数据                         */

    /* Wait utill operation completed */
    for (Timeout = 0; Timeout < 50000; Timeout++)
    {
        if ((read_dword(MIND) & (1ul<<0)) == 0)                             /* MII Mgmt当前状态: 空闲               */
        {
            return OS_OK;
        }
    }
    
    DBG_WARN("timeout in writing phy");
    return OS_ERR_HARDWARE;
}

/*********************************************************************************************************************
** Function name:           init_recv_description
** Descriptions:            初始化接收描述符
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-2  14:8:48
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void init_recv_description(void)
{
    int             i;

    for (i = 0; i < NUM_RX_FRAG; i++)
    {
        RX_DESC_PACKET(i)  = RX_BUF(i);                                     /* 初始化packet成员                     */
        RX_DESC_CTRL(i)    = (1ul << 31) |                                  /* 使能接收中断                         */
                             (ETH_FRAG_SIZE - 1);                           /* 初始化control成员                    */  
        RX_STAT_INFO(i)    = 0;                                             /* 初始化StatusInfo成员                 */  
        RX_STAT_HASHCRC(i) = 0;                                             /* 初始化HashCRC成员                    */  
    }
    write_dword(RxDescriptor, RX_DESC_BASE);                                /* 初始化描述符数组基址                 */
    write_dword(RxStatus,     RX_STAT_BASE);                                /* 初始化状态数组基址                   */
    write_dword(RxDescriptorNumber, NUM_RX_FRAG - 1);                       /* 初始化描述符数目                     */
    write_dword(RxConsumeIndex, 0);                                         /* 初始化消费索引: 0                    */
}

/*********************************************************************************************************************
** Function name:           init_sent_description
** Descriptions:            初始化接收描述符
** Input parameters:        
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-2  14:19:33
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
INLINE void init_sent_description(void)
{
    int             i;

    for (i = 0; i < NUM_TX_FRAG; i++)
    {
        TX_DESC_PACKET(i) = TX_BUF(i);
        TX_DESC_CTRL(i)   = (1ul << 31) |                                   /* 发送完成后产生中断                   */
                            (1ul << 30) |                                   /* 标识为最后一帧                       */
                            (1ul << 29) |                                   /* 硬件添加CRC到帧内                    */
                            (1ul << 28) |                                   /* 将短帧填充到64字节                   */
                            (1ul << 26) |                                   /* 采用bit[30~27]配置参数               */
                            (ETH_FRAG_SIZE - 1);                            /* 数据缓冲区的字节数                   */
        TX_STAT_INFO(i)   = 0;
    }
    
    /* Set EMAC Transmit Descriptor Registers. */
    write_dword(TxDescriptor, TX_DESC_BASE);
    write_dword(TxStatus, TX_STAT_BASE);
    write_dword(TxDescriptorNumber, NUM_TX_FRAG - 1);
    
    /* Tx Descriptors Point to 0 */
    write_dword(TxProduceIndex, 0);                                         /* 将接收生产索引指向0                  */
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
static err_t low_level_output(struct netif *netif, struct pbuf *pHead)
{
    struct pbuf *    pNode = pHead;
    int              ProduceIndex;
    int              ProduceNext;
    int              MaxIndex;
    char            *pBuffer;


    semaphore_wait(Access, 0);                                              /* 获取访问权限                         */    
    prints("output in\t");    
#if ETH_PAD_SIZE
    pbuf_header(pHead, -ETH_PAD_SIZE);                                      /* drop the padding word                */
#endif

    ProduceIndex = read_dword(TxProduceIndex);
    MaxIndex     = read_dword(TxDescriptorNumber);
    ProduceNext  = ProduceIndex + 1;
    if (MaxIndex < ProduceNext)
    {
        ProduceNext = 0;
    }
    
    while (ProduceNext == read_dword(TxConsumeIndex))                       /* 发送环形缓冲区满                     */
    {
        /* 等待发送环形缓冲区不满 */
        semaphore_wait(SendDone, 0);
    }
    
    for(pBuffer = (char *)TX_BUF(ProduceIndex); pNode != NULL; pNode = pNode->next)
    {
        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        memcpy(pBuffer, pNode->payload, pNode->len);
        pBuffer += pNode->len;
    }
    TX_DESC_CTRL(ProduceIndex) &= ~0x7FF;
    TX_DESC_CTRL(ProduceIndex) |= (pHead->tot_len - 1) & 0x7FF;
    
    write_dword(TxProduceIndex, ProduceNext);
#if ETH_PAD_SIZE
    pbuf_header(pHead, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    
    prints("output out\r\n");
    semaphore_post(Access);                                                 /* 释放访问权限                         */
    return ERR_OK;
}

/*********************************************************************************************************************
** Function name:           low_level_input
** Descriptions:            Should allocate a pbuf and transfer the bytes of the incoming packet from the interface
**                          into the pbuf. 
** Input parameters:        netif : 网络接口对象指针
**                         
** Output parameters:       
** Returned value:          != NULL : a pbuf filled with the received packet (including MAC header)
**                          == NULL : on memory error 
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-2-29  20:55:50
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static struct pbuf * low_level_input(struct netif *netif)
{
    struct pbuf     *pHead = NULL;
    int              ProduceIndex;                                          /* 生产描述符索引                       */
    int              ConsumeIndex;                                          /* 消费描述符索引                       */
    int              MaxIndex;


    semaphore_wait(Access, 0);                                              /* 获取访问权限                         */
    prints("scan in\t");

    ProduceIndex = read_dword(RxProduceIndex);
    ConsumeIndex = read_dword(RxConsumeIndex);
    if (ConsumeIndex != ProduceIndex)                                       /* 环形缓冲区队列有不为空               */
    {
        int         Size;

        Size = (RX_STAT_INFO(ConsumeIndex) & 0x7FFF) + 1;
        if (ETH_FRAG_SIZE < Size)
        {
            Size = ETH_FRAG_SIZE;
        }
#if ETH_PAD_SIZE
        Size += ETH_PAD_SIZE;
#endif        
        /* We allocate a pbuf chain of pbufs from the pool. */
        pHead = pbuf_alloc(PBUF_RAW, Size, PBUF_POOL);
        if (NULL != pHead)
        {
            struct pbuf     *pNode;
            const  char     *pBuffer;

#if ETH_PAD_SIZE
            pbuf_header(pHead, -ETH_PAD_SIZE);
#endif 
            pBuffer = (char *) RX_BUF(ConsumeIndex);
            /* We iterate over the pbuf chain until we have read the entire
             * packet into the pbuf. */
            for(pNode = pHead; pNode != NULL; pNode = pNode->next)
            {
                /* Read enough bytes to fill this pbuf in the chain. The
                 * available data in the pbuf is given by the q->len
                 * variable.
                 * This does not necessarily have to be a memcpy, you can also preallocate
                 * pbufs for a DMA-enabled MAC and after receiving truncate it to the
                 * actually received size. In this case, ensure the tot_len member of the
                 * pbuf is the sum of the chained pbuf len members.
                 */
                memcpy(pNode->payload, pBuffer, pNode->len);
                pBuffer += pNode->len;
            }
#if ETH_PAD_SIZE
            pbuf_header(pHead, ETH_PAD_SIZE); /* reclaim the padding word */
#endif            
        }
  
        MaxIndex = read_dword(RxDescriptorNumber);
        if (MaxIndex < ++ConsumeIndex)
        {
            ConsumeIndex = 0;
        }

        write_dword(RxConsumeIndex, ConsumeIndex);                          /* 更新接收缓冲区读索引                 */
    }
    else
    {
        write_dword(IntEnable, (1ul<<3) |                                   /* 使能接收描述符的中断                 */
                               (1ul<<7));                                   /* 使能发送描述符的中断                 */
    }
    prints("scan out\r\n");

    semaphore_post(Access);                                                 /* 释放访问权限                         */

    return pHead;
}

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
    INT32U                   InteruptStatus;                                /* 中断状态                             */


    InteruptStatus = read_dword(IntStatus);

    /*
     * 接收中断
     */
    if (InteruptStatus & (1ul<<3))                                          /* 接收中断                             */
    {
        semaphore_post(RecvDone);                                           /* 指示已接收数据帧                     */
    }
    else if (InteruptStatus & (1ul<<7))                                     /* 发送中断                             */
    {
        semaphore_post(SendDone);                                           /* 指示已发送数据帧                     */
    }

    /*
     * 
     */
    if (InteruptStatus & (1ul<<0))                                          /* 接收队列中出现重大的溢出错误         */
    {
        DBG_ERR("overrun InteruptStatus");
    }
    
    /*
     *
     */
    if (InteruptStatus & (1ul<<4))
    {
        DBG_ERR("underrun interupt");
    }
    write_dword(IntClear, InteruptStatus);                                  /* 清除中断状态标志                     */
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernet_scan(struct netif *netif)
{
    struct eth_hdr    *ethhdr;
    struct pbuf       *pFrame;                                              /* 数据帧                               */

    
    semaphore_wait(RecvDone, 0);                                            /* 等待接收到数据帧                     */
    
    pFrame = low_level_input(netif);
    if (NULL == pFrame)
    {
        return;
    }
    
    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = pFrame->payload;

    switch (htons(ethhdr->type))
    {
        /* IP or ARP packet? */
        case ETHTYPE_IP:                                                    /* IP数据包                             */
        case ETHTYPE_ARP:                                                   /* ARP数据包                            */
#if PPPOE_SUPPORT
        /* PPPoE packet? */
        case ETHTYPE_PPPOEDISC:
        case ETHTYPE_PPPOE:
#endif
            /* full packet send to tcpip_thread to process */
            if (netif->input(pFrame, netif)!=ERR_OK)
            {
                LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                pbuf_free(pFrame);
                pFrame = NULL;
            }
            break;

        default:
            pbuf_free(pFrame);
            pFrame = NULL;
            break;
    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernet_setup(struct netif *netif)
{
    INT32U              status;
    int                 i;  
    INT32U              PhyAddress, PhyID;
    struct ethernetif  *ethernetif;


    LWIP_ASSERT("netif != NULL", (netif != NULL));
    if (NULL == (Access = semaphore_new(1)))
    {
        DBG_WARN("创建驱动访问信号量时出错\r\n");
        return ERR_MEM;
    }
    
    if (NULL == (SendDone = semaphore_new(2)))
    {
        DBG_WARN("创建发送完成信号量时出错\r\n");
        return ERR_MEM;
    }

    if (NULL == (RecvDone = semaphore_new(0)))
    {
        DBG_WARN("创建接收完成信号量时出错\r\n");
        return ERR_MEM;
    }

    ethernetif = mem_malloc(sizeof(struct ethernetif));
    if (ethernetif == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        return ERR_MEM;
    }

    
#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
    * Initialize the snmp variables and counters inside the struct netif.
    * The last argument should be replaced with your link speed, in units
    * of bits per second.
    */
    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);
    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
    * You can instead declare your own function an call etharp_output()
    * from it if you have to do some checks before sending (e.g. if link
    * is available...) */
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;
    ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

    /* 
    * initialize the hardware
    */
    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    netif->hwaddr[0] = 0x00;
    netif->hwaddr[1] = 0x14;
    netif->hwaddr[2] = 0x97;
    netif->hwaddr[3] = 0x0F;
    netif->hwaddr[4] = 0xAF;
    netif->hwaddr[5] = 0x4C;

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

    /*
     * 网卡硬件初始化.
     */
    dword_set_bits(PCONP, 1ul << 30);                                       /* power up the EMAC controller         */

    /* 配置以太网相关引脚功能 */
    write_dword(PINSEL2, 0x50150105);                                       /* port1[0,1,4,8,9,10,14,15]            */
    dword_modify(PINSEL3, (1ul<<3) | (1ul<<1),
                          (1ul<<2) | (1ul<<0));                             /* port1[16,17]                         */

    /* 软件复位全部MAC内部模块. */
    write_dword(MAC1, (1ul<<15) |                                           /* 复位MAC内除主机接口以外的所有模块    */
                      (1ul<<14) |                                           /* 复位发送功能中的随机数发生器         */
                      (1ul<<11) |                                           /* 复位MAC控制子层/接收逻辑             */
                      (1ul<<10) |                                           /* 复位以太网接收逻辑                   */
                      (1ul<< 9) |                                           /* 复位MAC控制子层/发送逻辑             */
                      (1ul<< 8));                                           /* 复位发送功能的逻辑                   */
    write_dword(Command, (1ul<<5) |                                         /* 复位接收通道                         */
                         (1ul<<4) |                                         /* 复位发送通道                         */
                         (1ul<<3));                                         /* 复位所有的通道和主机寄存器           */
    
    
    /* A short delay after reset. */
    delay(100);     

    /*
     * 初始化MAC控制器寄存器.
     */
    write_dword(MAC1, (1ul<<1));                                            /* 使能传递所有类型帧                   */
    write_dword(MAC2, (1ul<<4) |                                            /* 使能在每帧上添加crc                  */
                      (1ul<<5));                                            /* 填充所有的短帧                       */
    write_dword(IPGR, 0x0C12);
    write_dword(CLRT, 0x370F);
    write_dword(MAXF, ETH_MAX_FLEN);                                        /* 最大帧size: 1536                     */
    
    
    /*
     * Fcpu = 100MHz, clock select=44, MDC=100/44=3MHz.
     */
    write_dword(MCFG, (1ul<<15) |                                           /* 复位MII管理硬件                      */
                      (0xA<<2));                                            /* 主机时钟44分频                       */
    delay(100);
    write_dword(MCFG, (0xA<<2));

    /*
     * 设置工作模式
     */
    write_dword(Command, (1ul<<6) |                                         /* 将小于64字节的短帧传递到存储器       */
                         (1ul<<7) |                                         /* 禁止接收过滤,接收所有帧并存入存储器  */
                         (1ul<<9));                                         /* 选择RMII模式                         */
    /*
     * 复位MII逻辑.
     */
    write_dword(SUPP, (1ul<<8) |
                      (1ul<<11));
    delay(100);
    write_dword(SUPP, 1ul<<8);

    /* 扫描phy器件地址 */
    for (i = 1; i < 32; i++)
    {
        read_phy(i, 3, &PhyID);
        read_phy(i, 2, &PhyID);
        if (0x0022 == PhyID)                                                /* KSZ8041器件ID: 0x0022                */
        {
            break;
        }
    }
    if (32 <= i)                                                            /* Phy器件有效地址范围: [1, 31]         */
    {
        return OS_ERR_HARDWARE;
    }
    PhyAddress = i;
    
    /*
     * 复位物理收发器,并使能启动自适应功能.
     */
    write_phy(PhyAddress, 0, (1ul<<15) |                                    /* software reset                       */
                             (1ul<<12) |                                    /* enable auto-negotiation process      */
                             (1ul<< 9));                                    /* restart auto-negotiation process     */
    sleep(2);
    do 
    {
        read_phy(PhyAddress, 0x1F, &status);
        status &= 0x1C;                                                     /* 提取适应信息                         */
    } while (0 == status);                                                  /* 等待自适应过程完成                   */

    switch (status)
    {
        case 0x04:                                                          /* 10Mbps & 半双工                      */
            write_dword(IPGT, 0x12);
            dword_clr_bits(MAC2, 1ul << 0);                                 /* 通信模式: 半双工模式                 */          
            dword_clr_bits(SUPP, 1ul<<8);                                   /* 工作速率: 10Mbps                     */
            dword_clr_bits(Command, 1ul << 10);                             /* 半双工模式                           */
            break;
            
        case 0x08:                                                          /* 100Mbps & 半双工                     */
            write_dword(IPGT, 0x12);
            dword_clr_bits(MAC2, 1ul << 0);                                 /* 通信模式: 半双工模式                 */
            dword_set_bits(SUPP, 1ul<<8);                                   /* 工作速率: 100Mbps                    */
            dword_clr_bits(Command, 1ul << 10);                             /* 半双工模式                           */
            break;
            
        case 0x14:                                                          /* 10Mbps & 全双工                      */
            write_dword(IPGT, 0x15);
            dword_set_bits(MAC2, 1ul << 0);                                 /* 通信模式: 全双工模式                 */            
            dword_clr_bits(SUPP, 1ul<<8);                                   /* 工作速率: 10Mbps                     */
            dword_set_bits(Command, 1ul << 10);                             /* 全双工模式                           */
            break;
            
        case 0x18:                                                          /* 100Mbps & 全双工                     */
            write_dword(IPGT, 0x15);
            dword_set_bits(MAC2, 1ul << 0);                                 /* 通信模式: 全双工模式                 */
            dword_set_bits(SUPP, 1ul<<8);                                   /* 工作速率: 100Mbps                    */
            dword_set_bits(Command, 1ul << 10);                             /* 全双工模式                           */
            break;

        default :
            return OS_ERR_HARDWARE;
    }

    /* 配置物理地址 */
    write_dword(SA0, (netif->hwaddr[0] << 8) + (netif->hwaddr[1] << 0));
    write_dword(SA1, (netif->hwaddr[2] << 8) + (netif->hwaddr[3] << 0));
    write_dword(SA2, (netif->hwaddr[4] << 8) + (netif->hwaddr[5] << 0));

    /*
     * 设置缓冲描述符.
     */
    init_recv_description();
    init_sent_description();
    
    write_dword(RxFilterCtrl, (1ul << 1) |                                  /* 接收所有广播帧                       */
                              (1ul << 5));                                  /* 接收目标地址与站地址相同的帧         */
    write_dword(IntClear, 0xFFFF);
    write_dword(IntEnable, (1ul << 3) |                                     /* 使能接收完成中断                     */
                           (1ul << 7));                                     /* 使能发送完成中断                     */
    dword_set_bits(Command, (1ul<<0)|                                       /* 使能接收通道                         */
                            (1ul<<1));                                      /* 使能发送通道                         */
    dword_set_bits(MAC1, (1ul << 0));                                       /* 允许对接收帧进行接收                 */
    
    /*
     * 注册设备
     */
    irq_register(ENET_IRQn, 20, ethernet_exception);
    
    DBG_INFO("以太网配置成功\r\n");
    return ERR_OK;
}

#if 0
/*********************************************************************************************************************
** Function name:           link
** Descriptions:            查看以太网接口连接状态
** Input parameters:        Option : 输入描述符
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-3-2  13:23:52
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
int link(char *Option)
{
    INT32U              LinkState;

    
    read_phy(5, 1, &LinkState);
    if (LinkState & (1ul << 2))
    {
        prints("已连接\r\n");
    }
    else
    {
        prints("未连接\r\n");
    }
    
    return OS_OK;
}
EXPORT_TO_CONSOLE("连接状态", link);
#endif

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

