/******************************************************Copyright (c)**************************************************
**                                              ��������ֹһ�п��ܵĸ���Ե��
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            ethernet mac.c
** Last version:         V1.00
** Descriptions:         ��̫�������������ļ�.
** Hardware platform:    ��������:lpc1768 & �����շ���:KSZ8041
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2012��2��7��  22:50:46
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
#define DEBUG_LOCAL_EN                              1                       /* ���ص��Կ���(Ĭ������): 0:��; 1:��   */
/*********************************************************************************************************************
                                                    ͷ�ļ���
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
                                                    �궨����
*********************************************************************************************************************/
/* MAC�Ĵ��� -------------------------------------------------------------------------------------------------------*/
#define     MAC1                    (0x50000000)                            /* MAC���üĴ���1                       */
#define     MAC2                    (0x50000004)                            /* MAC���üĴ���2                       */
#define     IPGT                    (0x50000008)                            /* �����������ڲ�����϶�Ĵ���           */
#define     IPGR                    (0x5000000C)                            /* �������������ڲ�����϶�Ĵ���         */

#define     CLRT                    (0x50000010)                            /* ��ͻ����/���ԼĴ���                  */
#define     MAXF                    (0x50000014)                            /* ���֡�Ĵ���                         */
#define     SUPP                    (0x50000018)                            /* PHY֧�ּĴ���                        */
#define     TEST                    (0x5000001C)                            /* ���ԼĴ���                           */

#define     MCFG                    (0x50000020)                            /* MII Mgmt ���üĴ���                  */
#define     MCMD                    (0x50000024)                            /* MII Mgmt ����Ĵ���                  */
#define     MADR                    (0x50000028)                            /* MII Mgmt ��ַ�Ĵ���                  */
#define     MWTD                    (0x5000002C)                            /* MII Mgmt д���ݼĴ���                */

#define     MRDD                    (0x50000030)                            /* MII Mgmt �����ݼĴ���                */
#define     MIND                    (0x50000034)                            /* MII Mgmt ָʾ�Ĵ���                  */

#define     SA0                     (0x50000040)                            /* վ��ַ0�Ĵ���                        */
#define     SA1                     (0x50000044)                            /* վ��ַ1�Ĵ���                        */
#define     SA2                     (0x50000048)                            /* վ��ַ2�Ĵ���                        */

/* ���ƼĴ��� ------------------------------------------------------------------------------------------------------*/
#define     Command                 (0x50000100)                            /* ����Ĵ���                           */
#define     Status                  (0x50000104)                            /* ״̬�Ĵ���                           */

#define     RxDescriptor            (0x50000108)                            /* ������������ַ�Ĵ���                 */
#define     RxStatus                (0x5000010C)                            /* ����״̬��ַ�Ĵ���                   */
#define     RxDescriptorNumber      (0x50000110)                            /* ������������Ŀ�Ĵ���                 */
#define     RxProduceIndex          (0x50000114)                            /* �������������Ĵ���                   */
#define     RxConsumeIndex          (0x50000118)                            /* �������������Ĵ���                   */

#define     TxDescriptor            (0x5000011C)                            /* ������������ַ�Ĵ���                 */
#define     TxStatus                (0x50000120)                            /* ����״̬��ַ�Ĵ���                   */
#define     TxDescriptorNumber      (0x50000124)                            /* ������������Ŀ�Ĵ���                 */
#define     TxProduceIndex          (0x50000128)                            /* �������������Ĵ���                   */
#define     TxConsumeIndex          (0x5000012C)                            /* �������������Ĵ���                   */

#define     TSV0                    (0x50000158)                            /* ����״̬����0�Ĵ���                  */
#define     TSV1                    (0x5000015C)                            /* ����״̬����1�Ĵ���                  */

#define     RSV                     (0x50000160)                            /* ����״̬�����Ĵ���                   */

#define     FlowControlCounter      (0x50000170)                            /* �����Ƽ������Ĵ���                   */
#define     FlowControlStatus       (0x50000174)                            /* ������״̬�Ĵ���                     */

/* ���չ��˼Ĵ��� --------------------------------------------------------------------------------------------------*/
#define     RxFilterCtrl            (0x50000200)                            /* ���չ��������ƼĴ���                 */
#define     RxFilterWoLStatus       (0x50000204)                            /* ���չ�����WoL״̬�Ĵ���              */
#define     RxFilterWoLClear        (0x50000208)                            /* ���չ�����WoL����Ĵ���              */

#define     HashFilterL             (0x50000210)                            /* Hash��������LSBs�Ĵ���               */
#define     HashFilterH             (0x50000214)                            /* Hash��������MSBs�Ĵ���               */

/* ��̫��ģ����ƼĴ��� --------------------------------------------------------------------------------------------*/
#define     IntStatus               (0x50000FE0)                            /* �ж�״̬�Ĵ���                       */
#define     IntEnable               (0x50000FE4)                            /* �ж�ʹ�ܼĴ���                       */
#define     IntClear                (0x50000FE8)                            /* �жϽ��ܼĴ���                       */
#define     IntSet                  (0x50000FEC)                            /* �ж���λ�Ĵ���                       */

#define     PowerDown               (0x50000FF4)                            /* ����Ĵ���                           */

/* ����ӿ����� ----------------------------------------------------------------------------------------------------*/
#define     IFNAME0                 'e'
#define     IFNAME1                 '0'

/* ����� ----------------------------------------------------------------------------------------------------------*/
/* EMAC Memory Buffer configuration for 16K Ethernet RAM. */
#define     NUM_RX_FRAG             4                                       /* ���ն���Ŀ: 4*1536= 6.0kB            */
#define     NUM_TX_FRAG             3                                       /* ���Ͷ���Ŀ: 3*1536= 4.6kB            */
#define     ETH_FRAG_SIZE           1536                                    /* �λ�������С: 1536 Bytes             */
#define     ETH_MAX_FLEN            1536                                    /* ������̫��֡��С                   */

/* EMAC variables located in 16K Ethernet SRAM */
#define     RX_DESC_BASE            0x2007C000                              /* ���������������ַ                   */ 
#define     RX_STAT_BASE            (RX_DESC_BASE + NUM_RX_FRAG*8)          /* ����״̬�����ַ                     */
#define     TX_DESC_BASE            (RX_STAT_BASE + NUM_RX_FRAG*8)          /* ���������������ַ                   */
#define     TX_STAT_BASE            (TX_DESC_BASE + NUM_TX_FRAG*8)          /* ����״̬�����ַ                     */
#define     RX_BUF_BASE             (TX_STAT_BASE + NUM_TX_FRAG*4)          /* ���ջ����������ַ                   */
#define     TX_BUF_BASE             (RX_BUF_BASE  + NUM_RX_FRAG*ETH_FRAG_SIZE) /* ���ͻ����������ַ                */
/* 0x2007C000+4*8+4*8+3*8+3*4+4*1536+3*1536 = 0x2007C000+10852 = 0x2007EA64*/

/* RX and TX descriptor and status definitions. */
#define     RX_DESC_PACKET(i)       (*(INT32U *)(RX_DESC_BASE   + 8*i))     /* ��i������������--packet��Ա          */
#define     RX_DESC_CTRL(i)         (*(INT32U *)(RX_DESC_BASE+4 + 8*i))     /* ��i������������--control��Ա         */
#define     RX_STAT_INFO(i)         (*(INT32U *)(RX_STAT_BASE   + 8*i))     /* ��i������״̬--StatusInfo��Ա        */
#define     RX_STAT_HASHCRC(i)      (*(INT32U *)(RX_STAT_BASE+4 + 8*i))     /* ��i������״̬--HashCRC��Ա           */
#define     TX_DESC_PACKET(i)       (*(INT32U *)(TX_DESC_BASE   + 8*i))     /* ��i������������--packet��Ա          */
#define     TX_DESC_CTRL(i)         (*(INT32U *)(TX_DESC_BASE+4 + 8*i))     /* ��i������������--control��Ա         */
#define     TX_STAT_INFO(i)         (*(INT32U *)(TX_STAT_BASE   + 4*i))     /* ��i������״̬--StatusInfo��Ա        */
#define     RX_BUF(i)               (RX_BUF_BASE + ETH_FRAG_SIZE*i)         /* ��i�����ջ�������ַ                  */
#define     TX_BUF(i)               (TX_BUF_BASE + ETH_FRAG_SIZE*i)         /* ��i�����ͻ�������ַ                  */

/*********************************************************************************************************************
                                                    ���Ͷ�����
*********************************************************************************************************************/
/* �������� --------------------------------------------------------------------------------------------------------*/
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
                                                  ȫ�ֱ���������
*********************************************************************************************************************/
static SEMAPHORE            Access;                                         /* ���������ź���                       */
static SEMAPHORE            SendDone;                                       /* ��������ź���                       */
static SEMAPHORE            RecvDone;                                       /* ��������ź���                       */

/*********************************************************************************************************************
** Function name:           delay
** Descriptions:            ��ʱ����ʱ
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
** Descriptions:            ������оƬ�Ĵ���
** Input parameters:        PhyAddress : ������ַ
**                          RegIndex   : �Ĵ�����ַ
** Output parameters:       pValue     : ��д������
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
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


    write_dword(MADR, (PhyAddress<<8) | RegIndex);                          /* ����PHY������ַ��Ĵ�����ַ          */
    write_dword(MCMD, 0x01);                                                /* ִ�е��ζ�����                       */

    /* Wait until operation completed */
    for (Timeout = 0; Timeout < 50000; Timeout++)
    {
        if ((read_dword(MIND) & (1ul<<0)) == 0)                             /* MII Mgmt��ǰ״̬: ����               */
        {
            write_dword(MCMD, 0x00);                                        /* ֹͣ���ζ����������������           */
            *pValue = read_dword(MRDD);
            return OS_OK;
        }
    }
    
    DBG_WARN("timeout in writing phy");
    return OS_ERR_HARDWARE;
}

/*********************************************************************************************************************
** Function name:           write_phy
** Descriptions:            д����оƬ�Ĵ���
** Input parameters:        PhyAddress : ������ַ
**                          RegIndex   : �Ĵ�����ַ
**                          Value      : ��д������
** Output parameters:       
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
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


    write_dword(MCMD, 0x00);                                                /* ֹͣ���ζ����������������           */
    write_dword(MADR, (PhyAddress<<8) | RegIndex);                          /* ����PHY������ַ��Ĵ�����ַ          */
    write_dword(MWTD, Value);                                               /* ���ô�д����                         */

    /* Wait utill operation completed */
    for (Timeout = 0; Timeout < 50000; Timeout++)
    {
        if ((read_dword(MIND) & (1ul<<0)) == 0)                             /* MII Mgmt��ǰ״̬: ����               */
        {
            return OS_OK;
        }
    }
    
    DBG_WARN("timeout in writing phy");
    return OS_ERR_HARDWARE;
}

/*********************************************************************************************************************
** Function name:           init_recv_description
** Descriptions:            ��ʼ������������
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
        RX_DESC_PACKET(i)  = RX_BUF(i);                                     /* ��ʼ��packet��Ա                     */
        RX_DESC_CTRL(i)    = (1ul << 31) |                                  /* ʹ�ܽ����ж�                         */
                             (ETH_FRAG_SIZE - 1);                           /* ��ʼ��control��Ա                    */  
        RX_STAT_INFO(i)    = 0;                                             /* ��ʼ��StatusInfo��Ա                 */  
        RX_STAT_HASHCRC(i) = 0;                                             /* ��ʼ��HashCRC��Ա                    */  
    }
    write_dword(RxDescriptor, RX_DESC_BASE);                                /* ��ʼ�������������ַ                 */
    write_dword(RxStatus,     RX_STAT_BASE);                                /* ��ʼ��״̬�����ַ                   */
    write_dword(RxDescriptorNumber, NUM_RX_FRAG - 1);                       /* ��ʼ����������Ŀ                     */
    write_dword(RxConsumeIndex, 0);                                         /* ��ʼ����������: 0                    */
}

/*********************************************************************************************************************
** Function name:           init_sent_description
** Descriptions:            ��ʼ������������
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
        TX_DESC_CTRL(i)   = (1ul << 31) |                                   /* ������ɺ�����ж�                   */
                            (1ul << 30) |                                   /* ��ʶΪ���һ֡                       */
                            (1ul << 29) |                                   /* Ӳ�����CRC��֡��                    */
                            (1ul << 28) |                                   /* ����֡��䵽64�ֽ�                   */
                            (1ul << 26) |                                   /* ����bit[30~27]���ò���               */
                            (ETH_FRAG_SIZE - 1);                            /* ���ݻ��������ֽ���                   */
        TX_STAT_INFO(i)   = 0;
    }
    
    /* Set EMAC Transmit Descriptor Registers. */
    write_dword(TxDescriptor, TX_DESC_BASE);
    write_dword(TxStatus, TX_STAT_BASE);
    write_dword(TxDescriptorNumber, NUM_TX_FRAG - 1);
    
    /* Tx Descriptors Point to 0 */
    write_dword(TxProduceIndex, 0);                                         /* ��������������ָ��0                  */
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


    semaphore_wait(Access, 0);                                              /* ��ȡ����Ȩ��                         */    
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
    
    while (ProduceNext == read_dword(TxConsumeIndex))                       /* ���ͻ��λ�������                     */
    {
        /* �ȴ����ͻ��λ��������� */
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
    semaphore_post(Access);                                                 /* �ͷŷ���Ȩ��                         */
    return ERR_OK;
}

/*********************************************************************************************************************
** Function name:           low_level_input
** Descriptions:            Should allocate a pbuf and transfer the bytes of the incoming packet from the interface
**                          into the pbuf. 
** Input parameters:        netif : ����ӿڶ���ָ��
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
    int              ProduceIndex;                                          /* ��������������                       */
    int              ConsumeIndex;                                          /* ��������������                       */
    int              MaxIndex;


    semaphore_wait(Access, 0);                                              /* ��ȡ����Ȩ��                         */
    prints("scan in\t");

    ProduceIndex = read_dword(RxProduceIndex);
    ConsumeIndex = read_dword(RxConsumeIndex);
    if (ConsumeIndex != ProduceIndex)                                       /* ���λ����������в�Ϊ��               */
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

        write_dword(RxConsumeIndex, ConsumeIndex);                          /* ���½��ջ�����������                 */
    }
    else
    {
        write_dword(IntEnable, (1ul<<3) |                                   /* ʹ�ܽ������������ж�                 */
                               (1ul<<7));                                   /* ʹ�ܷ������������ж�                 */
    }
    prints("scan out\r\n");

    semaphore_post(Access);                                                 /* �ͷŷ���Ȩ��                         */

    return pHead;
}

/*********************************************************************************************************************
** Function name:           ethernet_exception
** Descriptions:            ��̫���������жϷ���
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
    INT32U                   InteruptStatus;                                /* �ж�״̬                             */


    InteruptStatus = read_dword(IntStatus);

    /*
     * �����ж�
     */
    if (InteruptStatus & (1ul<<3))                                          /* �����ж�                             */
    {
        semaphore_post(RecvDone);                                           /* ָʾ�ѽ�������֡                     */
    }
    else if (InteruptStatus & (1ul<<7))                                     /* �����ж�                             */
    {
        semaphore_post(SendDone);                                           /* ָʾ�ѷ�������֡                     */
    }

    /*
     * 
     */
    if (InteruptStatus & (1ul<<0))                                          /* ���ն����г����ش���������         */
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
    write_dword(IntClear, InteruptStatus);                                  /* ����ж�״̬��־                     */
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
    struct pbuf       *pFrame;                                              /* ����֡                               */

    
    semaphore_wait(RecvDone, 0);                                            /* �ȴ����յ�����֡                     */
    
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
        case ETHTYPE_IP:                                                    /* IP���ݰ�                             */
        case ETHTYPE_ARP:                                                   /* ARP���ݰ�                            */
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
        DBG_WARN("�������������ź���ʱ����\r\n");
        return ERR_MEM;
    }
    
    if (NULL == (SendDone = semaphore_new(2)))
    {
        DBG_WARN("������������ź���ʱ����\r\n");
        return ERR_MEM;
    }

    if (NULL == (RecvDone = semaphore_new(0)))
    {
        DBG_WARN("������������ź���ʱ����\r\n");
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
     * ����Ӳ����ʼ��.
     */
    dword_set_bits(PCONP, 1ul << 30);                                       /* power up the EMAC controller         */

    /* ������̫��������Ź��� */
    write_dword(PINSEL2, 0x50150105);                                       /* port1[0,1,4,8,9,10,14,15]            */
    dword_modify(PINSEL3, (1ul<<3) | (1ul<<1),
                          (1ul<<2) | (1ul<<0));                             /* port1[16,17]                         */

    /* �����λȫ��MAC�ڲ�ģ��. */
    write_dword(MAC1, (1ul<<15) |                                           /* ��λMAC�ڳ������ӿ����������ģ��    */
                      (1ul<<14) |                                           /* ��λ���͹����е������������         */
                      (1ul<<11) |                                           /* ��λMAC�����Ӳ�/�����߼�             */
                      (1ul<<10) |                                           /* ��λ��̫�������߼�                   */
                      (1ul<< 9) |                                           /* ��λMAC�����Ӳ�/�����߼�             */
                      (1ul<< 8));                                           /* ��λ���͹��ܵ��߼�                   */
    write_dword(Command, (1ul<<5) |                                         /* ��λ����ͨ��                         */
                         (1ul<<4) |                                         /* ��λ����ͨ��                         */
                         (1ul<<3));                                         /* ��λ���е�ͨ���������Ĵ���           */
    
    
    /* A short delay after reset. */
    delay(100);     

    /*
     * ��ʼ��MAC�������Ĵ���.
     */
    write_dword(MAC1, (1ul<<1));                                            /* ʹ�ܴ�����������֡                   */
    write_dword(MAC2, (1ul<<4) |                                            /* ʹ����ÿ֡�����crc                  */
                      (1ul<<5));                                            /* ������еĶ�֡                       */
    write_dword(IPGR, 0x0C12);
    write_dword(CLRT, 0x370F);
    write_dword(MAXF, ETH_MAX_FLEN);                                        /* ���֡size: 1536                     */
    
    
    /*
     * Fcpu = 100MHz, clock select=44, MDC=100/44=3MHz.
     */
    write_dword(MCFG, (1ul<<15) |                                           /* ��λMII����Ӳ��                      */
                      (0xA<<2));                                            /* ����ʱ��44��Ƶ                       */
    delay(100);
    write_dword(MCFG, (0xA<<2));

    /*
     * ���ù���ģʽ
     */
    write_dword(Command, (1ul<<6) |                                         /* ��С��64�ֽڵĶ�֡���ݵ��洢��       */
                         (1ul<<7) |                                         /* ��ֹ���չ���,��������֡������洢��  */
                         (1ul<<9));                                         /* ѡ��RMIIģʽ                         */
    /*
     * ��λMII�߼�.
     */
    write_dword(SUPP, (1ul<<8) |
                      (1ul<<11));
    delay(100);
    write_dword(SUPP, 1ul<<8);

    /* ɨ��phy������ַ */
    for (i = 1; i < 32; i++)
    {
        read_phy(i, 3, &PhyID);
        read_phy(i, 2, &PhyID);
        if (0x0022 == PhyID)                                                /* KSZ8041����ID: 0x0022                */
        {
            break;
        }
    }
    if (32 <= i)                                                            /* Phy������Ч��ַ��Χ: [1, 31]         */
    {
        return OS_ERR_HARDWARE;
    }
    PhyAddress = i;
    
    /*
     * ��λ�����շ���,��ʹ����������Ӧ����.
     */
    write_phy(PhyAddress, 0, (1ul<<15) |                                    /* software reset                       */
                             (1ul<<12) |                                    /* enable auto-negotiation process      */
                             (1ul<< 9));                                    /* restart auto-negotiation process     */
    sleep(2);
    do 
    {
        read_phy(PhyAddress, 0x1F, &status);
        status &= 0x1C;                                                     /* ��ȡ��Ӧ��Ϣ                         */
    } while (0 == status);                                                  /* �ȴ�����Ӧ�������                   */

    switch (status)
    {
        case 0x04:                                                          /* 10Mbps & ��˫��                      */
            write_dword(IPGT, 0x12);
            dword_clr_bits(MAC2, 1ul << 0);                                 /* ͨ��ģʽ: ��˫��ģʽ                 */          
            dword_clr_bits(SUPP, 1ul<<8);                                   /* ��������: 10Mbps                     */
            dword_clr_bits(Command, 1ul << 10);                             /* ��˫��ģʽ                           */
            break;
            
        case 0x08:                                                          /* 100Mbps & ��˫��                     */
            write_dword(IPGT, 0x12);
            dword_clr_bits(MAC2, 1ul << 0);                                 /* ͨ��ģʽ: ��˫��ģʽ                 */
            dword_set_bits(SUPP, 1ul<<8);                                   /* ��������: 100Mbps                    */
            dword_clr_bits(Command, 1ul << 10);                             /* ��˫��ģʽ                           */
            break;
            
        case 0x14:                                                          /* 10Mbps & ȫ˫��                      */
            write_dword(IPGT, 0x15);
            dword_set_bits(MAC2, 1ul << 0);                                 /* ͨ��ģʽ: ȫ˫��ģʽ                 */            
            dword_clr_bits(SUPP, 1ul<<8);                                   /* ��������: 10Mbps                     */
            dword_set_bits(Command, 1ul << 10);                             /* ȫ˫��ģʽ                           */
            break;
            
        case 0x18:                                                          /* 100Mbps & ȫ˫��                     */
            write_dword(IPGT, 0x15);
            dword_set_bits(MAC2, 1ul << 0);                                 /* ͨ��ģʽ: ȫ˫��ģʽ                 */
            dword_set_bits(SUPP, 1ul<<8);                                   /* ��������: 100Mbps                    */
            dword_set_bits(Command, 1ul << 10);                             /* ȫ˫��ģʽ                           */
            break;

        default :
            return OS_ERR_HARDWARE;
    }

    /* ���������ַ */
    write_dword(SA0, (netif->hwaddr[0] << 8) + (netif->hwaddr[1] << 0));
    write_dword(SA1, (netif->hwaddr[2] << 8) + (netif->hwaddr[3] << 0));
    write_dword(SA2, (netif->hwaddr[4] << 8) + (netif->hwaddr[5] << 0));

    /*
     * ���û���������.
     */
    init_recv_description();
    init_sent_description();
    
    write_dword(RxFilterCtrl, (1ul << 1) |                                  /* �������й㲥֡                       */
                              (1ul << 5));                                  /* ����Ŀ���ַ��վ��ַ��ͬ��֡         */
    write_dword(IntClear, 0xFFFF);
    write_dword(IntEnable, (1ul << 3) |                                     /* ʹ�ܽ�������ж�                     */
                           (1ul << 7));                                     /* ʹ�ܷ�������ж�                     */
    dword_set_bits(Command, (1ul<<0)|                                       /* ʹ�ܽ���ͨ��                         */
                            (1ul<<1));                                      /* ʹ�ܷ���ͨ��                         */
    dword_set_bits(MAC1, (1ul << 0));                                       /* ����Խ���֡���н���                 */
    
    /*
     * ע���豸
     */
    irq_register(ENET_IRQn, 20, ethernet_exception);
    
    DBG_INFO("��̫�����óɹ�\r\n");
    return ERR_OK;
}

#if 0
/*********************************************************************************************************************
** Function name:           link
** Descriptions:            �鿴��̫���ӿ�����״̬
** Input parameters:        Option : ����������
** Output parameters:       
** Returned value:          ==OS_OK : �����ɹ�
**                          !=OS_OK : ����ʧ��(����������Ϣ)
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
        prints("������\r\n");
    }
    else
    {
        prints("δ����\r\n");
    }
    
    return OS_OK;
}
EXPORT_TO_CONSOLE("����״̬", link);
#endif

/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

