/*
 * trace.h
 *
 *  Created on: 14 mars 2022
 *      Author: nlacaill
 */

// TODO: remove?

#ifndef TRACE_H_
#define TRACE_H_

#include "FreeRTOS.h"
#include "projdefs.h"
#include "portable.h"

/*
 * for profiling need a timer counter
 *
	void TIM5_Init(uint32_t freq);
	uint32_t TIM5_get_cnt(void);
 */


/* to add tag field to FreeRTOS TCB */
#define configUSE_APPLICATION_TASK_TAG			1

/* note : keep task tag 0 for Idle task*/

//#define USE_FUNCTION_FOR_TRACE_TAG
#ifndef USE_FUNCTION_FOR_TRACE_TAG

/* a simple tag added to the TCB, the field will contain only a number
 * that can used by a trace function.
 * Exemple :
 */
/* call a function with task tag in parameters*/
#define traceTASK_SWITCHED_OUT() switched_out((int)pxCurrentTCB->pxTaskTag)
#define traceTASK_SWITCHED_IN() switched_in((int)pxCurrentTCB->pxTaskTag)
#define traceMOVED_TASK_TO_READY_STATE(xTask) task_to_ready(xTask)	// task handle

/* user trace function prototype */
void task_to_ready(void * pvParameter);
void switched_in(int task_tag);
void switched_out(int task_tag);


#else
/* it is possible to call only one task when this one is switched in
 * by registering a callback function via
 * vTaskSetApplicationTaskTag( NULL, taskcallback );
 * the first parameters if NULL is the current task, otherwise give
 * the task handle, the second parameters is the callback function
 * that must be of TaskHookFunction_t type, the function address is
 * stored in the tag field of TCB
 * ex : BaseType_t taskcallback( void * pvParameter )
 * Now the task callback function can be called when by trace function
 * it must call xTaskCallApplicationTaskHook (with pxCurrentTCB = taskhandle
 * as first parameters and a user parameters that will be passed to
 * the callbak function
 * #define traceTASK_SWITCHED_IN() xTaskCallApplicationTaskHook( NULL, 0 )
 * with first parameters NULL is equiv to pxCurrentTCB otherwise a taskhandle
 * must be provided. the second parameters is the one to the callback function
 */
#define traceTASK_SWITCHED_IN() xTaskCallApplicationTaskHook( NULL, pxCurrentTCB->pxTaskTag )

/* callback function for trace */
BaseType_t T1callback( void * pvParameter );
BaseType_t T2callback( void * pvParameter );


#endif //USE_FUNCTION_FOR_TRACE_TAG


#endif /* TRACE_H_ */
