
/* File generated by gen_cfile.py. Should not be modified. */

#ifndef MASTERDIC_H
#define MASTERDIC_H

#include "data.h"

/* Prototypes of function provided by object dictionnary */
UNS32 masterdic_valueRangeTest (UNS8 typeValue, void * value);
const indextable * masterdic_scanIndexOD (CO_Data *d, UNS16 wIndex, UNS32 * errorCode);

/* Master node data struct */
extern CO_Data masterdic_Data;
extern INTEGER16 position_1;		/* Mapped at index 0x2000, subindex 0x00*/
extern INTEGER16 position_2;		/* Mapped at index 0x2001, subindex 0x00*/
extern INTEGER16 position_3;		/* Mapped at index 0x2002, subindex 0x00*/
extern UNS32 counter_1;		/* Mapped at index 0x3000, subindex 0x00*/
extern UNS32 counter_2;		/* Mapped at index 0x3001, subindex 0x00*/
extern UNS32 counter_3;		/* Mapped at index 0x3003, subindex 0x00*/

#endif // MASTERDIC_H