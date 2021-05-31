/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: port.h ,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#ifndef _PORT_H
#define _PORT_H

#include "mbconfig.h"

#include "lmiracle.h"
#include "lm_error.h"
#include "lm_assert.h"
#include <inttypes.h>
#include <stdbool.h>

/* 调试宏开关 */
#define LM_MB_DEBUG         (0)

#ifndef assert
#define assert(e)  lm_assert(e)
#endif

#define INLINE
#define PR_BEGIN_EXTERN_C           extern "C" {
#define PR_END_EXTERN_C             }

#define ENTER_CRITICAL_SECTION()    lm_critical_enter()
#define EXIT_CRITICAL_SECTION()     lm_critical_exit()

typedef uint8_t BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

/**
 * @brief modbus获取串口传输类型
 *
 * @param[in]   None
 *
 * @return  (void *)
 */
extern void *get_serial_transmit_type (void);

#endif
