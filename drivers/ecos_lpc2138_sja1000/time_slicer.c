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

#include <stdlib.h>

#include <sys/time.h>
#include <signal.h>

#include <cyg/kernel/kapi.h>
#include <cyg/hal/hal_arch.h>

#include "applicfg.h"

#include <timer.h>


#define max(a,b) a>b?a:b
#define min(a,b) a<b?a:b


// ---------  The timer table ---------
s_timer_entry timers[MAX_NB_TIMER] = {{TIMER_FREE, NULL, NULL, 0, 0, 0},};
TIMEVAL total_sleep_time = TIMEVAL_MAX;
TIMER_HANDLE last_timer_raw = -1;

cyg_alarm_t chrono_timer;
cyg_alarm alarm_timer;
cyg_handle_t alarm_handle_timer;
cyg_handle_t rtclock, rtcounter;

void init_timer(void)
{
	rtclock = cyg_real_time_clock();
	cyg_clock_to_counter(rtclock, &rtcounter);

	cyg_alarm_create(rtcounter,
                         &chrono_timer,
                         0, 
                         &alarm_handle_timer,
                         &alarm_timer);
}


unsigned int getElapsedTime(void)
{
	// HCS12 main timer :  Increment every 4 us.
	return 0; // IO_PORTS_16(TCNTH);
}


void setTimer(TIMEVAL tm /* in ms? s? us? */)
{
/*
	this function is required by the library to make the scheduler work
	properly
	it is called by SetAlarm() (in src/timer.c)

	the interrupt vector must call TimeDispatch() (in src/timer.c)
*/
	/*
		value of alarm time for eCos
		  1 = 10 msec
		100 = 1 sec

#define MS_TO_TIMEVAL(ms) ms/10
#define US_TO_TIMEVAL(us) us*4

	*/

	cyg_alarm_initialize(alarm_handle_timer, tm, 0);
}


void chrono_timer(cyg_handle_t alarm, cyg_addrword_t data)
{
	int i;
	int next_wakeup = TIMEVAL_MAX; // used to compute when should normaly occur next wakeup
	// First run : change timer state depending on time
	// Get time since timer signal
	int overrun = getElapsedTime();
	
	int real_total_sleep_time = total_sleep_time + overrun;
	
	for(i=0; i <= last_timer_raw; i++)
	{
		s_timer_entry *row = (timers+i);

		if (row->state & TIMER_ARMED) // if row is active
		{
			if (row->val <= real_total_sleep_time) // to be trigged
			{
				if (!row->interval) // if simply outdated
				{
					row->state = TIMER_TRIG; // ask for trig
				}
				else // or period have expired
				{
					// set val as interval, with overrun correction
					row->val = row->interval - (overrun % row->interval);
					row->state = TIMER_TRIG_PERIOD; // ask for trig, periodic
					// Check if this new timer value is the soonest
					next_wakeup = min(row->val,next_wakeup);
				}
			}
			else
			{
				// Each armed timer value in decremented.
				row->val -= real_total_sleep_time;

				// Check if this new timer value is the soonest
				next_wakeup = min(row->val,next_wakeup);
			}
		}
	}
	
	// Remember how much time we should sleep.
	total_sleep_time = next_wakeup;

	// Set timer to soonest occurence
	setTimer(next_wakeup);

	// Then trig them or not.
	for(i=0; i<=last_timer_raw; i++)
	{
		s_timer_entry *row = (timers+i);

		if (row->state & TIMER_TRIG)
		{
			row->state &= ~TIMER_TRIG; // reset trig state (will be free if not periodic)
			(*row->callback)(row->d, row->id); // trig !
		}
	}
}




TIMER_HANDLE SetAlarm(CO_Data* d, UNS32 id, TimerCallback_t callback, TIMEVAL value, TIMEVAL period)
{
	int i;
	TIMER_HANDLE row_number = TIMER_NONE;

	if (callback == NULL) // nothing to store
		return TIMER_NONE;

	// in order to decide new timer setting we have to run over all timer rows
	
	for(i=0; i <= last_timer_raw + 1 && i < MAX_NB_TIMER; i++)
	{
		s_timer_entry *row = (timers+i);

		if (row->state == TIMER_FREE) // an empty row
		{	// just store
			row->callback = callback;
			row->d = d;
			row->id = id;
			row->val = value;
			row->interval = period;
			row->state = TIMER_ARMED;

			row_number = i;
			break;
		}
	}
	
	if (row_number != TIMER_NONE) // if successfull
	{
		if (row_number == last_timer_raw + 1) last_timer_raw++;
		
		// set next wakeup alarm if new entry is sooner than others, or if it is alone
		unsigned int real_timer_value = min(value, TIMEVAL_MAX);
		unsigned int elapsed_time = getElapsedTime();

		if (total_sleep_time > elapsed_time && total_sleep_time - elapsed_time > real_timer_value)
		{
			total_sleep_time = elapsed_time + real_timer_value;
			setTimer(real_timer_value);
		}

		return row_number;
	}

	return TIMER_NONE;
}

// ---------  Use this to remove an alarm ---------
TIMER_HANDLE DelAlarm(TIMER_HANDLE handle)
{
	if (handle != TIMER_NONE)
	{
		if (handle == last_timer_raw) 
			last_timer_raw--;
		timers[handle].state = TIMER_FREE; 		
	}
	else 
	{
	}

	return TIMER_NONE;
}
