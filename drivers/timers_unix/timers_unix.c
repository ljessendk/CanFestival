#include <stdlib.h>

#include <sys/time.h>
#include <signal.h>
#include <pthread.h> 

#include "applicfg.h"
#include "can_driver.h"
#include "timer.h"

pthread_mutex_t CanFestival_mutex = PTHREAD_MUTEX_INITIALIZER;

TIMEVAL last_time_set = TIMEVAL_MAX;

struct timeval last_sig;

char stop_timer=0;

void sig(int val)
{
	signal( SIGALRM, sig);
	gettimeofday(&last_sig,NULL);
//	printf("getCurrentTime() return=%u\n", p.tv_usec);
}

void initTimer(void)
{
	gettimeofday(&last_sig,NULL);
	signal( SIGALRM, sig);
	stop_timer = 0;
}

void stopTimer(void)
{
	stop_timer = 1;
	kill(0, SIGALRM);	
}

void EnterMutex(void)
{
	pthread_mutex_lock(&CanFestival_mutex); 
}

void LeaveMutex(void)
{
	pthread_mutex_unlock(&CanFestival_mutex);
}

void TimerLoop(TimerCallback_t init_callback)
{
	initTimer();
	// At first, TimeDispatch will call init_callback.
	SetAlarm(NULL, 0, init_callback, 0, 0);
	while (!stop_timer) {
		EnterMutex();
		TimeDispatch();
		LeaveMutex();
		pause();
	}
}

void ReceiveLoop(void* arg)
{
	canReceiveLoop((CAN_HANDLE)arg);
}

void CreateReceiveTask(CAN_HANDLE fd0, TASK_HANDLE* Thread)
{
	pthread_create(Thread, NULL, (void *)&ReceiveLoop, (void*)fd0);
}

void WaitReceiveTaskEnd(TASK_HANDLE Thread)
{
	pthread_join(Thread, NULL);
}

#define max(a,b) a>b?a:b
void setTimer(TIMEVAL value)
{
//	printf("setTimer(TIMEVAL value=%d)\n", value);
	struct itimerval timerValues;
	struct itimerval timerV = {{0,0},{0,0}};
	timerValues.it_value.tv_sec = 0;
	timerValues.it_value.tv_usec = max(value,1);
	timerValues.it_interval.tv_sec = 0;
	timerValues.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &timerValues, &timerV);
}

TIMEVAL getElapsedTime(void)
{
	struct timeval p;
	gettimeofday(&p,NULL);
//	printf("getCurrentTime() return=%u\n", p.tv_usec);
	return (p.tv_sec - last_sig.tv_sec)* 1000000 + p.tv_usec - last_sig.tv_usec;
}
