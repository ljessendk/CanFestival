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

//#define DEBUG_WAR_CONSOLE_ON
//#define DEBUG_ERR_CONSOLE_ON

#include <applicfg.h>
#include "timer.h"

// ---------  The timer table ---------
s_timer_entry timers[MAX_NB_TIMER] = {{TIMER_FREE, NULL, NULL, 0, 0, 0},};
//
TIMEVAL total_sleep_time = TIMEVAL_MAX;
TIMER_HANDLE last_timer_raw = -1;

#define max(a,b) a>b?a:b
#define min(a,b) a<b?a:b

// ---------  Use this to declare a new alarm ---------
TIMER_HANDLE SetAlarm(CO_Data* d, UNS32 id, TimerCallback_t callback, TIMEVAL value, TIMEVAL period)
{
	//printf("SetAlarm(UNS32 id=%d, TimerCallback_t callback=%x, TIMEVAL value=%d, TIMEVAL period=%d)\n", id, callback, value, period);
	TIMER_HANDLE i;
	TIMER_HANDLE row_number = TIMER_NONE;

	// in order to decide new timer setting we have to run over all timer rows
	for(i=0; i <= last_timer_raw + 1 && i < MAX_NB_TIMER; i++)
	{
		s_timer_entry *row = (timers+i);

		if (callback && 	// if something to store
		   row->state == TIMER_FREE) // and empty row
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
		TIMEVAL real_timer_value = min(value, TIMEVAL_MAX);
		TIMEVAL elapsed_time = getElapsedTime();

		//printf("elapsed_time=%d real_timer_value=%d total_sleep_time=%d\n", elapsed_time, real_timer_value, total_sleep_time);
		if (total_sleep_time > elapsed_time && total_sleep_time - elapsed_time > real_timer_value)
		{
			total_sleep_time = elapsed_time + real_timer_value;
			setTimer(real_timer_value);
		}
		//printf("SetAlarm() return %d\n", row_number);
		return row_number;
	}
	return TIMER_NONE;
}

// ---------  Use this to remove an alarm ---------
TIMER_HANDLE DelAlarm(TIMER_HANDLE handle)
{
	// Quick and dirty. system timer will continue to be trigged, but no action will be preformed.
	MSG_WAR(0x3320, "DelAlarm. handle = ", handle);
	if(handle != TIMER_NONE)
	{
		if(handle == last_timer_raw) 
			last_timer_raw--;
		timers[handle].state = TIMER_FREE; 		
	}
	else {
	}
	return TIMER_NONE;
}


// ---------  TimeDispatch is called on each timer expiration ---------
void TimeDispatch()
{
	TIMER_HANDLE i;
	TIMEVAL next_wakeup = TIMEVAL_MAX; // used to compute when should normaly occur next wakeup
	// First run : change timer state depending on time
	// Get time since timer signal
	TIMEVAL overrun = getElapsedTime();
	
	TIMEVAL real_total_sleep_time = total_sleep_time + overrun;
	//printf("total_sleep_time %d + overrun %d\n", total_sleep_time , overrun);

	for(i=0; i <= last_timer_raw; i++)
	{
		s_timer_entry *row = (timers+i);

		if (row->state & TIMER_ARMED) // if row is active
		{
			if (row->val <= real_total_sleep_time) // to be trigged
			{
				//printf("row->val(%d) <= (%d)real_total_sleep_time\n", row->val, real_total_sleep_time);
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

