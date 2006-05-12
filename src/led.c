/*
This file is part of CanFestival, a library implementing CanOpen Stack.

 Author: CANopen Canada (canfestival@canopencanada.ca)

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

	DS-303-3
	LED implementation
*/

#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <signal.h>

#include <applicfg.h>

#include <data.h>
#include <can_driver.h>

#include "led.h"


void led_start_timer(CO_Data *, UNS32 t0);
void led_stop_timer(void);
void led_set_green(UNS8 on);
void led_set_red(UNS8 on);
void led_callback(CO_Data* d, UNS32 id);


// 0 = always off, 1 = always on, 2 = flashing
static UNS8 led_state_red, led_state_green; 

static UNS16 led_sequence_red, led_seq_index_red;
static UNS16 led_sequence_green, led_seq_index_green;

static UNS8 led_error_code = LED_NO_ERROR;

const char *led_sequence_table[6] = // up and downs of the sequence
{
	"01",           // flickering
	"01",           // blinking
	"100000",       // single flash
	"10100000",     // double flash
	"1010100000",   // triple flash
	"101010100000"  // quadruple flash
};


void led_set_state(CO_Data *d, int state)
{
printf("led_set_state(%x)\n", state);

	switch(state)
	{
		case Initialisation:
/*
	must create a timer for the leds with the scheduler
*/
			break;
		case LED_AUTOBITRATE:
			led_state_green = 2;
			led_sequence_green = 0;
			break;
		case Pre_operational:
			led_state_green = 2;
			led_sequence_green = 1;
			break;
		case Stopped:
			led_state_green = 2;
			led_sequence_green = 2;
			break;
		case LED_PRG_DOWNLOAD:
			led_state_green = 2;
			led_sequence_green = 4;
			break;
		case Operational:
			led_state_green = 1;
			break;
	}

	if (state == LED_AUTOBITRATE)
		led_start_timer(d, 50);

	else if (led_state_green < 2  &&  led_state_red < 2)
	{
		led_stop_timer();

		//led_set_green(led_state_green);
		//led_set_red(led_state_red);
	}

	else
		led_start_timer(d, 200);
}


void led_set_error(CO_Data *d, UNS8 error)
{
	if (error == LED_NO_ERROR)
	{
		led_error_code = error;

		led_state_green = 0;
	}

	else if (error == LED_AUTOBITRATE)
	{
		led_error_code = error;

		led_state_red = 2;
		led_sequence_red = 0;

		led_start_timer(d, 50);
	}

	else if (error > led_error_code)
	{
		led_error_code = error;

		if (error & LED_INVALID_CONFIG)
		{
			led_state_red = 2;
			led_sequence_red = 1;
		}
		
		else if (error & LED_WARNING_LIMIT_REACH)
		{
			led_state_red = 2;
			led_sequence_red = 2;
		}

		else if (error & LED_ERROR_CTRL_EVENT)
		{
			led_state_red = 2;
			led_sequence_red = 3;
		}

		else if (error & LED_SYNC_ERROR)
		{
			led_state_red = 2;
			led_sequence_red = 4;
		}

		else if (error & LED_EVENT_TIMER_ERROR)
		{
			led_state_red = 2;
			led_sequence_green = 5;
		}

		else if (error & LED_BUS_OFF)
		{
			led_state_green = 1;
		}

		led_start_timer(d, 200);
		//led_set_red(led_state_red);
	}

	if (led_state_green < 2  &&  led_state_red < 2)
	{
		led_stop_timer();

		//led_set_green(led_state_green);
		//led_set_red(led_state_red);
	}
}


void led_start_timer(CO_Data* d, UNS32 tm)
{
	SetAlarm(d, 0, &led_callback, MS_TO_TIMEVAL(tm), MS_TO_TIMEVAL(tm));

	led_seq_index_red = 0;
	led_seq_index_green = 0;
}


void led_stop_timer(void)
{
}


void led_callback(CO_Data *d, UNS32 id)
{
	UNS8 bits = 0;

	// RED LED
	if (led_sequence_table[led_sequence_red][led_seq_index_red] == '1')
	{
		if (led_state_red > 0)
			bits = 1;
			/* led_set_red(1); */
	}
	else	
	{	
		/*if (led_state_red != 1)
			led_set_red(0);*/
	}

	led_seq_index_red++;
	if (led_seq_index_red > strlen(led_sequence_table[led_sequence_red]))
		led_seq_index_red = 0;

	// GREEN LED
	if (led_sequence_table[led_sequence_green][led_seq_index_green] == '1')
	{
		if (led_state_green > 0)
			bits = bits | 2;
			/* led_set_green(1); */
	}
	else	
	{	
		/* if (led_state_green != 1)
			led_set_green(0); */
	}

	led_seq_index_green++;
	if (led_seq_index_green > strlen(led_sequence_table[led_sequence_green]))
		led_seq_index_green = 0;

	led_set_redgreen(d, bits);
}





/*
char state(state);


Input function to set all the bihaviour indicator
typical state are:
	NoError
		RedLED=off
	AutoBitRate_LSS
		RedLED=flickering
		GreenLED=flickering
	InvalidConfiguration
		RedLED=blinking
	WarningLimitReached
		RedLED=singleflash
	ErrorControlEvent
		RedLED=doubleflash
	SyncError
		RedLED=tripleflash
	EventTimerError
		RedLED=quadrupleflash
	BusOFF
		RedLED=on
	PRE_OPERATIONAL	
		GreenLED=blinking
	STOPPED
		GreenLED=singleflash
	Programm_Firmware_download
		GreenLED=tripleflash
	OPERATIONNAL
		GreenLED=on
*/

/*
case LEDbihaviour:
	on

	flickeringerror
		RedLED(on)
		RedLED(off)
	flickeringerror
		GreenLED(off)
		GreenLED(on)
	blinking

	singleflash

	doubleflash

	tripleflash

	quadrupleflash

	off
*/

/*
char  LED(bitLEDs);
*/

/*
Output function to call the driver.
	if bit=0, then turn LED Off
	if bit=1, then turn LED On

bit#	color		name
0	red		error/status
1	green		run/status
2
3
4
5
6	
7
*/

