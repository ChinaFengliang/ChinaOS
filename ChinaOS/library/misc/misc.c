/******************************************************Copyright (c)**************************************************
**                                              胆怯是阻止一切可能的根本缘由
**
**                                             E-Mail: ChinaFengliang@163.com
**
**--------------File Info---------------------------------------------------------------------------------------------
** File name:           SourceMutex.c
**
** Last version:        V1.00
** Descriptions:        资源互斥管理文件.
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


/*********************************************************************************************************************
                                                   头文件区
*********************************************************************************************************************/
#include "./OS_DataType.h"

/*********************************************************************************************************************
** Function name:           MutexLock
** Descriptions:            资源锁定
** Input parameters:        pSourceFlag: 资源标志指针
** Output parameters:       None
** Returned value:          OK  ：操作成功；
**                          ERR : 操作失败。
** Created by:              Fengliang
** Created Date:            2009-10-07
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************************/
INT32S  MutexLock (volatile INT8U *pSourceFlag)
{
    if (NULL == pSourceFlag)
    {
        return OS_ERR_BAD_PARAM;
    }
    if (0 == *pSourceFlag)
    {
        (*pSourceFlag)++;
        if (1 == *pSourceFlag)
        {
            return OS_OK;
        }
        (*pSourceFlag)--;
    }
    return OS_ERR_DEVICE_BUSY;
}

/*********************************************************************************************************************
** Function name:           MutexUnlock
** Descriptions:            资源解锁
** Input parameters:        pSourceFlag: 资源标志指针
** Output parameters:       None
** Returned value:          OK  ：操作成功；
**                          ERR : 操作失败。
** Created by:              Fengliang
** Created Date:            2009-10-07
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************************/
INT32S MutexUnlock (volatile INT8U *pSourceFlag)
{
    if (NULL == pSourceFlag)
    {
        return OS_ERR_BAD_PARAM;
    }
    *pSourceFlag = 0;
    return OS_OK;
}

/*********************************************************************************************************************
** Function name:           parseParameter
** Descriptions:            字符串参数提取
** input parameters:        pcstr :  输入参数字符串
**                          pccmd :  标准命令字符串
**                          NumOfParam : 提取参数个数
** output parameters:       puiOutData ：提取出的参数
** Returned value:          成功提取出的参数对应的位图
** Created by:              Gaojiasheng
** Created Date:            2009-10-07
**--------------------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
*********************************************************************************************************************/
INT32U ParseParam(INT32U        *pOutData,
                  char          *pStr,
                  char*const     pCmd[],
                  INT32U         NumOfParam)
{
    INT32U          i;
    INT32U          Ret;                                                    /*  返回值                              */
    char           *pStr1 = NULL;                                           /*  输入字符串                          */

    if (pOutData == NULL ||
        pStr      == NULL ||
        pCmd      == NULL ||
        Num       == 0) {

        return 0;
    }

    Ret = 0;
    for (i = 0;i < NumOfParam;i++) {

        if (pCmd[i] == NULL) {
            continue;
        }
        pStr1 = strstr(pStr, pCmd[i]);                                      /*  查找命令字                          */
        if (pStr1 != NULL) {
                                                                            /*  提取参数值                          */
            if (sscanf(pStr1 + strlen(pCmd[i]), "%i", pOutData + i)) {
                Ret |= 1 << i;
            }
        }
    }

    return   Ret;

}

/*********************************************************************************************************************
  END OF FILE
*********************************************************************************************************************/

