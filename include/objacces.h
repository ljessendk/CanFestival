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

/** \file
 *  \brief Responsible for accessing the object dictionary.
 *
 *  This file contains functions for accessing the object dictionary and
 *  variables that are contained by the object dictionary.
 *  Accessing the object dictionary contains setting local variables
 *  as PDOs and accessing (read/write) all entries of the object dictionary
 *  \warning Only the basic entries of an object dictionary are included
 *           at the moment.
 */

#ifndef __objacces_h__
#define __objacces_h__

#include <applicfg.h>

typedef UNS32 (*valueRangeTest_t)(UNS8 typeValue, void *Value);

#include "data.h"



/*
Print MSG_WAR (s) if error to the access to the object dictionary occurs.
You must uncomment the lines
//#define DEBUG_CAN
//#define DEBUG_WAR_CONSOLE_ON
//#define DEBUG_ERR_CONSOLE_ON
in the file objaccess.c
sizeDataDict : Size of the data defined in the dictionary
sizeDataGiven : Size data given by the user.
code : error code to print. (SDO abort code. See file def.h)
Beware that sometimes, we force the sizeDataDict or sizeDataGiven to 0, when we wants to use
this function but we do not have the access to the right value. One example is
getSDOerror(). So do not take attention to these variables if they are null.
*/

UNS8 accessDictionaryError(UNS16 index, UNS8 subIndex, 
			   UNS8 sizeDataDict, UNS8 sizeDataGiven, UNS32 code);


/* Reads an entry from the object dictionary.\n
 *  \code
 *  // Example usage:
 *  UNS8  *pbData;
 *  UNS8 length;
 *  UNS32 returnValue;
 *
 *  returnValue = getODentry( (UNS16)0x100B, (UNS8)1, 
 *  (void * *)&pbData, (UNS8 *)&length );
 *  if( returnValue != SUCCESSFUL )
 *  {
 *      // error handling
 *  }
 *  \endcode 
 *  \param wIndex The index in the object dictionary where you want to read
 *                an entry
 *  \param bSubindex The subindex of the Index. e.g. mostly subindex 0 is
 *                   used to tell you how many valid entries you can find
 *                   in this index. Look at the canopen standard for further
 *                   information
 *  \param ppbData Pointer to the pointer which points to the variable where
 *                 the value of this object dictionary entry should be copied
 *  \param pdwSize This function writes the size of the copied value (in Byte)
 *                 into this variable.
 *  \param pDataType : The type of the data. See objdictdef.h
 *  \param CheckAccess if other than 0, do not read if the data is Write Only
 *                     [Not used today. Put always 0].
 *  \return OD_SUCCESSFUL or SDO abort code. (See file def.h)
 */
UNS32 getODentry( CO_Data* d, 
                  UNS16 wIndex,
		  UNS8 bSubindex,
		  void * pDestData,
		  UNS8 * pExpectedSize,
		  UNS8 * pDataType,
		  UNS8 checkAccess);


/* By this function you can write an entry into the object dictionary\n
 *  \code
 *  // Example usage:
 *  UNS8 B;
 *  B = 0xFF; // set transmission type
 *
 *  retcode = setODentry( (UNS16)0x1800, (UNS8)2, &B, sizeof(UNS8), 1 );
 *  \endocde
 *  \param wIndex The index in the object dictionary where you want to write
 *                an entry
 *  \param bSubindex The subindex of the Index. e.g. mostly subindex 0 is
 *                   used to tell you how many valid entries you can find
 *                   in this index. Look at the canopen standard for further
 *                   information
 *  \param pbData Pointer to the variable that holds the value that should
 *                 be copied into the object dictionary
 *  \param dwSize The size of the value (in Byte).
 *  \param CheckAccess if other than 0, do not read if the data is Read Only or Constant
 *  \return OD_SUCCESSFUL or SDO abort code. (See file def.h)
 */
UNS32 setODentry( CO_Data* d, 
                  UNS16 wIndex,
		  UNS8 bSubindex, 
		  void * pSourceData, 
		  UNS8 * pExpectedSize, 
		  UNS8 checkAccess);


/* Scan the index of object dictionary. Used only by setODentry and getODentry.
 *  *errorCode :  OD_SUCCESSFUL if index foundor SDO abort code. (See file def.h)
 *  Return NULL if index not found. Else : return the table part of the object dictionary.
 */
 const indextable * scanIndexOD (CO_Data* d, UNS16 wIndex, UNS32 *errorCode, ODCallback_t **Callback);

UNS32 RegisterSetODentryCallBack(CO_Data* d, UNS16 wIndex, UNS8 bSubindex, ODCallback_t Callback);

#endif /* __objacces_h__ */
