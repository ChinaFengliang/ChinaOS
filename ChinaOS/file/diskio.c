/******************************************************Copyright (c)**************************************************
**
**                              Copyright (c) 2009,2011 Fengliang And His Friends Corporation.
**
**                                             E-Mail: ChinaFengliang@163.com
**
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:            diskio.c
** Last version:         V1.00
** Descriptions:         文件.
** Hardware platform:    上位机.
** SoftWare platform:    Windows
**
**--------------------------------------------------------------------------------------------------------------------
** Created by:           FengLiang
** Created date:         2011年5月16日  14:43:15
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

#include "stdafx.h"
#include <windows.h> 
#include "OS_macro.h"
#include "OS_type.h"
#include "diskio.h"
#include "./ata/ata_disk.h"

/*********************************************************************************************************************
                                                    宏定义区
*********************************************************************************************************************/
/* 宏段名 ----------------------------------------------------------------------------------------------------------*/
#define ATA_DISK                        0                                   /* USB磁盘                              */
#define USB_DISK                        1                                   /* ATA磁盘                              */

/*********************************************************************************************************************
                                                    类型定义区
*********************************************************************************************************************/
/* 基本类型 --------------------------------------------------------------------------------------------------------*/


/*********************************************************************************************************************
                                                  全局变量定义区
*********************************************************************************************************************/


/*********************************************************************************************************************
** Function name:           disk_initialize
** Descriptions:            initializes the disk drive.
** Input parameters:        Drive : Physical drive number 
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-16  13:17:30
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
DSTATUS disk_initialize(BYTE Drive)
{
    switch (Drive)
    {
        case ATA_DISK:
            return ata_disk_initialize();

        case USB_DISK:
            break;  

        default :
            break;
    }
    
    return STA_NOINIT;
}

/*********************************************************************************************************************
** Function name:           disk_status
** Descriptions:            returns the current disk status.
** Input parameters:        Drive : Specifies the physical drive number to be confirmed.
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-16  13:22:16
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
DSTATUS disk_status(BYTE Drive)
{
    switch (Drive)
    {
        case ATA_DISK:
            return ata_disk_status();

        case USB_DISK:
            break;  

        default :
            break;
    }
    
    return RES_PARERR;
}

/*********************************************************************************************************************
** Function name:           disk_read
** Descriptions:            reads sector(s) from the disk drive.
** Input parameters:        Drive        : Physical drive number.
**                          pBuffer      : Pointer to the read data buffer
**                          SectorNumber : Start sector number
**                          SectorCount  : Number of sectros to read
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-16  13:26:27
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
DRESULT disk_read(BYTE Drive, BYTE* pBuffer, DWORD SectorNumber, BYTE SectorCount)
{
    switch (Drive)
    {
        case ATA_DISK:
            return ata_disk_read(pBuffer, SectorNumber, SectorCount);

        case USB_DISK:
            break;  

        default :
            break;
    }
    
    return RES_PARERR;
}

#if _READONLY == 0
/*********************************************************************************************************************
** Function name:           disk_write
** Descriptions:            writes sector(s) to the disk.
** Input parameters:        Drive        : Physical drive number.
**                          pBuffer      : Pointer to the write data buffer
**                          SectorNumber : Start sector number
**                          SectorCount  : Number of sectros to write
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-16  14:52:0
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
DRESULT disk_write(BYTE Drive, const BYTE* pBuffer, DWORD SectorNumber, BYTE SectorCount)
{
    switch (Drive)
    {
        case ATA_DISK:
            return ata_disk_write(pBuffer, SectorNumber, SectorCount);

        case USB_DISK:
            break;  

        default :
            break;
    }

    return OS_ERR;
}
#endif

/*********************************************************************************************************************
** Function name:           disk_ioctl
** Descriptions:            cntrols device specified features and miscellaneous functions other than disk read/write.
** Input parameters:        Drive   : Drive number
**                          Command : Control command code
**                          pBuffer : Parameter and data buffer
** Output parameters:       
** Returned value:          ==OS_OK : 操作成功
**                          !=OS_OK : 操作失败(包含出错信息)
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-16  14:54:14
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
DRESULT disk_ioctl (BYTE Drive, BYTE Command, void *pBuffer)
{
    switch (Drive)
    {
        case ATA_DISK:
            return ata_disk_ioctl(Command, pBuffer);

        case USB_DISK:
            break;  

        default :
            break;
    }
    
    return RES_PARERR;
}

/*********************************************************************************************************************
** Function name:           get_fattime
** Descriptions:            获取系统时间.
** Input parameters:        
** Output parameters:       
** Returned value:          bit[31:25]: year;  从1980年到现在的年数
**                          bit[24:21]: month; 
**                          bit[20:16]: day;
**                          bit[15:11]: hour;
**                          bit[10: 5]: minute;
**                          bit[4 : 0]: second/2;
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Fengliang
** Created Date:            2011-5-16  14:54:14
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
#if !_FS_READONLY
DWORD get_fattime (void)
{
    return (10ul << 25) |
           ( 6ul << 21) |
           (18ul << 16) |
           (18ul << 11) |
           (28ul <<  5) |
           (10ul);
}
#endif
/*********************************************************************************************************************
                                                    END OF FILE
*********************************************************************************************************************/

