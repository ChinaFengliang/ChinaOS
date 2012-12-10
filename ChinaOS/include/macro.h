/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**---------File Information-------------------------------------------------------------------------------------------
** File name:           macro.h
** Last version:        V1.00
** Descriptions:        系统宏头文件.
** Hardware platform:   ARM7、ARM9、Cortex-M0、Cortex-M3
** SoftWare platform:   ChinaOS

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
#ifndef __OS_MACRO_H
#define __OS_MACRO_H

/*********************************************************************************************************************
                                                   宏定义区
*********************************************************************************************************************/
/* 关键字 ----------------------------------------------------------------------------------------------------------*/
#define  INLINE                    __inline                                 /* 内嵌关键字                           */

/* 通用宏 ----------------------------------------------------------------------------------------------------------*/
#ifndef  NULL
#define  NULL                      ((void *)0)                              /* 空指针值                             */
#endif

#ifndef  FALSE                                                              /* 假                                   */
#define  FALSE                     0
#endif

#ifndef  TRUE                                                               /* 真                                   */
#define  TRUE                      1
#endif

/* 状态信息 --------------------------------------------------------------------------------------------------------*/
#define  OK                         0                                       /* 操作成功(通用)                       */
#define  ERR                       -1                                       /* 操作失败(通用)                       */
#define  ERR_HARDWARE              -2                                       /* 硬件出错                             */
#define  ERR_SOFTWARE              -3                                       /* 软件出错                             */
#define  ERR_BAD_PARAM             -4                                       /* 参数出错                             */
#define  ERR_TIMEOUT               -5                                       /* 超时出错                             */
#define  ERR_DEVICE_BUSY           -6                                       /* 设备忙                               */
#define  ERR_NO_MEMERY             -7                                       /* 内存不足                             */
#define  ERR_BAD_ID                -8                                       /* 非法ID号                             */

/* 事件信号 --------------------------------------------------------------------------------------------------------*/
#define  SIG_TERM                   0                                       /* 进程终止                             */
#define  SIG_KILL                   1                                       /* 进程杀死                             */

/* 系统定义 --------------------------------------------------------------------------------------------------------*/
#define  ENDIAN                     										/* 存储端格式							*/

/*********************************************************************************************************************
** Function name:           ARRAY_SIZE
** Descriptions:            求数组的成员数目
** Input parameters:        aArray : 数组名称.
** Output parameters:       
** Returned value:          成员数目.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-6-19  10:22:36
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
#define  ARRAY_SIZE(aArray)                                                \
         (sizeof(aArray) / sizeof(aArray[0]))

/*
 * NOTE! This ctype does not handle EOF like the standard C
 * library is required to.
 */

#define _U	0x01	/* upper */
#define _L	0x02	/* lower */
#define _D	0x04	/* digit */
#define _C	0x08	/* cntrl */
#define _P	0x10	/* punct */
#define _S	0x20	/* white space (space/lf/tab) */
#define _X	0x40	/* hex digit */
#define _SP	0x80	/* hard space (0x20) */

extern const unsigned char _ctype[];

#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)	((__ismask(c)&(_U|_L|_D)) != 0)
#define isalpha(c)	((__ismask(c)&(_U|_L)) != 0)
#define iscntrl(c)	((__ismask(c)&(_C)) != 0)
#define isdigit(c)	((__ismask(c)&(_D)) != 0)
#define isgraph(c)	((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define islower(c)	((__ismask(c)&(_L)) != 0)
#define isprint(c)	((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define ispunct(c)	((__ismask(c)&(_P)) != 0)
/* Note: isspace() must return false for %NUL-terminator */
#define isspace(c)	((__ismask(c)&(_S)) != 0)
#define isupper(c)	((__ismask(c)&(_U)) != 0)
#define isxdigit(c)	((__ismask(c)&(_D|_X)) != 0)

#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

/*********************************************************************************************************************
** Function name:           tolower
** Descriptions:            将大写字母转换成小写字母.
** Input parameters:        C : 大写字母.
** Output parameters:       
** Returned value:          小写字母.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-6-19  10:30:58
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static INLINE unsigned char tolower(unsigned char C)
{
	if (isupper(C))
		C -= 'A'-'a';
	return C;
}

/*********************************************************************************************************************
** Function name:           toupper
** Descriptions:            将小写字母转换成大写字母.
** Input parameters:        c : 小写字母.
** Output parameters:       
** Returned value:          大写字母.
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-6-19  10:31:2
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
static INLINE unsigned char toupper(unsigned char c)
{
	if (islower(c))
		c -= 'a'-'A';
	return c;
}

/*********************************************************************************************************************
** Function name:           container_of
** Descriptions:            translate member pointer int container pointer.
** Input parameters:        pMember       : the pointer to the member.
**                          ContainerType : the type of the container struct.
**                          MemberName    : the name of the member within the struct.
** Output parameters:       
** Returned value:          container pointer
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-6-18  20:40:7
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
#define  container_of(pMember, ContainerType, MemberName)                  \
((ContainerType *)((char *)pMember - (int)&(((ContainerType *)0)->MemberName)))

/*********************************************************************************************************************
** Function name:           SECTION
** Descriptions:            标识段名称属性
** Input parameters:        Name : 段名称
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-6-19  10:15:49
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
#define SECTION(Name)               __attribute__((section(Name)))
//#define SECTION(Name)               __attribute__(used, (section(Name)))
/*********************************************************************************************************************
** Function name:           EXPORT_TO_COMMAND
** Descriptions:            导出函数到命令列表
** Input parameters:        Description: 功能描述
**                          Function   : 功能函数
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-6-18  20:51:36
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
#define EXPORT_TO_COMMAND(Description, Function)                           \
        const SERVICE Service##Function SECTION("aConsoleServices")=       \
        {                                                                  \
            #Function,                                                     \
            Description,                                                   \
            Function                                                       \
        }

/*********************************************************************************************************************
** Function name:           EXPORT_TO_DEVICE
** Descriptions:            导出设备到设备列表
** Input parameters:        Device     : 设备对象
**                          Description: 设备描述
** Output parameters:       
** Returned value:          
**--------------------------------------------------------------------------------------------------------------------
** Created by:              Feng Liang
** Created Date:            2012-6-18  20:51:28
** Test recorde:            
**--------------------------------------------------------------------------------------------------------------------
** Modified by:             /device
** Modified date:
** Test recorde: 
*********************************************************************************************************************/
#define EXPORT_TO_DEVICE(Device, Description)                              \
        const DEVICE List##Device SECTION("aDeviceList")=                  \
        {                                                                  \
            #Device,                                                       \
            Description,                                                   \
            (int (**)(void))&Device.setup,                                 \
        }

#endif                                                                      /* __OS_MACRO_H                         */
/*********************************************************************************************************************
                                                   END OF FILE
*********************************************************************************************************************/

