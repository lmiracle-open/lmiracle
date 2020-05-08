#include "lmiracle.h"
#include "lm_kservice.h"

#include "user_mb_port.h"

/*---------------------------- modbus从机寄存器缓存定义 ---------------------------*/

/* 离散输入寄存器起始地址 */
static USHORT   usSDiscInStart     = S_DISCRETE_INPUT_START;

/* 离散输入寄存器缓存 */
#if S_DISCRETE_INPUT_NDISCRETES%8
static UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8+1];/* 超过8bit 字节+1 */
#else
static UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8];
#endif

/* 线圈寄存器起始地址 */
static USHORT   usSCoilStart        = S_COIL_START;
/* 写线圈寄存器字节起始地址 */
static USHORT   WriteCoilStart      = 0;
/* 写线圈寄存器位起始地址 */
static USHORT   WriteCoilBitStart   = 0;
/* 写线圈寄存器数量 */
static USHORT   WriteCoilRegNum     = 0;

/* 线圈寄存器缓存 */
#if S_COIL_NCOILS%8
static UCHAR    ucSCoilBuf[S_COIL_NCOILS/8+1];                /* 超过8bit 字节+1 */
#else
static UCHAR    ucSCoilBuf[S_COIL_NCOILS/8];
#endif

/* 输入寄存器起始地址 */
static USHORT   usSRegInStart  = S_REG_INPUT_START;

/* 输入寄存器缓存 */
static USHORT   usSRegInBuf[S_REG_INPUT_NREGS];

/* 保持寄存器起始地址 */
static USHORT   usSRegHoldStart    = S_REG_HOLDING_START;

/* 写保持寄存器起始地址 */
static USHORT   WriteHoldStart      = 0;
/* 写保持寄存器数量 */
static USHORT   WriteHoldRegNum     = 0;

/* 保持寄存器缓存 */
static USHORT   usSRegHoldBuf[S_REG_HOLDING_NREGS];

/******************************************************************************/

/**************************** modbus 从机参数配置定义 ******************************/

/* modbus默认参数 */
const static lm_mb_param_t __mb_def_param = {
    .stack_size         = 512,                  /* 栈深度 */
    .prio               = 4,                    /* 优先级 */
    .embmode            = MB_RTU,               /* RTU模式 */
    .slave_addr         = 0x01,                 /* 从机地址 */
    .uport              = 0,                    /* 串口号 */
    .embparity          = MB_PAR_EVEN,          /* 偶校验 */
    .ubaud              = 115200,               /* 波特率 */
};

/* 定义modbus参数指针 */
const static lm_mb_param_t *g_modbus = NULL;

/* 定义写寄存器事件实体 */
static lm_devent_t event_write_reg = NULL;

/******************************************************************************/

/*************************** modbus 从机参数外部注册接口 ***************************/
/**
 * modbus 参数注册接口
 */
int mb_param_register (const lm_mb_param_t *mb_param)
{
    /* 1.检查输入参数是否有效 */
    if (unlikely(NULL == mb_param)) {
        return LM_ERROR;
    }

    /* 2.注册modbus参数 */
    g_modbus = mb_param;

    return LM_OK;
}

/******************************************************************************/

/******************************* modbus 从机轮训任务 ******************************/

/**
 * modbus 从机轮询任务
 */
static void mb_slave_poll (void *p_arg)
{
    lm_kprintf("modbus slave poll task start... \r\n");

    /* 1.初始化modbus */
    eMBInit(g_modbus->embmode, \
            g_modbus->slave_addr, \
            g_modbus->uport, \
            g_modbus->ubaud, \
            g_modbus->embparity);

    /* 2.使能modbus */
    eMBEnable();

    /* 3.轮询modbus事件 */
    while (1) {
        eMBPoll();
    }
}

/**
 * modbus 从机事件任务
 */
static void mb_slave_event (void *p_arg)
{
    lm_bits_t recv_event;

    lm_kprintf("modbus slave event task start... \r\n");

    /* 1.轮询modbus操作寄存器事件 */
    while (1) {
        recv_event = lm_event_wait(  event_write_reg,
                        WRITE_COIL_REG_EVENT | WRITE_HOLDING_REG_EVENT,
                        pdTRUE,     /* 退出时清除事件位 */
                        pdFALSE,    /* 以上事件只要满足其一就可以 */
                        LM_SEM_WAIT_FOREVER);
        if (WRITE_COIL_REG_EVENT == recv_event) {
            /* 执行写线圈寄存器回调 */
            if (g_modbus->write_coil_reg_cb) {
                g_modbus->write_coil_reg_cb(ucSCoilBuf, \
                                            WriteCoilStart, \
                                            WriteCoilBitStart, \
                                            WriteCoilRegNum);
            }
        } else if (WRITE_HOLDING_REG_EVENT == recv_event) {
            /* 执行写保持寄存器回调 */
            if (g_modbus->write_hold_reg_cb) {
                g_modbus->write_hold_reg_cb(usSRegHoldBuf, \
                                            WriteHoldStart, \
                                            WriteHoldRegNum);
            }
        }
    }
}

/**
 * modbus 协议初始化
 */
int mb_protocol_init (void)
{
    lm_err_t ret = LM_OK;

    /* 1.参数检查 */
    if (NULL == g_modbus) {
        g_modbus = &__mb_def_param;
    }

    /* 2.创建写寄存器事件 */
    event_write_reg = lm_event_create();           /* 创建事件标志组 */
    if (unlikely(NULL == event_write_reg)) {
        return FALSE;
    }

    /* 3.创建从机寄存器事件任务 */
    lm_task_create("reg_event", \
                    mb_slave_event, \
                    NULL, \
                    g_modbus->stack_size, \
                    g_modbus->prio);

    /* 4.创建从机轮询任务 */
    lm_task_create("slave_poll", \
                    mb_slave_poll, \
                    NULL, \
                    g_modbus->stack_size, \
                    g_modbus->prio);

    return ret;
}

/******************************************************************************/

/****************************** modbus 寄存器回调函数 *****************************/

/**
 * modbus 输入寄存器回调函数 (只读 注意：此函数中已经包含了写操作，但是modbus主机只能读访问)
 * @param pucRegBuffer: 读输入寄存器数据指针
 * @param usAddress:    读输入寄存器数据地址
 * @param usNRegs:      读输入寄存器数量
 * @param eMode:        读写模式
 * @return              错误码
 */
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, \
                            USHORT usAddress, \
                            USHORT usNRegs, \
                            eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegInputBuf;
    USHORT          REG_INPUT_START;
    USHORT          REG_INPUT_NREGS;
    USHORT          usRegInStart;

    pusRegInputBuf = usSRegInBuf;
    REG_INPUT_START = S_REG_INPUT_START;
    REG_INPUT_NREGS = S_REG_INPUT_NREGS;
    usRegInStart = usSRegInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= REG_INPUT_START)
            && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS)) {
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

    return eStatus;
}

/**
 * modbus 保持寄存器回调函数 (可读可写)
 * @param pucRegBuffer: 读写保持寄存器数据指针
 * @param usAddress:    读写保持寄存器数据地址
 * @param usNRegs:      读写保持寄存器数量
 * @param eMode:        读写模式
 * @return              错误码
 */
eMBErrorCode eMBRegHoldingCB(   UCHAR * pucRegBuffer, \
                                USHORT usAddress, \
                                USHORT usNRegs, \
                                eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;
    USHORT *        pusRegHoldingBuf;
    USHORT          REG_HOLDING_START;
    USHORT          REG_HOLDING_NREGS;
    USHORT          usRegHoldStart;

    pusRegHoldingBuf = usSRegHoldBuf;
    REG_HOLDING_START = S_REG_HOLDING_START;
    REG_HOLDING_NREGS = S_REG_HOLDING_NREGS;
    usRegHoldStart = usSRegHoldStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= REG_HOLDING_START)
            && (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS)) {
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
            WriteHoldStart = iRegIndex;
            WriteHoldRegNum = usNRegs;

            while (usNRegs > 0) {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
            /* 发送写保持寄存器事件 */
            lm_event_set(event_write_reg, WRITE_HOLDING_REG_EVENT);
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/**
 * modbus 线圈寄存器回调函数 (可读可写)
 * @param pucRegBuffer: 读写线圈寄存器数据指针
 * @param usAddress:    读写线圈寄存器数据地址
 * @param usNCoils:     读写线圈寄存器数量
 * @param eMode:        读写模式
 * @return              错误码
 */
eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, \
                            USHORT usAddress, \
                            USHORT usNCoils, \
                            eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucCoilBuf;
    USHORT          COIL_START;
    USHORT          COIL_NCOILS;
    USHORT          usCoilStart;

    /**
     * iNReg:   需要操作的线圈寄存器数量(字节数) eg: usNCoils=10
     * iNReg = usNCoils / 8 + 1 = 2
     * 表示总共操作两个字节
     */
    iNReg =  usNCoils / 8 + 1;

    pucCoilBuf = ucSCoilBuf;
    COIL_START = S_COIL_START;
    COIL_NCOILS = S_COIL_NCOILS;
    usCoilStart = usSCoilStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if( ( usAddress >= COIL_START ) &&
        ( usAddress + usNCoils <= COIL_START + COIL_NCOILS ) ) {
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
            WriteCoilRegNum = usNCoils;
            WriteCoilStart = iRegIndex;
            WriteCoilBitStart = iRegBitIndex;

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
            /* 发送写线圈寄存器事件 */
            lm_event_set(event_write_reg, WRITE_COIL_REG_EVENT);
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
 * modbus 离散输入寄存器回调函数 (只读 注意：此函数中已经包含了写操作，但是modbus主机只能读访问)
 * @param pucRegBuffer: 读离散输入寄存器数据指针
 * @param usAddress:    读离散输入寄存器数据地址
 * @param usNDiscrete:  读离散输入寄存器数量
 * @param eMode:        读写模式
 * @return              错误码
 */
eMBErrorCode eMBRegDiscreteCB(  UCHAR * pucRegBuffer, \
                                USHORT usAddress, \
                                USHORT usNDiscrete, \
                                eMBRegisterMode eMode)
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex , iRegBitIndex , iNReg;
    UCHAR *         pucDiscreteInputBuf;
    USHORT          DISCRETE_INPUT_START;
    USHORT          DISCRETE_INPUT_NDISCRETES;
    USHORT          usDiscreteInputStart;
    iNReg =  usNDiscrete / 8 + 1;

    pucDiscreteInputBuf = ucSDiscInBuf;
    DISCRETE_INPUT_START = S_DISCRETE_INPUT_START;
    DISCRETE_INPUT_NDISCRETES = S_DISCRETE_INPUT_NDISCRETES;
    usDiscreteInputStart = usSDiscInStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if ((usAddress >= DISCRETE_INPUT_START)
            && (usAddress + usNDiscrete    <= \
                    DISCRETE_INPUT_START + DISCRETE_INPUT_NDISCRETES)) {
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

    return eStatus;
}

/******************************************************************************/

/**
 * 用户操作寄存器接口
 */
int lm_modbus_reg_ops (mbRegOpsType type, uint8_t *pBuf, uint16_t addr, uint16_t num)
{
     mbRegOpsType iType = type;
     int    err = MB_ENOERR;

     lm_assert(pBuf == NULL);

     if (LM_WRITE_COIL_REG == iType) {                      /* 写线圈寄存器 */
         err = eMBRegCoilsCB(pBuf, addr, num, MB_REG_WRITE);
     } else if (LM_WRITE_DISCRETE_INPUT_REG == iType) {     /* 写离散输入寄存器 */
         err = eMBRegDiscreteCB(pBuf, addr, num, MB_REG_WRITE);
     } else if (LM_WRITE_HOLDING_REG == iType) {            /* 写 保持寄存器 */
         err = eMBRegHoldingCB(pBuf, addr, num, MB_REG_WRITE);
     } else if (LM_WRITE_INPUT_REG == iType) {              /* 写输入寄存器 */
         err = eMBRegInputCB(pBuf, addr, num, MB_REG_WRITE);
     } else {
         /* 参数错误 不做处理 */
     }

     return err;
}

/* end of file */
