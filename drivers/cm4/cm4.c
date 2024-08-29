// Includes for the Canfestival driver
#include "canfestival.h"
#include "timer.h"
#include "data.h"
#include "cm4.h"

TIMEVAL last_counter_val = 0;
TIMEVAL elapsed_time = 0;

static CO_Data *co_data = NULL;

// Initializes the timer, turn on the interrupt and put the interrupt time to zero
void initTimer(void)
{

}

//Set the timer for the next alarm.
void setTimer(TIMEVAL value)
{
    uint32_t timer = __HAL_TIM_GET_COUNTER(&htim3); // Copy the value of the running timer
    elapsed_time += timer - last_counter_val;
    last_counter_val = TIMEVAL_MAX - value;
    __HAL_TIM_SET_COUNTER(&htim3, TIMEVAL_MAX-value);

    __HAL_TIM_ENABLE(&htim3);
}

//Return the elapsed time to tell the Stack how much time is spent since last call.
TIMEVAL getElapsedTime(void)
{
    uint32_t timer = __HAL_TIM_GET_COUNTER(&htim3);
    if(timer < last_counter_val)
    {
        timer += TIMEVAL_MAX;
    }
        
    TIMEVAL elapsed = timer - last_counter_val + elapsed_time;
    
    return elapsed;
}


/* prescaler values for 87.5%  sampling point (88.9% at 1Mbps)
   if unknown bitrate default to 50k
*/
uint16_t brp_from_birate(uint32_t bitrate)
{
    if(bitrate == 10000)
        return 225;
    if(bitrate == 50000)
        return 45;
    if(bitrate == 125000)
        return 18;
    if(bitrate == 250000)
        return 9;
    if(bitrate == 500000)
        return 9;
    if(bitrate == 1000000)
        return 4;
    return 45;
}

//Initialize the CAN hardware 
unsigned char canInit(CO_Data * d, uint32_t bitrate)
{
    // /* save the canfestival handle */  
    co_data = d;

    return 1;
}

// The driver send a CAN message passed from the CANopen stack
unsigned char canSend(CAN_PORT notused, Message *m)
{
    uint32_t TxMailbox;
    CAN_TxHeaderTypeDef TxMessage;
    uint8_t data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    TxMessage.IDE = CAN_ID_STD;     //设置ID类型
    TxMessage.StdId = m->cob_id;;   //设置ID�???
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

    if (HAL_CAN_AddTxMessage(&hcan1, &TxMessage, data, &TxMailbox) != HAL_OK)
    {
        // __disable_irq();
        // while (1)
        // {
        // }
        return 0;
    }

    return 1;
}

//The driver pass a received CAN message to the stack
/*
unsigned char canReceive(Message *m)
{
}
*/
unsigned char canChangeBaudRate_driver( CAN_HANDLE fd, char* baud)
{
    return 0;
}


// void disable_it(void)
// {
    // TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);
    // CAN_ITConfig(CANx, CAN_IT_FMP0, DISABLE);
// }

// void enable_it(void)
// {
    // TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    // CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
// }


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    uint8_t  data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    HAL_StatusTypeDef  status;
    
    if (hcan == &hcan1)
    {
        Message rxm = {0};
        CAN_RxHeaderTypeDef RxMessage;
        
        status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &RxMessage, data);
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
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim3)
    {
        last_counter_val = 0;
        elapsed_time = 0;

        __HAL_TIM_CLEAR_FLAG(&htim3, TIM_SR_UIF);

        TimeDispatch();
    }
}
