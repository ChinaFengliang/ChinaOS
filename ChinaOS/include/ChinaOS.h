/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            ChinaOS.h
** Last version:         V1.00
** Descriptions:         system interface file.
** Hardware platform:    
** SoftWare platform:    ChinaOS
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           Feng Liang
** Created date:         2009-10-07  11:23:6
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
#ifndef __CHINA_OS__H
#define __CHINA_OS__H

/*********************************************************************************************************************
                                                   头文件区
*********************************************************************************************************************/
/* 操作系统接口头文件列表 ------------------------------------------------------------------------------------------*/
#include    <include/macro.h>
#include    <include/types.h>
#include    <include/debug.h>
#include    <kernel/kernel.h>
#include    <kernel/pipe/pipe.h>
#include    <kernel/mailbox/mailbox.h>
#include    <kernel/semaphore/semaphore.h>
#include    <syscall.h>

/* 工具库接口头文件列表 --------------------------------------------------------------------------------------------*/
#include    <library/sysio/sysio.h>
#include    <library/ioport.h>

/* 设备接口头文件列表 ----------------------------------------------------------------------------------------------*/
#include    <device.h>

/* 以太网络接口头文件列表 ------------------------------------------------------------------------------------------*/
#if 0
#include 	"lwip/opt.h"
#include 	"lwip/ip_addr.h"
#include 	"lwip/init.h"
#include 	"lwip/stats.h"
#include 	"lwip/sys.h"
#include 	"lwip/mem.h"
#include 	"lwip/memp.h"
#include 	"lwip/pbuf.h"
#include 	"lwip/netif.h"
#include 	"lwip/sockets.h"
#include 	"lwip/ip.h"
#include 	"lwip/raw.h"
#include 	"lwip/udp.h"
#include 	"lwip/tcp.h"
#include 	"lwip/snmp_msg.h"
#include 	"lwip/autoip.h"
#include 	"lwip/igmp.h"
#include 	"lwip/dns.h"
#include 	"netif/etharp.h"
#include 	"netif/ethernet.h"
#include 	"lwip/tcpip.h"
#include 	"lwip/sockets.h"
#include 	"lwip/netif.h"
#include 	"lwip/ip_addr.h"
#endif
/* 文件系统接口头文件列表 ------------------------------------------------------------------------------------------*/


#endif                                                                      /* end of __CHINA_OS__H                 */
/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

