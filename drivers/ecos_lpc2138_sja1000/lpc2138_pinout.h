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
 * Définitions du brochage LPC2138 par défaut.
 */
#if !defined(_LPC2138_PINOUT_H_)
#define _LPC2138_PINOUT_H_

#define LPC2138_uart0_tx                0 /* Pin 0 */
#define LPC2138_uart0_tx_PORT          P0
#define LPC2138_uart0_tx_SIZE           1

#define LPC2138_uart0_rx                1 /* Pin 1 */
#define LPC2138_uart0_rx_PORT          P0
#define LPC2138_uart0_rx_SIZE           1

#define LPC2138_cs_sja1000              4 /* Pin 4 */
#define LPC2138_cs_sja1000_PORT        P0
#define LPC2138_cs_sja1000_SIZE         1

#define LPC2138_cs_s1d13706             5 /* Pin 5 */
#define LPC2138_cs_s1d13706_PORT       P0
#define LPC2138_cs_s1d13706_SIZE        1

#define LPC2138_wait                    7 /* Pin 7 */
#define LPC2138_wait_PORT              P0
#define LPC2138_wait_SIZE               1

#define LPC2138_uart1_tx                8 /* Pin 8 */
#define LPC2138_uart1_tx_PORT          P0
#define LPC2138_uart1_tx_SIZE           1

#define LPC2138_uart1_rx                9 /* Pin 9 */
#define LPC2138_uart1_rx_PORT          P0
#define LPC2138_uart1_rx_SIZE           1

#define LPC2138_bhe                    10 /* Pin 10 */
#define LPC2138_bhe_PORT               P0
#define LPC2138_bhe_SIZE                1

#define LPC2138_a17_mr                 12 /* Pin 12 */
#define LPC2138_a17_mr_PORT            P0
#define LPC2138_a17_mr_SIZE             1

#define LPC2138_a16                    13 /* Pin 13 */
#define LPC2138_a16_PORT               P0
#define LPC2138_a16_SIZE                1

#define LPC2138_interrupt_sja1000      14 /* Pin 14 */
#define LPC2138_interrupt_sja1000_PORT P0
#define LPC2138_interrupt_sja1000_SIZE  1

#define LPC2138_data                   16 /* Pins 16-23 */
#define LPC2138_data_PORT              P0
#define LPC2138_data_SIZE               8

#define LPC2138_dac0                   25 /* Pin 25 */
#define LPC2138_dac0_PORT              P0
#define LPC2138_dac0_SIZE               1

#define LPC2138_ale                    26 /* Pin 26 */
#define LPC2138_ale_PORT               P0
#define LPC2138_ale_SIZE                1

#define LPC2138_redgreenled             27 /* Pin 27 */
#define LPC2138_redgreenled_PORT        P0
#define LPC2138_redgreenled_SIZE        2

#define LPC2138_rd                     29 /* Pin 29 */
#define LPC2138_rd_PORT                P0
#define LPC2138_rd_SIZE                 1

#define LPC2138_wr                     30 /* Pin 30 */
#define LPC2138_wr_PORT                P0
#define LPC2138_wr_SIZE                 1

#define LPC2138_a0_a15                 16 /* Pins 16-31 */
#define LPC2138_a0_a15_PORT            P1
#define LPC2138_a0_a15_SIZE            16

#define LPC2138_addresses_SIZE         18

#endif
