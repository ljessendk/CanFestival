/*
This file is part of CanFestival, a library implementing CanOpen Stack.

 Author: Christian Fortin (canfestival@canopencanada.ca)

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

#if !defined(_SJA1000_H_)
#define _SJA1000_H_

#define MOD 0
#define RM 0 /* RM 1=reset 0 = normal */
#define bustiming0 6
#define OCMODE1 1
#define OCMODE0 0

#define SJW1 7
#define SJW0 6
#define BRP5 5
#define BRP4 4
#define BRP3 3
#define BRP2 2
#define BRP1 1
#define BRP0 0

#define CDO 3 /* 1=clear data overun status bit */
#define SAM 7
#define TSEG22 6
#define TSEG21 5
#define TSEG20 4
#define TSEG13 3
#define TSEG12 2
#define TSEG11 1
#define TSEG10 0
#define OCTP1 7
#define OCTN1 6
#define OCPOL1 5
#define OCTP0 4
#define OCTN0 3
#define OCPOL0 2
#define OCMODE1 1
#define OCMODE0 0
#define clockdivider 31
#define CANmode 7 /* 1= PELICAN */
#define CBP 6 /* 1= bypass comparator */
#define RXINTEN 5 /* 1= receive interrupt from tx */
#define clockoff 3 /* 1= disabled */
#define CD2 2
#define CD1 1
#define CD0 0

#define AFM 3 /* AFM 1=single 0=dual acceptance filter */
#define STM 2 /* STM 1=self test 0= norma */
#define IER 4
#define bustiming1 7
#define outputcontrol 8
#define TXIDENTIFIER1 17
#define TXIDENTIFIER2 18


#endif
