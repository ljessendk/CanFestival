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

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>

#include "lpc2138_pinout.h"
#include "lpc2138_defs.h"
#include "lpc2138.h"

#include "sja1000.h"


#define CYGNUM_HAL_INTERRUPT_1 CYGNUM_HAL_INTERRUPT_EINT1
#define CYGNUM_HAL_PRI_HIGH    0


cyg_uint32 interrupt_1_isr(cyg_vector_t vector, cyg_addrword_t data);
void interrupt_1_dsr(cyg_vector_t   vector,
                            cyg_ucount32   count,
                            cyg_addrword_t data);


/* Interrupt for CAN device. */
static cyg_interrupt interrupt_1;
static cyg_handle_t  interrupt_1_handle;


void init_sja1000(void)
{
    do
    {
        sja1000_write(MOD, 1<<RM);                  /* demande reset */
    }
    while ((sja1000_read(MOD) & (1<<RM)) == 0);   /* loop until reset good */

/*
    sja1000_write(bustiming0, ((0<<SJW1)|(0<<SJW0)|(0<<BRP5)|(0<<BRP4)|(0<<BRP3)|(0<<BRP2)|(0<<BRP1)|(0<<BRP0)));
    sja1000_write(bustiming1, ((1<<SAM)|(0<<TSEG22)|(1<<TSEG21)|(0<<TSEG20)|(0<<TSEG13)|(1<<TSEG12)|(0<<TSEG11)|(0<<TSEG10)));
*/

/* OUTPUT CONTROL REGISTER */
    sja1000_write(outputcontrol, ((1<<OCTP1)|(1<<OCTN1)|(0<<OCPOL1)|(1<<OCTP0)|(1<<OCTN0)|(0<<OCPOL0)|(1<<OCMODE1)|(0<<OCMODE0)));


    sja1000_write(clockdivider, ((1<<CANmode)|(1<<CBP)|(1<<RXINTEN)|(0<<clockoff)|(1<<CD2)|(1<<CD1)|(1<<CD0)));
    sja1000_write(16,  0x01);   /* 0 code all accept block high bit */
    sja1000_write(17,  0x00);   /* 0 all accept block high bit */
    sja1000_write(18,  0x00);   /* 0 all accept block high bit */
    sja1000_write(19,  0x00);   /* 0 all accept block high bit */
    sja1000_write(20,  0xFE);   /* 1 mask */
    sja1000_write(21,  0xFF);
    sja1000_write(22,  0xFF);
    sja1000_write(23,  0xFF);
    sja1000_write(IER, 0x01);
    sja1000_write(clockdivider, ((1<<CANmode)|(1<<CBP)|(1<<RXINTEN)|(0<<clockoff)|(1<<CD2)|(1<<CD1)|(0<<CD0)));

    do
    {
        sja1000_write(MOD, (1<<AFM)|(0<<STM)|(0<<RM));
    }
    while ((sja1000_read(MOD) & (1<<RM)) == 1);   /* loop until reset gone */
}


/***************************************************************************/


void init_interrupts(void)
{
    cyg_vector_t   interrupt_1_vector   = CYGNUM_HAL_INTERRUPT_1;
    cyg_priority_t interrupt_1_priority = CYGNUM_HAL_PRI_HIGH;

    cyg_interrupt_create(
        interrupt_1_vector,
        interrupt_1_priority,
        (cyg_addrword_t) 0,
        (cyg_ISR_t *) &interrupt_1_isr,
        (cyg_DSR_t *) &interrupt_1_dsr,
        &interrupt_1_handle,
        &interrupt_1);

    cyg_interrupt_attach(interrupt_1_handle);

    cyg_interrupt_acknowledge(interrupt_1_vector);

    cyg_interrupt_unmask(interrupt_1_vector);
}

/* External Interrupt 1 Service */
void eint1_srv(void) /*__irq*/ 
{
	//++intrp_count;                              // increment interrupt count
	EXTINT      = 2;                            // Clear EINT1 interrupt flag
	VICVectAddr = 0;                            // Acknowledge Interrupt
}


/* Initialize EINT1 Interrupt Pin */
void init_eint1(void)
{
	EXTMODE       = 0x2;                         // Edge sensitive mode on EINT1
	EXTPOLAR      = 0;                           // falling edge sensitive
	P0_PINSEL0   |= 2 << 28;                     // Enable EINT1 on GPIO_0.14
	VICVectAddr0  = (unsigned long) eint1_srv;   // set interrupt vector in VIC 0
	VICVectCntl0  = 0x20 | 15;                   // use VIC 0 for EINT1 Interrupt
	EXTINT        = 2;
	VICIntEnable  = 1 << 15;                     // Enable EINT1 Interrupt
}


/*
{
	unsigned char byte=sja1000_read(0x02);

	if (byte & 0x01)
	{
		// RXFIFO full
	}
	
	if (byte & 0x02)
	{
		// overrun
	}
	
	if (byte & 0x04)
	{
		// the cpu may write a msg
	}
	
	if (byte & 0x08)
	{
		// tx complete
	}
	
	if (byte & 0x10)
	{
		// receiving
	}
	
	if (byte & 0x20)
	{
		// transmitting
	}
	
	if (byte & 0x40)
	{
		// at least one of the error counter has reached or exceeeded
		// the CPU warning limit
	}
	
	if (byte & 0x80)
	{
		// bus off
	}
}
*/


