/*
  This file is part of CanFestival, a library implementing CanOpen
  Stack.

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
  USA
*/
/*!
** @file   objacces.c
** @author Edouard TISSERANT and Francis DUPIN
** @date   Tue Jun  5 08:55:23 2007
**
** @brief
**
**
*/




/* #define DEBUG_WAR_CONSOLE_ON */
/* #define DEBUG_ERR_CONSOLE_ON */

#include "objacces.h"
#include "data.h"


void* memcpy_flash(void* dest, const void* source, size_t length)
{
    char* dstPointer = dest;
    const char* srcPointer = source;
    size_t i;
    for(i = 0; i < length; i++) 
    {
        dstPointer[i] = srcPointer[i];
    }
    return dstPointer;
}

//We need the function implementation for linking
//Only a placeholder with a define isnt enough!
UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex,
                           UNS32 sizeDataDict, UNS32 sizeDataGiven, UNS32 code)
{
#ifdef DEBUG_WAR_CONSOLE_ON
  MSG_WAR("Dictionary index : 0x%X", index);
  MSG_WAR("        subindex : 0x%X", subIndex);
  switch (code)
  {
  case OD_NO_SUCH_OBJECT:
    MSG_WAR("Index 0x%X not found", index);
    break;
  case OD_NO_SUCH_SUBINDEX :
    MSG_WAR("SubIndex 0x%X not found ", subIndex);
    break;
  case OD_WRITE_NOT_ALLOWED :
    MSG_WAR("Write not allowed, data is read only");
    break;
  case OD_LENGTH_DATA_INVALID :
    MSG_WAR("Conflict size data. Should be (bytes) : %d", sizeDataDict);
    MSG_WAR("But you have given the size : %d", sizeDataGiven);
    break;
  case OD_NOT_MAPPABLE :
    MSG_WAR("Not mappable data in a PDO at index 0x%X: ", index);
    break;
  case OD_VALUE_TOO_LOW :
    MSG_WAR("Value range error : value too low. SDOabort : 0x%X", code);
    break;
  case OD_VALUE_TOO_HIGH :
    MSG_WAR("Value range error : value too high. SDOabort : 0x%X", code);
    break;
  default :
    MSG_WAR("Unknown error code : 0x%X", code);
  }
  #endif

  return 0;
}

UNS32 _getODentry( CO_Data *d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void *pDestData,
                   UNS32 *pExpectedSize,
                   UNS8 *pDataType,
                   UNS8 checkAccess,
                   UNS8 endianize)
{ /* DO NOT USE MSG_ERR because the macro may send a PDO -> infinite
    loop if it fails. */
    (void)endianize;
    UNS32 errorCode;
    UNS32 szData;
    const indextable* ptrTable;
    ODCallback_t* Callback;

    ptrTable = (*(d->scanIndexOD))(wIndex, &errorCode, &Callback);

    if (errorCode != OD_SUCCESSFUL)
    {
        return errorCode;
    }
        
    if( ptrTable->bSubCount <= bSubindex )
    {
        /* Subindex not found */
        accessDictionaryError(wIndex, bSubindex, 0, 0, OD_NO_SUCH_SUBINDEX);
        return OD_NO_SUCH_SUBINDEX;
    }

    const subindex* pSubIdx = &(ptrTable->pSubindex[bSubindex]);
    if (checkAccess && (pSubIdx->bAccessType == WO))
    {
        MSG_WAR("Access Type : %d", pSubIdx->bAccessType);
        accessDictionaryError(wIndex, bSubindex, 0, 0, OD_READ_NOT_ALLOWED);
        return OD_READ_NOT_ALLOWED;
    }

    if (pDestData == 0)
    {
        return SDOABT_GENERAL_ERROR;
    }

    if (pSubIdx->size > (*pExpectedSize))
    {
        /* Requested variable is too large to fit into a transfer line, inform    *
        * the caller about the real size of the requested variable.              */
        *pExpectedSize = pSubIdx->size;
        return SDOABT_OUT_OF_MEMORY;
    }

    *pDataType = pSubIdx->bDataType;
    szData = pSubIdx->size;

#ifdef CANOPEN_BIG_ENDIAN
    if( endianize && (*pDataType > boolean) && !(*pDataType >= visible_string && *pDataType <= domain) )
    {
        /* data must be transmited with low byte first */
        MSG_WAR("data type %d (bool: %d; visible_string: %d)", *pDataType, boolean, visible_string);
        UNS8 j = 0;
        for (UNS8 i = szData; i > 0; i--)
        {
            ((UNS8*)pDestData)[j] = (pSubIdx->bAccessType == CONST) ? (pSubIdx->pObjectConst)[i-1] : (pSubIdx->pObject)[i-1];
            ++j;
        }
        *pExpectedSize = szData;
    }
    else /* no endianisation change */
#endif
    if(pSubIdx->bAccessType == CONST)
    {
        if((pSubIdx->bDataType == visible_string) && (bSubindex != 0))
        {
            const char* dp = *(const char* const *)(pSubIdx->pObjectConst);
            memcpy_flash(pDestData, dp, szData);
        }
        else
        {
            memcpy_flash(pDestData, pSubIdx->pObjectConst, szData);
        }
    }
    else
    {
        memcpy(pDestData, pSubIdx->pObject,szData);
    }

    if(*pDataType != visible_string)
    {
        *pExpectedSize = szData;
    } 
    else
    {
        /* VISIBLE_STRING objects are returned with \0 termination, if the user   *
        * provided enough space.                                                 *
        * Note:  If the parameter "Default String Size" of the Object Dictionary *
        *        Editor is larger than the string, then the \0 byte will be      *
        *        appended anyways!                                               */
        if((*pExpectedSize) > pSubIdx->size)
        {
            *((UNS8*)pDestData + szData) = '\0';
            *pExpectedSize = szData + 1;
        }
        else
        {
            *pExpectedSize = szData;
        }
    }
    return OD_SUCCESSFUL;
}

UNS32 _setODentry( CO_Data *d,
                   UNS16 wIndex,
                   UNS8 bSubindex,
                   void *pSourceData,
                   UNS32 *pExpectedSize,
                   UNS8 checkAccess,
                   UNS8 endianize)
{
    (void)endianize;
    UNS32 szData;
    UNS8 dataType;
    UNS32 errorCode;
    const indextable *ptrTable;
    ODCallback_t *Callback;

    ptrTable = (*d->scanIndexOD)(wIndex, &errorCode, &Callback);
    if (errorCode != OD_SUCCESSFUL)
    {
        return errorCode;
    }

    if( ptrTable->bSubCount <= bSubindex )
    {
        /* Subindex not found */
        accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_NO_SUCH_SUBINDEX);
        return OD_NO_SUCH_SUBINDEX;
    }

    if (checkAccess && (ptrTable->pSubindex[bSubindex].bAccessType == RO || ptrTable->pSubindex[bSubindex].bAccessType == CONST)) 
    {
        MSG_WAR("Access Type : %d", ptrTable->pSubindex[bSubindex].bAccessType);
        accessDictionaryError(wIndex, bSubindex, 0, *pExpectedSize, OD_WRITE_NOT_ALLOWED);
        return OD_WRITE_NOT_ALLOWED;
    }


    dataType = ptrTable->pSubindex[bSubindex].bDataType;
    szData = ptrTable->pSubindex[bSubindex].size;

    /* check the size, we must allow to store less bytes than data size, even for intergers
	 (e.g. UNS40 : objdictedit will store it in a uint64_t, setting the size to 8 but PDO comes
	 with 5 bytes so ExpectedSize is 5 */
    if( (*pExpectedSize == 0) || (*pExpectedSize <= szData) )
    {
#ifdef CANOPEN_BIG_ENDIAN
        /* re-endianize do not occur for bool, strings time and domains */
        if(endianize && (dataType > boolean) && !(dataType >= visible_string && dataType <= domain))
        {
            /* we invert the data source directly. This let us do range testing without */
            /* additional temp variable */
            for (UNS8 i = 0 ; i < ( ptrTable->pSubindex[bSubindex].size >> 1); i++)
            {
                // i from left to right, dataIdx from right to left
                UNS32 dataIdx = (ptrTable->pSubindex[bSubindex].size - 1) - i;
                UNS8* dataPtr = (UNS8 *)pSourceData;
                UNS8 tmp = dataPtr[dataIdx];
                dataPtr[dataIdx] = dataPtr[i];
                dataPtr[i] = tmp;
            }
        }
#endif
        errorCode = (*d->valueRangeTest)(dataType, pSourceData);
        if (errorCode)
        {
            accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, errorCode);
            return errorCode;
        }
        memcpy(ptrTable->pSubindex[bSubindex].pObject, pSourceData, *pExpectedSize);
        /* TODO : CONFORM TO DS-301 : 
        *  - stop using NULL terminated strings
        *  - store string size in td_subindex 
        * */
        /* terminate visible_string with '\0' */
        if(dataType == visible_string && *pExpectedSize < szData)
        {
            ((UNS8*)ptrTable->pSubindex[bSubindex].pObject)[*pExpectedSize] = 0;
        }

        *pExpectedSize = szData;

        /* Callbacks */
        if(Callback && Callback[bSubindex])
        {
            errorCode = (Callback[bSubindex])(d, wIndex, bSubindex);
            if(errorCode != OD_SUCCESSFUL)
            {
                return errorCode;
            }
        }

        /* Store value if requested with user defined function
         Function should return OD_ACCES_FAILED in case of store error */
        if (ptrTable->pSubindex[bSubindex].bAccessType & TO_BE_SAVE)
        {
            return (*d->storeODSubIndex)(d, wIndex, bSubindex);
        }
        return OD_SUCCESSFUL;
    }
    else
    {
        *pExpectedSize = szData;
        accessDictionaryError(wIndex, bSubindex, szData, *pExpectedSize, OD_LENGTH_DATA_INVALID);
        return OD_LENGTH_DATA_INVALID;
    }
}

const indextable* scanIndexOD (CO_Data *d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback)
{
    return (*(d->scanIndexOD))(wIndex, errorCode, Callback);
}

UNS32 RegisterSetODentryCallBack(CO_Data *d, UNS16 wIndex, UNS8 bSubindex, ODCallback_t Callback)
{
    UNS32 errorCode;
    ODCallback_t *CallbackList;
    const indextable *odentry;

    odentry = scanIndexOD(d, wIndex, &errorCode, &CallbackList);
    if((errorCode == OD_SUCCESSFUL)  &&  CallbackList  &&  (bSubindex < odentry->bSubCount))
    {
        CallbackList[bSubindex] = Callback;
    }
    return errorCode;
}

UNS32 dummy_storeODSubIndex (CO_Data *d, UNS16 wIndex, UNS8 bSubindex)
{
    return OD_SUCCESSFUL;
}
