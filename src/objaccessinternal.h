#ifndef __objaccessinternal_h__
#define __objaccessinternal_h__

#include "objdictdef.h"
#include "data.h"

#define READ_UNS32(objDict, index, subIndex)\
        (objDict[index].pSubindex[subIndex].bAccessType != CONST ? *(UNS32*)objDict[index].pSubindex[subIndex].pObject : *(const CONSTSTORE UNS32*)objDict[index].pSubindex[subIndex].pObjectConst)

#define READ_UNS16(objDict, index, subIndex)\
        (objDict[index].pSubindex[subIndex].bAccessType != CONST ? *(UNS16*)objDict[index].pSubindex[subIndex].pObject : *(const CONSTSTORE UNS16*)objDict[index].pSubindex[subIndex].pObjectConst)

#define READ_UNS8(objDict, index, subIndex)\
       (objDict[index].pSubindex[subIndex].bAccessType != CONST ? *(UNS8*)objDict[index].pSubindex[subIndex].pObject : *(const CONSTSTORE UNS8*)objDict[index].pSubindex[subIndex].pObjectConst)

#define IS_NULL(objDict, index, subIndex)\
        (objDict[index].pSubindex[subIndex].bAccessType != CONST ? objDict[index].pSubindex[subIndex].pObject == NULL : objDict[index].pSubindex[subIndex].pObjectConst == NULL)

#define WRITE_UNS32(objDict, index, subIndex, value)\
    (*((UNS32*)objDict[index].pSubindex[subIndex].pObject) = value)

#define WRITE_UNS16(objDict, index, subIndex, value)\
    (*((UNS16*)objDict[index].pSubindex[subIndex].pObject) = value)

#define WRITE_UNS8(objDict, index, subIndex, value)\
    (*((UNS8*)objDict[index].pSubindex[subIndex].pObject) = value)

/**
 * @brief Scan the index of object dictionary. Used only by setODentry and getODentry.
 * @param *d Pointer to a CAN object data structure
 * @param wIndex
 * @param *errorCode :  OD_SUCCESSFUL if index foundor SDO abort code. (See file def.h)
 * @param **Callback
 * @return NULL if index not found. Else : return the table part of the object dictionary.
 */
 const CONSTSTORE indextable * scanIndexOD (CO_Data* d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback);

#endif /* __objaccessinternal_h__ */
