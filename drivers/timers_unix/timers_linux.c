#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <poll.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>

#include "applicfg.h"
#include "timer.h"

static pthread_mutex_t CanFestival_mutex = PTHREAD_MUTEX_INITIALIZER;

static TIMEVAL last_sig;

static int iTimerFD = -1;
static int iWakeFD = -1;

static pthread_t iTimeThrId;
static int timerThreadRunning = 0;


static TIMEVAL monotonic_time_us(void)
{
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) == -1)
    {
        perror("clock_gettime()");
        return 0;
    }

    return ((TIMEVAL)now.tv_sec * 1000000ULL) + ((TIMEVAL)now.tv_nsec / 1000ULL);
}


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
    (void)arg;

    while (timerThreadRunning)
    {
        struct pollfd pollfds[2];
        uint64_t exp = 0;
        int pollRet;

        pollfds[0].fd = iTimerFD;
        pollfds[0].events = POLLIN;
        pollfds[0].revents = 0;
        pollfds[1].fd = iWakeFD;
        pollfds[1].events = POLLIN;
        pollfds[1].revents = 0;

        pollRet = poll(pollfds, 2, -1);
        if (pollRet == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            if (timerThreadRunning)
            {
                perror("poll()");
            }

            break;
        }

        if (pollfds[1].revents & POLLIN)
        {
            uint64_t wakeValue;

            if (read(iWakeFD, &wakeValue, sizeof(wakeValue)) == -1 && errno != EAGAIN)
            {
                perror("read()");
            }

            break;
        }

        if (!(pollfds[0].revents & POLLIN))
        {
            continue;
        }

        ssize_t ret = read(iTimerFD, &exp, sizeof(exp));
        
        if (ret == (ssize_t)sizeof(exp))
        {
            if (!timerThreadRunning)
            {
                break;
            }

            EnterMutex();
            last_sig = monotonic_time_us();
            TimeDispatch();
            LeaveMutex();

            continue;
        }

        if (ret == -1 && errno == EINTR)
        {
            continue;
        }

        if (timerThreadRunning)
        {
            perror("read()");
        }

        break;
    }

    return NULL;
}


void TimerInit(void)
{
    // Take first absolute time ref.
    last_sig = monotonic_time_us();

    iWakeFD = eventfd(0, EFD_CLOEXEC);
    if (iWakeFD == -1)
    {
        perror("eventfd()");
        return;
    }

    iTimerFD = timerfd_create(CLOCK_MONOTONIC, 0);
    if (iTimerFD == -1)
    {
        perror("timer_create()");
        close(iWakeFD);
        iWakeFD = -1;
        return;
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

    timerThreadRunning = 1;
    if(pthread_create(&iTimeThrId, NULL, timer_notify_thr, NULL))
    {
        perror("pthread_create()");
        timerThreadRunning = 0;
        close(iTimerFD);
        iTimerFD = -1;
        close(iWakeFD);
        iWakeFD = -1;
    }
}

void StopTimerLoop(TimerCallback_t exitfunction)
{
    if (iTimerFD != -1 && iWakeFD != -1)
    {
        uint64_t wakeValue = 1;

        timerThreadRunning = 0;

        if (write(iWakeFD, &wakeValue, sizeof(wakeValue)) == -1)
        {
            perror("write()");
        }

        if (pthread_join(iTimeThrId, NULL))
        {
            perror("pthread_join()");
        }

        close(iTimerFD);
        iTimerFD = -1;
        close(iWakeFD);
        iWakeFD = -1;
    }

    EnterMutex();
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
    if (value == TIMEVAL_MAX || iTimerFD == -1)
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
    TIMEVAL now = monotonic_time_us();

    if (now < last_sig)
    {
        return 0;
    }

    return (now - last_sig);
}
