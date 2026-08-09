// Includes for the Canfestival driver
#include "canfestival.h"
#include "timerscfg.h"
#include "timer.h"
#include "data.h"


static TIMEVAL last_counter_val = 0;
static TIMEVAL elapsed_time = 0;

static CO_Data *co_data = NULL;

static CAN_HandleTypeDef *CanPtr   = NULL;
static TIM_HandleTypeDef *TimerPtr = NULL;


void selectTimer(TIM_HandleTypeDef *timer)
{
    TimerPtr = timer;
}

void selectCAN(CAN_HandleTypeDef *can)
{
    CanPtr = can;
}

//Set the timer for the next alarm.
void setTimer(TIMEVAL value)
{   
    if ((!TimerPtr) || value >= TIMEVAL_MAX)
    {
        return;
    }
    
    uint32_t timer = __HAL_TIM_GET_COUNTER(TimerPtr); // Copy the value of the running timer
    elapsed_time += timer - last_counter_val;
    last_counter_val = TIMEVAL_MAX - value;
    __HAL_TIM_SET_COUNTER(TimerPtr, TIMEVAL_MAX-value);

    __HAL_TIM_ENABLE(TimerPtr);
}

//Return the elapsed time to tell the Stack how much time is spent since last call.
TIMEVAL getElapsedTime()
{
    if (!TimerPtr)
    {
        return 0;
    }

    uint32_t timer = __HAL_TIM_GET_COUNTER(TimerPtr);
    if(timer < last_counter_val)
    {
        timer += TIMEVAL_MAX;
    }
        
    TIMEVAL elapsed = timer - last_counter_val + elapsed_time;
    
    return elapsed;
}


//Initialize the CAN hardware 
unsigned char canInit(CO_Data *d, uint32_t bitrate)
{
    // Set CANopen data pointer
    co_data = d;

    return 1;
}

// The driver send a CAN message passed from the CANopen stack
unsigned char canSend(CAN_PORT notused, Message *m)
{
    if (!CanPtr)
    {
        return 0;
    }

    uint32_t TxMailbox;
    CAN_TxHeaderTypeDef TxMessage;
    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    TxMessage.IDE = CAN_ID_STD;     // set type of cobid
    TxMessage.StdId = m->cob_id;;   // copy cobid
    if(m->rtr)
    {
        TxMessage.RTR = CAN_RTR_REMOTE; // remote frame
    }
    else
    {
        TxMessage.RTR = CAN_RTR_DATA; // data frame
    }

    TxMessage.DLC = m->len;
    for(uint32_t i = 0 ; i < m->len ; i++)
    {
        data[i] = m->data[i];
    }

    if (HAL_CAN_AddTxMessage(CanPtr, &TxMessage, data, &TxMailbox) != HAL_OK)
    {
        // disable_it();
        return 0;
    }

    return 1;
}


// void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
void handleCANPendingMessage()
{
    uint8_t  data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    HAL_StatusTypeDef  status;

    if (!CanPtr)
    {
        return;
    }

    Message rxm = {0};
    CAN_RxHeaderTypeDef RxMessage;
    
    status = HAL_CAN_GetRxMessage(CanPtr, CAN_RX_FIFO0, &RxMessage, data);
    if (HAL_OK == status)
    {
        if(RxMessage.IDE == CAN_ID_EXT)
        {
            return;
        }
        
        rxm.cob_id = RxMessage.StdId;
        if(RxMessage.RTR == CAN_RTR_REMOTE)
        {
            rxm.rtr = 1;
        }
        
        rxm.len = RxMessage.DLC;
        for(uint32_t i = 0; i < rxm.len; i++)
        {
            rxm.data[i] = data[i];
        }
        
        canDispatch(co_data, &rxm);
    }
}


// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
void handleTimerPeriodElapsed()
{
    if (!TimerPtr)
    {
        return;
    }
    
    last_counter_val = 0;
    elapsed_time = 0;

    __HAL_TIM_CLEAR_FLAG(TimerPtr, TIM_SR_UIF);

    TimeDispatch();
}
