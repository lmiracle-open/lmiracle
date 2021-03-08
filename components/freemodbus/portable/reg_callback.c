/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : reg_callback.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : modbus寄存器回调函数定义(需要外部实现)
*******************************************************************************/

#include "port.h"
#include "mb.h"

#include "lm_types.h"

/**
 * @brief modbus线圈寄存器回调函数(需要外部实现)
 */
eMBErrorCode __default eMBRegCoilsCB(       UCHAR * pucRegBuffer, \
                                            USHORT usAddress,\
                                            USHORT usNCoils, \
                                            eMBRegisterMode eMode)
{
    return MB_ENOERR;
}

/**
 * @brief modbus离散寄存器回调函数(需要外部实现)
 */
eMBErrorCode __default eMBRegDiscreteCB(    UCHAR * pucRegBuffer, \
                                            USHORT usAddress,\
                                            USHORT usNDiscrete, \
                                            eMBRegisterMode eMode)
{
    return MB_ENOERR;
}

/**
 * @brief modbus保持寄存器回调函数(需要外部实现)
 */
eMBErrorCode __default eMBRegHoldingCB(     UCHAR * pucRegBuffer, \
                                            USHORT usAddress,\
                                            USHORT usNRegs, \
                                            eMBRegisterMode eMode)
{
    return MB_ENOERR;
}

/**
 * @brief modbus输入寄存器回调函数(需要外部实现)
 */
eMBErrorCode __default eMBRegInputCB(       UCHAR * pucRegBuffer, \
                                            USHORT usAddress,\
                                            USHORT usNRegs, \
                                            eMBRegisterMode eMode)
{
    return MB_ENOERR;
}

/* end of file */
