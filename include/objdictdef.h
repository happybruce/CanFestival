/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __OBJ_DICT_DEF_H__
#define __OBJ_DICT_DEF_H__

#if defined(__CC_ARM)
#pragma anon_unions
#endif

/************************* CONSTANTS **********************************/
/** These are statically defined data types taken from the CANopen standard. They
 *  are located at index 0x0001 to 0x001B. As described in the standard, they
 *  are in the object dictionary for definition purposes only. A device does not
 *  need to support all of these data types.
 */
#define boolean         0x01
#define int8            0x02
#define int16           0x03
#define int32           0x04
#define uint8           0x05
#define uint16          0x06
#define uint32          0x07
#define real32          0x08
#define visible_string  0x09
#define octet_string    0x0A
#define unicode_string  0x0B
#define time_of_day     0x0C
#define time_difference 0x0D

#define domain          0x0F
#define int24           0x10
#define real64          0x11
#define int40           0x12
#define int48           0x13
#define int56           0x14
#define int64           0x15
#define uint24          0x16

#define uint40          0x18
#define uint48          0x19
#define uint56          0x1A
#define uint64          0x1B

#define pdo_communication_parameter 0x20
#define pdo_mapping                 0x21
#define sdo_parameter               0x22
#define identity                    0x23

/* CanFestival is using 0x24 to 0xFF to define some types containing a 
 value range (See how it works in objdict.c)
 */


/** Each entry of the object dictionary can be READ ONLY (RO), READ/WRITE (RW),
 *  WRITE-ONLY (WO)
 */
#define RW     0x00  
#define WO     0x01
#define RO     0x02
#define CONST  0x03

#define TO_BE_SAVE  0x04
#define DCF_TO_SEND 0x08


/************************ STRUCTURES ****************************/
/** These are structures needed to create entries
 *  of the object dictionary.
 */
typedef struct td_subindex
{
    const UNS8 bAccessType;
    const UNS8 bDataType; /* Defines the data type of the entry */
    UNS32      size;      /* The size (in Byte) of the variable */
    union {
        void* pObject;    /* Pointer to the variable */
        const void* const pObjectConst;
    };
} subindex;

/** Structure for creating entries in the communication profile
 */
typedef struct td_indextable
{
    const subindex* const pSubindex; /* Pointer to the subindex */
    const UNS8 bSubCount;            /* Number of valid entries for this subindex
                                      * This count defines how much memory has been
                                      * allocated. This memory does not have to be used.
                                      */
    const UNS16 index;
} indextable;

/**
 * @brief Index of specified entry in Object Dict (OD). Only valid if value is > 0
 * 
 */
typedef struct s_quick_index {
    UNS16 SDO_SVR;     /* Index in OD for SDO Server */
    UNS16 SDO_CLT;     /* Index in OD for SDO Client */
    UNS16 PDO_RCV;     /* Index in OD for received PDO */
    UNS16 PDO_RCV_MAP; /* Index in OD for received PDO mapping */
    UNS16 PDO_TRS;     /* Index in OD for transmitted PDO */
    UNS16 PDO_TRS_MAP; /* Index in OD for transmitted PDO mapping */
} quick_index;

/************************** MACROS *********************************/


#include "declaration.h"
typedef UNS32 (*ODCallback_t)(CO_Data* d, UNS16 wIndex, UNS8 bSubindex);
typedef const indextable* (*scanIndexOD_t)(UNS16 wIndex, UNS32* errorCode, ODCallback_t** Callback);

/* Useful CANopen helpers */
#define GET_NODE_ID(m)         (UNS16_LE(m.cob_id) & 0x7f)
#define GET_FUNCTION_CODE(m)   (UNS16_LE(m.cob_id) >> 7)

#endif /* __OBJ_DICT_DEF_H__ */
