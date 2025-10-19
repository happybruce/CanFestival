#include <stdlib.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#include "applicfg.h"
#include "timer.h"

static pthread_mutex_t CanFestival_mutex = PTHREAD_MUTEX_INITIALIZER;

static struct timeval last_sig;

static int iTimerFD = -1;

static pthread_t iTimeThrId;


void TimerCleanup(void)
{
    /* only used in realtime apps */
}

void EnterMutex(void)
{
    if(pthread_mutex_lock(&CanFestival_mutex))
    {
        fprintf(stderr, "pthread_mutex_lock() failed\n");
    }
}

void LeaveMutex(void)
{
    if(pthread_mutex_unlock(&CanFestival_mutex))
    {
        fprintf(stderr, "pthread_mutex_unlock() failed\n");
    }
}


void* timer_notify_thr(void* arg)
{
    while (1)
    {
        uint64_t exp = 0;
        
        int ret = read(iTimerFD, &exp, sizeof(uint64_t));
        
        if (ret == sizeof(uint64_t))
        {
            if(gettimeofday(&last_sig, NULL))
            {
                perror("gettimeofday()");
            }

            EnterMutex();
            TimeDispatch();
            LeaveMutex();
        }
    }
}


void TimerInit(void)
{
    // Take first absolute time ref.
    if(gettimeofday(&last_sig, NULL))
    {
        perror("gettimeofday()");
    }


    iTimerFD = timerfd_create(CLOCK_MONOTONIC, 0);
    if (iTimerFD == -1)
    {
        perror("timer_create()");
    }

    struct itimerspec itime;
    itime.it_value.tv_sec     = 0;
    itime.it_value.tv_nsec    = 0;
    itime.it_interval.tv_sec  = 0;
    itime.it_interval.tv_nsec = 0;
    // stop timer at first
    if (timerfd_settime(iTimerFD, 0, &itime, NULL) == -1)
    {
        perror("timerfd_settime()");
    }

    if(pthread_create(&iTimeThrId, NULL, timer_notify_thr, NULL))
    {
        perror("pthread_create()");
    }
}

void StopTimerLoop(TimerCallback_t exitfunction)
{
    EnterMutex();

    struct itimerspec itime;
    itime.it_value.tv_sec     = 0;
    itime.it_value.tv_nsec    = 0;
    itime.it_interval.tv_sec  = 0;
    itime.it_interval.tv_nsec = 0;
    /* stop timer */
    if (timerfd_settime(iTimerFD, 0, &itime, NULL) == -1)
    {
        perror("timerfd_settime()");
    }
    close(iTimerFD);
    
    iTimerFD = -1;

    exitfunction(NULL,0);
    LeaveMutex();
}

void StartTimerLoop(TimerCallback_t init_callback)
{
    EnterMutex();
    // At first, TimeDispatch will call init_callback.
    SetAlarm(NULL, 0, init_callback, 0, 0);
    LeaveMutex();
}

void canReceiveLoop_signal(int sig)
{
}
/* We assume that ReceiveLoop_task_proc is always the same */
static void (*unixtimer_ReceiveLoop_task_proc)(CAN_PORT) = NULL;

/**
 * Enter in realtime and start the CAN receiver loop
 * @param port
 */
void* unixtimer_canReceiveLoop(void* port)
{
    /*get signal*/
      //  if(signal(SIGTERM, canReceiveLoop_signal) == SIG_ERR) {
    //        perror("signal()");
    //}
    unixtimer_ReceiveLoop_task_proc((CAN_PORT)port);

    return NULL;
}

void CreateReceiveTask(CAN_PORT port, TASK_HANDLE* Thread, void* ReceiveLoopPtr)
{
    unixtimer_ReceiveLoop_task_proc = ReceiveLoopPtr;
    if(pthread_create(Thread, NULL, unixtimer_canReceiveLoop, (void*)port))
    {
        perror("pthread_create()");
    }
}

void WaitReceiveTaskEnd(TASK_HANDLE *Thread)
{
    if(pthread_cancel(*Thread))
    {
        perror("pthread_cancel()");
    }

    if(pthread_join(*Thread, NULL))
    {
        perror("pthread_join()");
    }
}

#define maxval(a,b) ((a>b)?a:b)
void setTimer(TIMEVAL value)
{
    if (value == TIMEVAL_MAX)
    {
        return;
    }
    
    // TIMEVAL is us whereas setitimer wants ns...
    long tv_nsec = 1000 * (maxval(value,1)%1000000);
    time_t tv_sec = value/1000000;

    // printf("setTimer(TIMEVAL value=%lx, %ld, %ld)\n", value, tv_nsec, tv_sec);
    
    struct itimerspec timerValues;
    timerValues.it_value.tv_sec = tv_sec;
    timerValues.it_value.tv_nsec = tv_nsec;
    timerValues.it_interval.tv_sec = 0;
    timerValues.it_interval.tv_nsec = 0;

    if (timerfd_settime(iTimerFD, 0, &timerValues, NULL) == -1)
    {                
        perror("timerfd_settime");
    }
}

TIMEVAL getElapsedTime(void)
{
    struct timeval p;
    if(gettimeofday(&p,NULL)) 
    {
        perror("gettimeofday()");
    }
//    printf("getCurrentTime() return=%u\n", p.tv_usec);
    return (p.tv_sec - last_sig.tv_sec)* 1000000 + p.tv_usec - last_sig.tv_usec;
}
