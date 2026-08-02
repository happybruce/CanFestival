#ifndef __CM4_CANFESTIVAL_H__
#define __CM4_CANFESTIVAL_H__


#include "can_driver.h"
#include "declaration.h"
#include "applicfg.h"
#include "stm32f4xx_hal.h"


/*********** These APIs are from drivers/cm4/cm4.c ***********/

/**
 * @brief Set the timer instance to be used by the driver
 * @param timer Pointer to the TIM_HandleTypeDef instance
 * @param irq The IRQ number for the timer interrupt
 */
void selectTimer(TIM_HandleTypeDef *timer);

/**
 * @brief Set the CAN instance to be used by the driver
 * @param can Pointer to the CAN_HandleTypeDef instance
 * @param irq The IRQ number for the CAN interrupt
 */
void selectCAN(CAN_HandleTypeDef *can);

/**
 * @brief Send a CAN message
 * @param notused Unused parameter
 * @param m Pointer to the CAN message to be sent
 * @return 0 on success, non-zero on failure
 */
unsigned char canSend(CAN_PORT notused, Message *m);

/**
 * @brief Initialize the CAN interface
 * @param d Pointer to the CAN object data structure
 * @param dummy Unused parameter
 * @return 0 on success, non-zero on failure
 */
unsigned char canInit(CO_Data *d, uint32_t dummy);

/**
 * @brief Handle a pending CAN message, will be called in HAL_CAN_RxFifo0MsgPendingCallback()
 */
void handleCANPendingMessage();

/**
 * @brief Handle a timer period elapsed event, will be called in HAL_TIM_PeriodElapsedCallback()
 */
void handleTimerPeriodElapsed();

#endif /* __CM4_CANFESTIVAL_H__ */
