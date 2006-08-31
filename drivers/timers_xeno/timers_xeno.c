#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/mutex.h>
#include <native/alarm.h>

#include "applicfg.h"
#include "can_driver.h"
#include "timer.h"

#define TIMERLOOP_TASK_CREATED        1

RT_MUTEX CanFestival_mutex;
RT_TASK timerloop_task;
RTIME last_time_read;
RTIME last_occured_alarm;
RTIME last_alarm_set;

char stop_timer=0;

void cleanup_all(void)
{
	rt_task_delete(&timerloop_task);
}
void StopTimerLoop(void)
{
	stop_timer = 1;
	rt_task_unblock(&timerloop_task);
}


void EnterMutex(void)
{
	rt_mutex_lock(&CanFestival_mutex, TM_INFINITE); 
}

void LeaveMutex(void)
{
	rt_mutex_unlock(&CanFestival_mutex);
}

void timerloop_task_proc(void *arg)
{
	int ret;
	do{
		do{
			last_occured_alarm = last_alarm_set;
			EnterMutex();
			TimeDispatch();
			LeaveMutex();
			while ((ret = rt_task_sleep_until(last_alarm_set)) == -EINTR);
		}while (ret == 0);
	}while (!stop_timer);
	printf("End of TimerLoop, code %d\n",ret);
}

void StartTimerLoop(TimerCallback_t init_callback)
{
	int ret;
	stop_timer = 0;
	char taskname[32];
	snprintf(taskname, sizeof(taskname), "timerloop-%d", getpid());

	mlockall(MCL_CURRENT | MCL_FUTURE);

	//create timerloop_task
	ret = rt_task_create(&timerloop_task, taskname, 0, 50, 0);
	if (ret) {
		printf("Failed to create timerloop_task, code %d\n",errno);
		return;
	}
 	
	getElapsedTime();
	last_alarm_set = last_time_read;
	last_occured_alarm = last_alarm_set;
	SetAlarm(NULL, 0, init_callback, 0, 0);
	// start timerloop_task
	ret = rt_task_start(&timerloop_task,&timerloop_task_proc,NULL);
	if (ret) {
		printf("Failed to start timerloop_task, code %d\n",errno);
		goto error;
	}
	
	return;
	
error:
	cleanup_all();
}

void ReceiveLoop_task_proc(void* arg)
{
	canReceiveLoop((CAN_HANDLE)arg);
}

void CreateReceiveTask(CAN_HANDLE fd0, TASK_HANDLE *ReceiveLoop_task)
{
	int ret;
	static int id = 0;
	char taskname[32];
	snprintf(taskname, sizeof(taskname), "canloop%d-%d", id, getpid());
	id++;

	mlockall(MCL_CURRENT | MCL_FUTURE);

	//create timerloop_task
	ret = rt_task_create(ReceiveLoop_task,taskname,0,50,0);
	if (ret) {
		printf("Failed to create ReceiveLoop_task number %d, code %d\n", id, errno);
		return;
	}
	// start timerloop_task
	ret = rt_task_start(ReceiveLoop_task,&ReceiveLoop_task_proc,(void*)fd0);
	if (ret) {
		printf("Failed to start ReceiveLoop_task number %d, code %d\n", id, errno);
		return;
	}
}

void WaitReceiveTaskEnd(TASK_HANDLE *Thread)
{
	rt_task_delete(Thread);
}

void setTimer(TIMEVAL value)
{
	last_alarm_set = (value == TIMEVAL_MAX) ? TIMEVAL_MAX : last_time_read + value;
	rt_task_unblock(&timerloop_task);
}

TIMEVAL getElapsedTime(void)
{
	last_time_read = rt_timer_ticks2ns(rt_timer_read());
	return last_time_read - last_occured_alarm;
}
