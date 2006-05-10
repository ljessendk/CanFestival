/*
This file is part of CanFestival, a library implementing CanOpen Stack.

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


#if !defined(_LED_INDICATORS_H_)
#define _LED_INDICATORS_H_


enum 
{
	LED_NO_ERROR = 6, /* reference to States values */
	LED_AUTOBITRATE,
	LED_INVALID_CONFIG,
	LED_WARNING_LIMIT_REACH,
	LED_ERROR_CTRL_EVENT,
	LED_SYNC_ERROR,
	LED_EVENT_TIMER_ERROR,
	LED_BUS_OFF,
	LED_PRG_DOWNLOAD
};


void led_set_state(CO_Data *d, int state);


#endif
