/********************************* Copyright(c) ********************************
*
*                          LANMENG Scientific Creation
*                          https: //www.lmiracle.com
*
* File Name     : portmodbus.c
* Change Logs   :
* Date          Author          Notes
* 2019-06-07    terryall        V1.0    first version
*******************************************************************************/

/*******************************************************************************
* Description   : modbus移植接口文件
*******************************************************************************/

#include "lm_mb_interface.h"
#include "lm_pack.h"

#define __MB_POLL_TASK_PRIO             6           /* modbus轮询事件任务优先级 */
#define __MB_POLL_TASK_STACK            256         /* modbus轮询事件任务栈深度 */

/* 定义modbus寄存器触发事件 */
#define __MB_HOLD_REG_EVENT             (1<<0)
#define __MB_COIL_REG_EVENT             (1<<1)

/* 定义modbus参数指针 */
static lm_mb_param_t                *__gp_mb_param      = NULL;

/* 定义modbus寄存器指针 */
static lm_mb_reg_t                  *__gp_mb_reg        = NULL;

/* 定义modbus寄存器事件 */
static lm_devent_t                  __gp_mb_reg_event   = NULL;

/* 定义modbus寄存器读写锁 */
static lm_mutex_t                   __gp_mb_reg_mutex;

/* 当前保持寄存器写地址 */
static USHORT cur_reg_waddr                       = 0;

/* 当前保持寄存器写个数 */
static USHORT cur_reg_wnum                        = 0;

/*******************************************************************************
* Description   : modbus四个寄存器回调函数
*******************************************************************************/

/**
 * @brief modbus 输入寄存器回调函数 (只读: modbus模块调用)
 */
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer,   \
                            USHORT usAddress,       \
                            USHORT usNRegs,         \
                            eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegInputBuf;
    USHORT          REG_INPUT_NREGS;
    USHORT          usRegInStart;

    /* 1. 参数有效性检查 */
    if (NULL == __gp_mb_reg || NULL == __gp_mb_reg->in_buf) goto end;

    /* 2. 地址内存映射 */
    pusRegInputBuf          = __gp_mb_reg->in_buf;
    REG_INPUT_NREGS         = __gp_mb_reg->in_num;
    usRegInStart            = __gp_mb_reg->in_start_addr;

    /* it already plus one in modbus function method. */
    usAddress--;

    /* 3. 获取锁 */
    lm_mutex_lock(&__gp_mb_reg_mutex, LM_SEM_WAIT_FOREVER);

    /* 4. 内存拷贝 */
    if ((usAddress >= usRegInStart)
            && (usAddress + usNRegs <= usRegInStart + REG_INPUT_NREGS)) {
        iRegIndex = usAddress - usRegInStart;

        switch (eMode) {
        /* read current register values from the protocol stack. */
        case MB_REG_READ:
            while (usNRegs > 0) {
                *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;
        /* write current register values with new values from the protocol stack. */
        case MB_REG_WRITE:
            while (usNRegs > 0) {
                pusRegInputBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegInputBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }

    /* 5. 释放锁 */
    lm_mutex_unlock(&__gp_mb_reg_mutex);

    end:
    return eStatus;
}

/**
 * @brief modbus 保持寄存器回调函数 (可读可写: modbus模块调用)
 */
eMBErrorCode eMBRegHoldingCB(   UCHAR * pucRegBuffer,   \
                                USHORT usAddress,       \
                                USHORT usNRegs,         \
                                eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegHoldingBuf;
    USHORT          REG_HOLDING_NREGS;
    USHORT          usRegHoldStart;

    /* 1. 参数有效性检查 */
    if (NULL == __gp_mb_reg || NULL == __gp_mb_reg->hold_buf) goto end;

    /* 2. 地址内存映射 */
    pusRegHoldingBuf        = __gp_mb_reg->hold_buf;
    REG_HOLDING_NREGS       = __gp_mb_reg->hold_num;
    usRegHoldStart          = __gp_mb_reg->hold_start_addr;

    /* it already plus one in modbus function method. */
    usAddress--;

    /* 3. 获取锁 */
    lm_mutex_lock(&__gp_mb_reg_mutex, LM_SEM_WAIT_FOREVER);

    /* 4. 内存拷贝 */
    if ((usAddress >= usRegHoldStart)
            && (usAddress + usNRegs <= usRegHoldStart + REG_HOLDING_NREGS)) {
        iRegIndex = usAddress - usRegHoldStart;
        switch (eMode) {
        /* read current register values from the protocol stack. */
        case MB_REG_READ:
            while (usNRegs > 0) {
                *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] >> 8);
                *pucRegBuffer++ = (UCHAR) (pusRegHoldingBuf[iRegIndex] & 0xFF);
                iRegIndex++;
                usNRegs--;
            }
            break;
        /* write current register values with new values from the protocol stack. */
        case MB_REG_WRITE:
            /* 1. 设置当前寄存器写地址和数量 */
            cur_reg_waddr = usAddress + 1;  /* 保存写寄存器起始地址,模块内部使用 */
            cur_reg_wnum  = usNRegs;        /* 保存写寄存器数量,模块内部使用 */

            /* 2. 更新保持寄存器的内容 */
            while (usNRegs > 0) {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }

            /* 3. 发送写保持寄存器事件 */
            lm_event_set(__gp_mb_reg_event, __MB_HOLD_REG_EVENT);
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }

    /* 5. 释放锁 */
    lm_mutex_unlock(&__gp_mb_reg_mutex);

    end:
    return eStatus;
}

/**
 * @brief modbus 线圈寄存器回调函数 (可读可写: modbus模块调用)
 */
eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer,   \
                            USHORT usAddress,       \
                            USHORT usNCoils,        \
                            eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucCoilBuf;
    USHORT          COIL_NCOILS;
    USHORT          usCoilStart;

    /* 1. 参数有效性检查 */
    if (NULL == __gp_mb_reg || NULL == __gp_mb_reg->coil_buf) goto end;

    /**
     * iNReg:   需要操作的线圈寄存器数量(字节数) eg: usNCoils=10
     * iNReg = usNCoils / 8 + 1 = 2
     * 表示总共操作两个字节
     */
    /* 2. 地址内存映射 */
    iNReg                   = usNCoils / 8 + 1;
    pucCoilBuf              = __gp_mb_reg->coil_buf;
    COIL_NCOILS             = __gp_mb_reg->coil_num;
    usCoilStart             = __gp_mb_reg->coil_start_addr;

    /* it already plus one in modbus function method. */
    usAddress--;

    /* 3. 获取锁 */
    lm_mutex_lock(&__gp_mb_reg_mutex, LM_SEM_WAIT_FOREVER);

    /* 4. 内存拷贝 */
    if( ( usAddress >= usCoilStart ) &&
        ( usAddress + usNCoils <= usCoilStart + COIL_NCOILS ) ) {
        /**
         * iRegIndex:   寄存器字节起始序号 eg: usAddress=19 usCoilStart=0
         * iRegIndex = (usAddress - usCoilStart) / 8 = 2
         * 从线圈寄存器的第二个字节开始操作
         */
        iRegIndex = (USHORT) (usAddress - usCoilStart) / 8;
        /**
         * iRegBitIndex:   寄存器位起始序号 eg: usAddress=19 usCoilStart=0
         * iRegBitIndex = (usAddress - usCoilStart) % 8 = 3
         * 从线圈寄存器的第二个字节的第3位开始操作
         */
        iRegBitIndex = (USHORT) (usAddress - usCoilStart) % 8;

        switch ( eMode ) {
        /* read current coil values from the protocol stack. */
        case MB_REG_READ:
            /*
             * 1. iNReg = 2  iRegIndex = 2  iRegBitIndex = 3  usNCoils = 10  pucRegBuffer[0]
             * 2. iNReg = 2  iRegIndex = 3  iRegBitIndex = 3  usNCoils = 10  pucRegBuffer[1]
             * 3. usNCoils = 2  pucRegBuffer[1] 10101101 << 6 = 01000000
             * 4. usNCoils = 2  pucRegBuffer[1] 01000000 >> 6 = 00000001
             */
            while (iNReg > 0) {
                *pucRegBuffer++ = xMBUtilGetBits(&pucCoilBuf[iRegIndex++],
                        iRegBitIndex, 8);
                iNReg--;
            }
            pucRegBuffer--;
            /* last coils */
            usNCoils = usNCoils % 8;
            /* filling zero to high bit */
            *pucRegBuffer = *pucRegBuffer << (8 - usNCoils);
            *pucRegBuffer = *pucRegBuffer >> (8 - usNCoils);
            break;
        /* write current coil values with new values from the protocol stack. */
        case MB_REG_WRITE:
            /* 1. 设置当前寄存器写地址和数量 */
            cur_reg_waddr       = usAddress + 1;    /* 保存写寄存器起始地址,模块内部使用 */
            cur_reg_wnum        = usNCoils;         /* 保存写寄存器数量,这里代表字节数,模块内部使用 */

            /* 2. 更新线圈寄存器的内容 */
            while (iNReg > 1) {
                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, 8,
                        *pucRegBuffer++);
                iNReg--;
            }
            /* last coils */
            usNCoils = usNCoils % 8;
            /* xMBUtilSetBits has bug when ucNBits is zero */
            if (usNCoils != 0) {
                xMBUtilSetBits(&pucCoilBuf[iRegIndex++], iRegBitIndex, usNCoils,
                        *pucRegBuffer++);
            }

            /* 3. 发送写线圈寄存器事件 */
            lm_event_set(__gp_mb_reg_event, __MB_COIL_REG_EVENT);
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }

    /* 5. 释放锁 */
    lm_mutex_unlock(&__gp_mb_reg_mutex);

    end:
    return eStatus;
}

/**
 * @brief modbus 离散输入寄存器回调函数 (只读: modbus模块调用)
 */
eMBErrorCode eMBRegDiscreteCB(  UCHAR * pucRegBuffer,   \
                                USHORT usAddress,       \
                                USHORT usNDiscrete,     \
                                eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucDiscreteInputBuf;
    USHORT          DISCRETE_INPUT_NDISCRETES;
    USHORT          usDiscreteInputStart;

    /* 1. 参数有效性检查 */
    if (NULL == __gp_mb_reg || NULL == __gp_mb_reg->disc_buf) goto end;

    /* 2. 地址内存映射 */
    pucDiscreteInputBuf         = __gp_mb_reg->disc_buf;
    DISCRETE_INPUT_NDISCRETES   = __gp_mb_reg->disc_num;
    usDiscreteInputStart        = __gp_mb_reg->disc_start_addr;
    iNReg                       = usNDiscrete / 8 + 1;

    /* it already plus one in modbus function method. */
    usAddress--;

    /* 3. 获取锁 */
    lm_mutex_lock(&__gp_mb_reg_mutex, LM_SEM_WAIT_FOREVER);

    /* 4. 内存拷贝 */
    if ((usAddress >= usDiscreteInputStart)
            && (usAddress + usNDiscrete    <= \
                    usDiscreteInputStart + DISCRETE_INPUT_NDISCRETES)) {
        iRegIndex = (USHORT) (usAddress - usDiscreteInputStart) / 8;
        iRegBitIndex = (USHORT) (usAddress - usDiscreteInputStart) % 8;

        switch ( eMode ) {
        /* read current discrete values from the protocol stack. */
        case MB_REG_READ:
            while (iNReg > 0) {
                *pucRegBuffer++ = xMBUtilGetBits(&pucDiscreteInputBuf[iRegIndex++],
                        iRegBitIndex, 8);
                iNReg--;
            }
            pucRegBuffer--;
            /* last discrete */
            usNDiscrete = usNDiscrete % 8;
            /* filling zero to high bit */
            *pucRegBuffer = *pucRegBuffer << (8 - usNDiscrete);
            *pucRegBuffer = *pucRegBuffer >> (8 - usNDiscrete);
            break;
        /* write current discrete values with new values from the protocol stack. */
        case MB_REG_WRITE:
            while (iNReg > 1) {
                xMBUtilSetBits(&pucDiscreteInputBuf[iRegIndex++], iRegBitIndex, 8,
                        *pucRegBuffer++);
                iNReg--;
            }
            /* last coils */
            usNDiscrete = usNDiscrete % 8;
            /* xMBUtilSetBits has bug when ucNBits is zero */
            if (usNDiscrete != 0) {
                xMBUtilSetBits(&pucDiscreteInputBuf[iRegIndex++], iRegBitIndex, usNDiscrete,
                        *pucRegBuffer++);
            }
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }

    /* 5. 释放锁 */
    lm_mutex_unlock(&__gp_mb_reg_mutex);

    end:
    return eStatus;
}

/*******************************************************************************
* Description   : modbus poll任务
*******************************************************************************/

/**
 * @brief modbus 从机轮询任务
 */
static void __mb_slave_poll_task (void *p_arg)
{
    /* 1. 检查参数是否有效 */
    lm_assert(NULL != __gp_mb_param);

    /* 2. 初始化modbus */
    eMBInit(__gp_mb_param->mb_mode,       \
            __gp_mb_param->slave_addr,    \
            __gp_mb_param->mb_com,        \
            __gp_mb_param->mb_baud,       \
            __gp_mb_param->mb_par);

    /* 3. 使能modbus */
    eMBEnable();

    while (1) {
        /* 轮询modbus事件 */
        eMBPoll();
    }
}

/*******************************************************************************
* Description   : modbus 寄存器对外读写接口
*******************************************************************************/

/**
 * @brief modbus寄存器写操作
 */
int lm_modbus_reg_write (uint8_t type, const uint8_t *pbuf, uint16_t addr, uint16_t inum)
{
    int ret = LM_OK;

    /* 1. 参数有效性检查 */
    if (NULL == pbuf) {
        return -LM_ERROR;
    }

    /* 2. modbus操作寄存器地址+1, todo: 此处为了和modbus协议内部处理保持一致 地址应+1 */
    addr += 1;

    /* 3. 根据不同类型操作对应的寄存器 */
    switch (type) {
    case LM_MB_COIL_REG:       /* todo: 暂时不做处理 */
        break;
    case LM_MB_DISCRETE_REG:   /* todo: 暂时你做处理 */
        break;
    case LM_MB_INPUT_REG:
        ret = eMBRegInputCB((UCHAR *)pbuf, (USHORT)addr, (USHORT)inum, MB_REG_WRITE);
        break;
    case LM_MB_HOLD_REG:       /* todo: 暂时不做处理 */
        break;
    default :
        break;
    }

    return ret;
}

/**
 * @brief modbus寄存器读操作
 */
int lm_modbus_reg_read (void *data, uint16_t size)
{
    int ret = -LM_ERROR;

    uint16_t len = 0;

    lm_bits_t r_event;          /* 接收事件信息 */

    uint16_t *p_data = (uint16_t *)data;

    /* 1. 参数有效性检查 */
    if (NULL == data) {
        return ret;
    }

    /* 2. 检测是否有事件发生,阻塞等待事件 */
    r_event = lm_event_wait(__gp_mb_reg_event,                          \
                            __MB_HOLD_REG_EVENT | __MB_COIL_REG_EVENT,  \
                            pdTRUE, pdFALSE, LM_SEM_WAIT_FOREVER);

    /* 3. 如果有事件发生 */
    if (__MB_HOLD_REG_EVENT == r_event ||  __MB_COIL_REG_EVENT == r_event) {

        /* 填充地址 */
        p_data[len++] = cur_reg_waddr-1;

        /* 填充长度 */
        p_data[len++] = cur_reg_wnum;

        /* 拷贝数据 */
        if (__MB_HOLD_REG_EVENT == r_event) {
            if ((cur_reg_wnum*2) > (size-(2*len))) {
                eMBRegHoldingCB((UCHAR *)&p_data[len], cur_reg_waddr, (size-(2*len))/2, MB_REG_READ);
                /* 打包数据 (大端格式) */
                pack_be16(&p_data[len], &p_data[len], (size-(2*len))/2);
                len += (size-(2*len))/2;
            } else {
                eMBRegHoldingCB((UCHAR *)&p_data[len], cur_reg_waddr, cur_reg_wnum, MB_REG_READ);
                /* 打包数据 (大端格式) */
                pack_be16(&p_data[len], &p_data[len], cur_reg_wnum);
                len += cur_reg_wnum;
            }
            len *= 2;       /* todo: 转换为字节长度 */
        } else if (__MB_COIL_REG_EVENT == r_event) {
            if ((cur_reg_wnum/8 + 1) > (size-(2*len))) {
                eMBRegCoilsCB((UCHAR *)&p_data[len], cur_reg_waddr, 8*(size-(2*len)), MB_REG_READ);
                len += (size-(2*len));
            } else {
                eMBRegCoilsCB((UCHAR *)&p_data[len], cur_reg_waddr, cur_reg_wnum, MB_REG_READ);
                len += (cur_reg_wnum/8 + 1);
            }
            len += 2;       /* todo: 长度和地址按照16位存放的,所以此处多加2字节 */
        }

        /* 返回实际拷贝的长度 */
        ret = len;
    }

    return ret;
}

/*******************************************************************************
* Description   : modbus 寄存器注册接口
*******************************************************************************/
/**
 * @brief modbus寄存器接口注册
 */
int lm_modbus_reg_register (lm_mb_reg_t *p_mb_reg)
{
    int ret = LM_OK;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != p_mb_reg);

    /* 2. 注册 */
    __gp_mb_reg = p_mb_reg;

    return ret;
}

/*******************************************************************************
* Description   : modbus 参数注册接口
*******************************************************************************/
/**
 * @brief modbus参数接口注册
 */
int lm_modbus_param_register (lm_mb_param_t *p_mb_param)
{
    int ret = LM_OK;

    /* 1. 检查输入参数是否有效 */
    lm_assert(NULL != p_mb_param);

    /* 2. 注册 */
    __gp_mb_param = p_mb_param;

    /* 3. 创建寄存器读写锁 */
    lm_mutex_create(&__gp_mb_reg_mutex);

    /* 4. 创建寄存器事件 */
    __gp_mb_reg_event = lm_event_create();
    lm_assert(NULL != __gp_mb_reg_event);

    /* 5. 创建modbus从机轮询任务 */
    ret = lm_task_create(   "slave_poll",                   \
                            __mb_slave_poll_task,           \
                            __MB_POLL_TASK_STACK,            \
                            __MB_POLL_TASK_PRIO,           \
                            NULL);
    lm_assert(LM_TYPE_FAIL != (lm_base_t)ret);

    return LM_OK;
}

/* end of file */
