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

/**
 * Définitions pour le LPC2138.
 */

#if !defined(_LPC2138_DEF_H_)
#define _LPC2138_DEF_H_

#include <stdio.h>

// #include "types.h"

#define BITMASK_0  0x00000000
#define BITMASK_1  0x00000001
#define BITMASK_2  0x00000003
#define BITMASK_4  0x0000000F
#define BITMASK_8  0x000000FF
#define BITMASK_16 0x0000FFFF
#define BITMASK_32 0xFFFFFFFF

typedef volatile unsigned char   REG8;
typedef volatile unsigned char  *REG8_ADDR;
typedef volatile unsigned short  REG16;
typedef volatile unsigned short *REG16_ADDR;
typedef volatile unsigned int    REG32;
typedef volatile unsigned int   *REG32_ADDR;


#define SET_EQ_SET =
#define SET_EQ_CLR =

#define P0_IOPIN_ADDR   0xE0028000
#define P0_IOSET_ADDR   0xE0028004
#define P0_IODIR_ADDR   0xE0028008
#define P0_IOCLR_ADDR   0xE002800C
#define P0_PINSEL0_ADDR 0xE002C000
#define P0_PINSEL1_ADDR 0xE002C004

#define P1_IOPIN_ADDR   0xE0028010
#define P1_IOSET_ADDR   0xE0028014
#define P1_IODIR_ADDR   0xE0028018
#define P1_IOCLR_ADDR   0xE002801C
#define P1_PINSEL2_ADDR 0xE002C014

#define DACR_ADDR       0xE006C000

/* Vectored Interrupt Controller (VIC) */
#define VICVectAddr_ADDR  0xFFFFF030
#define VICVectAddr0_ADDR 0xFFFFF100
#define VICVectCntl0_ADDR 0xFFFFF200
#define VICIntEnable_ADDR 0xFFFFF010

/* External Interrupts */
#define EXTINT_ADDR     0xE01FC140
#define INTWAKE_ADDR    0xE01FC144
#define EXTMODE_ADDR    0xE01FC148
#define EXTPOLAR_ADDR   0xE01FC14C

#ifdef TEST
#include "test_stubs.h"
#endif

/* Vectored Interrupt Controller (VIC) */
#define VICIRQStatus   (*((REG32_ADDR) 0xFFFFF000))
#define VICFIQStatus   (*((REG32_ADDR) 0xFFFFF004))
#define VICRawIntr     (*((REG32_ADDR) 0xFFFFF008))
#define VICIntSelect   (*((REG32_ADDR) 0xFFFFF00C))
#define VICIntEnable   (*((REG32_ADDR) 0xFFFFF010))
#define VICIntEnClr    (*((REG32_ADDR) 0xFFFFF014))
#define VICSoftInt     (*((REG32_ADDR) 0xFFFFF018))
#define VICSoftIntClr  (*((REG32_ADDR) 0xFFFFF01C))
#define VICProtection  (*((REG32_ADDR) 0xFFFFF020))
#define VICVectAddr    (*((REG32_ADDR) 0xFFFFF030))
#define VICDefVectAddr (*((REG32_ADDR) 0xFFFFF034))
#define VICVectAddr0   (*((REG32_ADDR) 0xFFFFF100))
#define VICVectAddr1   (*((REG32_ADDR) 0xFFFFF104))
#define VICVectAddr2   (*((REG32_ADDR) 0xFFFFF108))
#define VICVectAddr3   (*((REG32_ADDR) 0xFFFFF10C))
#define VICVectAddr4   (*((REG32_ADDR) 0xFFFFF110))
#define VICVectAddr5   (*((REG32_ADDR) 0xFFFFF114))
#define VICVectAddr6   (*((REG32_ADDR) 0xFFFFF118))
#define VICVectAddr7   (*((REG32_ADDR) 0xFFFFF11C))
#define VICVectAddr8   (*((REG32_ADDR) 0xFFFFF120))
#define VICVectAddr9   (*((REG32_ADDR) 0xFFFFF124))
#define VICVectAddr10  (*((REG32_ADDR) 0xFFFFF128))
#define VICVectAddr11  (*((REG32_ADDR) 0xFFFFF12C))
#define VICVectAddr12  (*((REG32_ADDR) 0xFFFFF130))
#define VICVectAddr13  (*((REG32_ADDR) 0xFFFFF134))
#define VICVectAddr14  (*((REG32_ADDR) 0xFFFFF138))
#define VICVectAddr15  (*((REG32_ADDR) 0xFFFFF13C))
#define VICVectCntl0   (*((REG32_ADDR) 0xFFFFF200))
#define VICVectCntl1   (*((REG32_ADDR) 0xFFFFF204))
#define VICVectCntl2   (*((REG32_ADDR) 0xFFFFF208))
#define VICVectCntl3   (*((REG32_ADDR) 0xFFFFF20C))
#define VICVectCntl4   (*((REG32_ADDR) 0xFFFFF210))
#define VICVectCntl5   (*((REG32_ADDR) 0xFFFFF214))
#define VICVectCntl6   (*((REG32_ADDR) 0xFFFFF218))
#define VICVectCntl7   (*((REG32_ADDR) 0xFFFFF21C))
#define VICVectCntl8   (*((REG32_ADDR) 0xFFFFF220))
#define VICVectCntl9   (*((REG32_ADDR) 0xFFFFF224))
#define VICVectCntl10  (*((REG32_ADDR) 0xFFFFF228))
#define VICVectCntl11  (*((REG32_ADDR) 0xFFFFF22C))
#define VICVectCntl12  (*((REG32_ADDR) 0xFFFFF230))
#define VICVectCntl13  (*((REG32_ADDR) 0xFFFFF234))
#define VICVectCntl14  (*((REG32_ADDR) 0xFFFFF238))
#define VICVectCntl15  (*((REG32_ADDR) 0xFFFFF23C))

#define P0_IOPIN   (*((REG32_ADDR) P0_IOPIN_ADDR))
#define P0_IOSET   (*((REG32_ADDR) P0_IOSET_ADDR))
#define P0_IODIR   (*((REG32_ADDR) P0_IODIR_ADDR))
#define P0_IOCLR   (*((REG32_ADDR) P0_IOCLR_ADDR))
#define P0_PINSEL0 (*((REG32_ADDR) P0_PINSEL0_ADDR))
#define P0_PINSEL1 (*((REG32_ADDR) P0_PINSEL1_ADDR))

#define P1_IOPIN   (*((REG32_ADDR) P1_IOPIN_ADDR))
#define P1_IOSET   (*((REG32_ADDR) P1_IOSET_ADDR))
#define P1_IODIR   (*((REG32_ADDR) P1_IODIR_ADDR))
#define P1_IOCLR   (*((REG32_ADDR) P1_IOCLR_ADDR))
#define P1_PINSEL2 (*((REG32_ADDR) P1_PINSEL2_ADDR))

#define DACR       (*((REG32_ADDR) DACR_ADDR))

/* External Interrupts */
#define EXTINT   (*((REG32_ADDR) EXTINT_ADDR))
#define INTWAKE  (*((REG32_ADDR) INTWAKE_ADDR))
#define EXTMODE  (*((REG32_ADDR) EXTMODE_ADDR))
#define EXTPOLAR (*((REG32_ADDR) EXTPOLAR_ADDR))


/* Timer 0 */
#define T0IR           (*((REG32_ADDR) 0xE0004000))
#define T0TCR          (*((REG32_ADDR) 0xE0004004))
#define T0TC           (*((REG32_ADDR) 0xE0004008))
#define T0PR           (*((REG32_ADDR) 0xE000400C))
#define T0PC           (*((REG32_ADDR) 0xE0004010))
#define T0MCR          (*((REG32_ADDR) 0xE0004014))
#define T0MR0          (*((REG32_ADDR) 0xE0004018))
#define T0MR1          (*((REG32_ADDR) 0xE000401C))
#define T0MR2          (*((REG32_ADDR) 0xE0004020))
#define T0MR3          (*((REG32_ADDR) 0xE0004024))
#define T0CCR          (*((REG32_ADDR) 0xE0004028))
#define T0CR0          (*((REG32_ADDR) 0xE000402C))
#define T0CR1          (*((REG32_ADDR) 0xE0004030))
#define T0CR2          (*((REG32_ADDR) 0xE0004034))
#define T0CR3          (*((REG32_ADDR) 0xE0004038))
#define T0EMR          (*((REG32_ADDR) 0xE000403C))
#define T0CTCR         (*((REG32_ADDR) 0xE0004070))

/* Timer 1 */
#define T1IR           (*((REG32_ADDR) 0xE0008000))
#define T1TCR          (*((REG32_ADDR) 0xE0008004))
#define T1TC           (*((REG32_ADDR) 0xE0008008))
#define T1PR           (*((REG32_ADDR) 0xE000800C))
#define T1PC           (*((REG32_ADDR) 0xE0008010))
#define T1MCR          (*((REG32_ADDR) 0xE0008014))
#define T1MR0          (*((REG32_ADDR) 0xE0008018))
#define T1MR1          (*((REG32_ADDR) 0xE000801C))
#define T1MR2          (*((REG32_ADDR) 0xE0008020))
#define T1MR3          (*((REG32_ADDR) 0xE0008024))
#define T1CCR          (*((REG32_ADDR) 0xE0008028))
#define T1CR0          (*((REG32_ADDR) 0xE000802C))
#define T1CR1          (*((REG32_ADDR) 0xE0008030))
#define T1CR2          (*((REG32_ADDR) 0xE0008034))
#define T1CR3          (*((REG32_ADDR) 0xE0008038))
#define T1EMR          (*((REG32_ADDR) 0xE000803C))
#define T1CTCR         (*((REG32_ADDR) 0xE0008070))

/* Real Time Clock */
#define ILR            (*((REG8_ADDR) 0xE0024000))
#define CTC            (*((REG16_ADDR) 0xE0024004))
#define CCR            (*((REG8_ADDR) 0xE0024008))
#define CIIR           (*((REG8_ADDR) 0xE002400C))
#define AMR            (*((REG8_ADDR) 0xE0024010))
#define CTIME0         (*((REG32_ADDR) 0xE0024014))
#define CTIME1         (*((REG32_ADDR) 0xE0024018))
#define CTIME2         (*((REG32_ADDR) 0xE002401C))
#define SEC            (*((REG8_ADDR) 0xE0024020))
#define MIN            (*((REG8_ADDR) 0xE0024024))
#define HOUR           (*((REG8_ADDR) 0xE0024028))
#define DOM            (*((REG8_ADDR) 0xE002402C))
#define DOW            (*((REG8_ADDR) 0xE0024030))
#define DOY            (*((REG16_ADDR) 0xE0024034))
#define MONTH          (*((REG8_ADDR) 0xE0024038))
#define YEAR           (*((REG16_ADDR) 0xE002403C))
#define ALSEC          (*((REG8_ADDR) 0xE0024060))
#define ALMIN          (*((REG8_ADDR) 0xE0024064))
#define ALHOUR         (*((REG8_ADDR) 0xE0024068))
#define ALDOM          (*((REG8_ADDR) 0xE002406C))
#define ALDOW          (*((REG8_ADDR) 0xE0024070))
#define ALDOY          (*((REG16_ADDR) 0xE0024074))
#define ALMON          (*((REG8_ADDR) 0xE0024078))
#define ALYEAR         (*((REG16_ADDR) 0xE002407C))
#define PREINT         (*((REG16_ADDR) 0xE0024080))
#define PREFRAC        (*((REG16_ADDR) 0xE0024084))

/* SPI Registers */
#define S0SPCR         (*((REG32_ADDR) 0xE0020000))
#define S0SPSR         (*((REG32_ADDR) 0xE0020004))
#define S0SPDR         (*((REG32_ADDR) 0xE0020008))
#define S0SPCCR        (*((REG32_ADDR) 0xE002000C))
#define S0SPINT        (*((REG32_ADDR) 0xE002001C))

/* SSP Registers */
#define SSPCR0         (*((REG32_ADDR) 0xE0068000))
#define SSPCR1         (*((REG32_ADDR) 0xE0068004))
#define SSPDR          (*((REG32_ADDR) 0xE0068008))
#define SSPSR          (*((REG32_ADDR) 0xE006800C))
#define SSPCPSR        (*((REG32_ADDR) 0xE0068010))
#define SSPIMSC        (*((REG32_ADDR) 0xE0068014))
#define SSPRIS         (*((REG32_ADDR) 0xE0068018))
#define SSPMIS         (*((REG32_ADDR) 0xE006801C))
#define SSPICR         (*((REG32_ADDR) 0xE0068020))


typedef enum {
    LPC2138_MODE_INPUT  = 0,
    LPC2138_MODE_OUTPUT = 1
} LPC2138_MODE;

typedef enum {
    P0 = 0,
    P1 = 1
} LPC2138_PORT;

/* === Fonctions "#define" génériques ======================================= */

#define _cat(a, b) a##b
#define _CAT(a, b) _cat(a, b)

#define _PIN(pin)  LPC2138_##pin
#define _PORT(pin) LPC2138_##pin##_PORT
#define _SIZE(pin) LPC2138_##pin##_SIZE

/* ((P[0|1]_IOPIN >> pin) & BITMASK_[0-32]) */
#define lpc2138_get(pin) \
    ((_CAT(_PORT(pin), _IOPIN) >> _PIN(pin)) & _CAT(BITMASK_, _SIZE(pin)))

#define lpc2138_set(pin, i) \
    { if (_SIZE(pin) == 1) { lpc2138_set_bit(pin, i); } \
        else { lpc2138_set_all(pin, i); } }

#define lpc2138_set_bit(pin, i) \
    { if ((i) == 1) { _CAT(_PORT(pin), _IOSET) SET_EQ_SET (1 << _PIN(pin)); } \
           else { _CAT(_PORT(pin), _IOCLR) SET_EQ_CLR (1 << _PIN(pin)); } }

#define lpc2138_set_all(pin, i) \
    (_CAT(_PORT(pin), _IOPIN) = \
        (_CAT(_PORT(pin), _IOPIN) & \
            ~(_CAT(BITMASK_, _SIZE(pin)) << _PIN(pin))) | ((i) << _PIN(pin)))

/* Identique à lpc2138_set(p, f, nbits, i) sans effet secondaire (plus lent). */
#define lpc2138_set_SAFE_(pin, i) \
    ((_SIZE(pin) == 1) && lpc2138_set_bit_SAFE_(pin, (i)) || \
        lpc2138_set_all(pin, (i)))

/* Identique à lpc2138_set_bit(p, f, i) sans effet secondaire (plus lent). */
#define lpc2138_set_bit_SAFE_(pin, i) \
    ((i == 1) && (_CAT(_PORT(pin), _IOSET) SET_EQ_SET (1 << _PIN(pin))) || \
        (_CAT(_PORT(pin), _IOCLR) SET_EQ_CLR (1 << _PIN(pin))))

#define lpc2138_set_mode(pin, mode) \
    (_CAT(_PORT(pin), _IODIR) = (mode == LPC2138_MODE_OUTPUT) ? \
        (_CAT(_PORT(pin), _IODIR)|(_CAT(BITMASK_, _SIZE(pin)) << _PIN(pin))) : \
        (_CAT(_PORT(pin), _IODIR) & ~(_CAT(BITMASK_, _SIZE(pin)) << _PIN(pin))))

#define lpc2138_set_pinsel(pin, func) \
    lpc2138_pinsel_set(_PIN(pin), _PORT(pin), _SIZE(pin), func)

/* === Fonctions pinout "#define" par défaut ================================ */

#ifndef lpc2138_uart0_tx_set_pinsel
#define lpc2138_uart0_tx_set_pinsel(func)          lpc2138_set_pinsel(uart0_tx, func)
#endif

#ifndef lpc2138_uart0_rx_set_pinsel
#define lpc2138_uart0_rx_set_pinsel(func)          lpc2138_set_pinsel(uart0_rx, func)
#endif

#ifndef lpc2138_cs_s1d13706_get
#define lpc2138_cs_s1d13706_get()                  lpc2138_get       (cs_s1d13706)
#endif

#ifndef lpc2138_cs_s1d13706_set
#define lpc2138_cs_s1d13706_set(i)                 lpc2138_set       (cs_s1d13706, i)
#endif

#ifndef lpc2138_cs_s1d13706_set_mode
#define lpc2138_cs_s1d13706_set_mode(mode)         lpc2138_set_mode  (cs_s1d13706, mode)
#endif

#ifndef lpc2138_cs_s1d13706_set_pinsel
#define lpc2138_cs_s1d13706_set_pinsel(func)       lpc2138_set_pinsel(cs_s1d13706, func)
#endif

#ifndef lpc2138_cs_sja1000_get
#define lpc2138_cs_sja1000_get()                   lpc2138_get       (cs_sja1000)
#endif

#ifndef lpc2138_cs_sja1000_set
#define lpc2138_cs_sja1000_set(i)                  lpc2138_set       (cs_sja1000, i)
#endif

#ifndef lpc2138_cs_sja1000_set_mode
#define lpc2138_cs_sja1000_set_mode(mode)          lpc2138_set_mode  (cs_sja1000, mode)
#endif

#ifndef lpc2138_cs_sja1000_set_pinsel
#define lpc2138_cs_sja1000_set_pinsel(func)        lpc2138_set_pinsel(cs_sja1000, func)
#endif

#ifndef lpc2138_wait_get
#define lpc2138_wait_get()                         lpc2138_get       (wait)
#endif

#ifndef lpc2138_wait_set
#define lpc2138_wait_set(i)                        lpc2138_set       (wait, i)
#endif

#ifndef lpc2138_wait_set_mode
#define lpc2138_wait_set_mode(mode)                lpc2138_set_mode  (wait, mode)
#endif

#ifndef lpc2138_wait_set_pinsel
#define lpc2138_wait_set_pinsel(func)              lpc2138_set_pinsel(wait, func)
#endif

#ifndef lpc2138_bhe_get
#define lpc2138_bhe_get()                          lpc2138_get       (bhe)
#endif

#ifndef lpc2138_bhe_set
#define lpc2138_bhe_set(i)                         lpc2138_set       (bhe, i)
#endif

#ifndef lpc2138_bhe_set_mode
#define lpc2138_bhe_set_mode(mode)                 lpc2138_set_mode  (bhe, mode)
#endif

#ifndef lpc2138_bhe_set_pinsel
#define lpc2138_bhe_set_pinsel(func)               lpc2138_set_pinsel(bhe, func)
#endif

#ifndef lpc2138_interrupt_sja1000_get
#define lpc2138_interrupt_sja1000_get()            lpc2138_get       (interrupt_sja1000)
#endif

#ifndef lpc2138_interrupt_sja1000_set
#define lpc2138_interrupt_sja1000_set(i)           lpc2138_set       (interrupt_sja1000, i)
#endif

#ifndef lpc2138_interrupt_sja1000_set_mode
#define lpc2138_interrupt_sja1000_set_mode(mode)   lpc2138_set_mode  (interrupt_sja1000, mode)
#endif

#ifndef lpc2138_interrupt_sja1000_set_pinsel
#define lpc2138_interrupt_sja1000_set_pinsel(func) lpc2138_set_pinsel(interrupt_sja1000, func)
#endif

#ifndef lpc2138_redgreenled_get
#define lpc2138_redgreenled_get()            lpc2138_get       (redgreenled)
#endif

#ifndef lpc2138_redgreenled_set
#define lpc2138_redgreenled_set(i)           lpc2138_set       (redgreenled, i)
#endif

#ifndef lpc2138_redgreenled_set_mode
#define lpc2138_redgreenled_set_mode(mode)   lpc2138_set_mode  (redgreenled, mode)
#endif

#ifndef lpc2138_redgreenled_set_pinsel
#define lpc2138_redgreenled_set_pinsel(func) lpc2138_set_pinsel(redgreenled, func)
#endif

#ifndef lpc2138_dac0_set
#define lpc2138_dac0_set()                         lpc2138_set       (dac0, i)
#endif

#ifndef lpc2138_dac0_set_value
#define lpc2138_dac0_set_value(i)                  DACR =            ((1 << 16) | ((i & 0x3FF) << 6))
#endif

#ifndef lpc2138_dac0_set_pinsel
#define lpc2138_dac0_set_pinsel(func)              lpc2138_set_pinsel(dac0, func)
#endif

#ifndef lpc2138_spi0_set
#define lpc2138_spi0_set()                         lpc2138_set       (spi0, i)
#endif

#ifndef lpc2138_spi0_set_value
#define lpc2138_spi0_set_value(i)                  SSPDR = i
#endif

#ifndef lpc2138_spi0_set_pinsel
#define lpc2138_spi0_set_pinsel(func)              lpc2138_set_pinsel(spi0, func)
#endif

#ifndef lpc2138_ale_get
#define lpc2138_ale_get()                          lpc2138_get       (ale)
#endif

#ifndef lpc2138_ale_set
#define lpc2138_ale_set(i)                         lpc2138_set       (ale, i)
#endif

#ifndef lpc2138_ale_set_mode
#define lpc2138_ale_set_mode(mode)                 lpc2138_set_mode  (ale, mode)
#endif
#ifndef lpc2138_ale_set_pinsel
#define lpc2138_ale_set_pinsel(func)               lpc2138_set_pinsel(ale, func)
#endif

#ifndef lpc2138_rd_get
#define lpc2138_rd_get()                           lpc2138_get       (rd)
#endif

#ifndef lpc2138_rd_set
#define lpc2138_rd_set(i)                          lpc2138_set       (rd, i)
#endif

#ifndef lpc2138_rd_set_mode
#define lpc2138_rd_set_mode(mode)                  lpc2138_set_mode  (rd, mode)
#endif

#ifndef lpc2138_rd_set_pinsel
#define lpc2138_rd_set_pinsel(func)                lpc2138_set_pinsel(rd, func)
#endif

#ifndef lpc2138_wr_get
#define lpc2138_wr_get()                           lpc2138_get       (wr)
#endif

#ifndef lpc2138_wr_set
#define lpc2138_wr_set(i)                          lpc2138_set       (wr, i)
#endif

#ifndef lpc2138_wr_set_mode
#define lpc2138_wr_set_mode(mode)                  lpc2138_set_mode  (wr, mode)
#endif

#ifndef lpc2138_wr_set_pinsel
#define lpc2138_wr_set_pinsel(func)                lpc2138_set_pinsel(wr, func)
#endif

#ifndef lpc2138_data_get
#define lpc2138_data_get()                         lpc2138_get       (data)
#endif

#ifndef lpc2138_data_set
#define lpc2138_data_set(i)                        lpc2138_set       (data, i)
#endif

#ifndef lpc2138_data_set_mode
#define lpc2138_data_set_mode(mode)                lpc2138_set_mode  (data, mode)
#endif

#ifndef lpc2138_data_set_pinsel
#define lpc2138_data_set_pinsel(func)              lpc2138_set_pinsel(data, func)
#endif

#ifndef lpc2138_addresses_get
#define lpc2138_addresses_get()                    ((lpc2138_get      (a17_mr) << 17) | \
                                                    (lpc2138_get      (a16)    << 16) | \
                                                    (lpc2138_get      (a0_a15)))
#endif

#ifndef lpc2138_addresses_set
#define lpc2138_addresses_set(i)                     lpc2138_set      (a17_mr, ((i >> 17) & BITMASK_1)); \
                                                     lpc2138_set      (a16,    ((i >> 16) & BITMASK_1)); \
                                                     lpc2138_set      (a0_a15, ((i)       & BITMASK_16))
#endif

#ifndef lpc2138_addresses_set_mode
#define lpc2138_addresses_set_mode(mode)            (lpc2138_set_mode  (a17_mr, mode), \
                                                     lpc2138_set_mode  (a16,    mode), \
                                                     lpc2138_set_mode  (a0_a15, mode))
#endif

#ifndef lpc2138_addresses_set_pinsel
#define lpc2138_addresses_set_pinsel(func)          (lpc2138_set_pinsel(a17_mr, func), \
                                                     lpc2138_set_pinsel(a16,    func), \
                                                     lpc2138_set_pinsel(a0_a15, func))
#endif

#define CMR 1
#define RRB 2	// 1=released message in fifo are released
#define AT 1	// 1= cancel next tranmission
#define SR 2
#define TBS 2	// 1=released the cpu may write a message in the transmit buffer
#define SRR 4	// 1=present  a message shall be transmit and receive sim
#define TR 0	// 1=present a message shall be transmit


/*
	FLASH
*/
#define EE_SEC_L		1		// Flash sector where EEPROM begins (see UM for details)
#define EE_SEC_H		3	  	// Flash sector where EEPROM ends (see UM for details)
#define EE_ADDR_L		0x00001000	// Must match the EE_SEC_L Flash sector start address
#define EE_ADDR_H		0x00003FFF 	// Must match the EE_SEC_H Flash sector end address
#define EE_CCLK			60000		// system clock cclk expressed in kHz (5*12 MHz)
#define EE_BUFFER_SIZE	        256
#define EE_START_MASK           0xFFFFFF00
#define EE_BUFFER_MASK          0x000000F0


#endif
