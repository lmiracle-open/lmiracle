#include "lmiracle.h"
#include "lm_kservice.h"

#include "user_mb_port.h"

/*------------------------Slave mode use these variables----------------------*/

/* Slave mode:DiscreteInputs variables */
USHORT   usSDiscInStart                               = S_DISCRETE_INPUT_START;
#if S_DISCRETE_INPUT_NDISCRETES%8
UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8+1];
#else
UCHAR    ucSDiscInBuf[S_DISCRETE_INPUT_NDISCRETES/8]  ;
#endif
/* Slave mode:Coils variables */
USHORT   usSCoilStart                                 = S_COIL_START;
#if S_COIL_NCOILS%8
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8+1]                ;
#else
UCHAR    ucSCoilBuf[S_COIL_NCOILS/8]                  ;
#endif
/* Slave mode:InputRegister variables */
USHORT   usSRegInStart                                = S_REG_INPUT_START;
USHORT   usSRegInBuf[S_REG_INPUT_NREGS]               ;
/* Slave mode:HoldingRegister variables */
USHORT   usSRegHoldStart                              = S_REG_HOLDING_START;
USHORT   usSRegHoldBuf[S_REG_HOLDING_NREGS]           ;

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

/**
 * modbus 从机发送任务 (测试用)
 */
static void mb_slave_send (void *p_arg)
{
    lm_kprintf("modbus slave send task start... \r\n");

    /* 1.往保持寄存器 中写值 */
    while (1) {
        usSRegHoldBuf[0] ++;
        lm_task_delay(200);
    }
}

/**
 * modbus 从机轮询任务
 */
static void mb_slave_poll (void *p_arg)
{
    lm_kprintf("modbus poll slave task start... \r\n");

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
 * modbus 协议初始化
 */
int mb_protocol_init (void)
{
    lm_err_t ret = LM_OK;

    /* 1.参数检查 */
    if (NULL == g_modbus) {
        g_modbus = &__mb_def_param;
    }

    /* 2.创建从机轮询任务 */
    lm_task_create("slave_poll", \
                    mb_slave_poll, \
                    NULL, \
                    g_modbus->stack_size, \
                    g_modbus->prio);

    /* 3.创建从机发送任务 */
    lm_task_create("slave_send", \
                    mb_slave_send, \
                    NULL, \
                    g_modbus->stack_size, \
                    g_modbus->prio);

    return ret;
}

/**
 * modbus从机 输入寄存器回调函数
 * @param pucRegBuffer input register buffer
 * @param usAddress input register address
 * @param usNRegs input register number
 * @return result
 */
eMBErrorCode eMBRegInputCB( UCHAR * pucRegBuffer, \
                            USHORT usAddress, \
                            USHORT usNRegs)
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
        while (usNRegs > 0) {
            *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] >> 8);
            *pucRegBuffer++ = (UCHAR) (pusRegInputBuf[iRegIndex] & 0xFF);
            iRegIndex++;
            usNRegs--;
        }
    } else {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/**
 * modbus从机 保持寄存器回调函数
 * @param pucRegBuffer holding register buffer
 * @param usAddress holding register address
 * @param usNRegs holding register number
 * @param eMode read or write
 * @return result
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
            while (usNRegs > 0) {
                pusRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                pusRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
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
 * modbus从机 线圈寄存器回调函数
 * @param pucRegBuffer coils buffer
 * @param usAddress coils address
 * @param usNCoils coils number
 * @param eMode read or write
 * @return result
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
    iNReg =  usNCoils / 8 + 1;

    pucCoilBuf = ucSCoilBuf;
    COIL_START = S_COIL_START;
    COIL_NCOILS = S_COIL_NCOILS;
    usCoilStart = usSCoilStart;

    /* it already plus one in modbus function method. */
    usAddress--;

    if( ( usAddress >= COIL_START ) &&
        ( usAddress + usNCoils <= COIL_START + COIL_NCOILS ) ) {
        iRegIndex = (USHORT) (usAddress - usCoilStart) / 8;
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
            break;
        }
    } else {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}

/**
 * modbus从机 离散量寄存器回调函数
 * @param pucRegBuffer discrete buffer
 * @param usAddress discrete address
 * @param usNDiscrete discrete number
 * @return result
 */
eMBErrorCode eMBRegDiscreteCB(  UCHAR * pucRegBuffer, \
                                USHORT usAddress, \
                                USHORT usNDiscrete )
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
    } else {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/* end of file */
