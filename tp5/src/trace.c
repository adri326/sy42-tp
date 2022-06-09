/**
 ******************************************************************************
 * @file           : trace.c
 * @author         : NL
 * @brief          : functions for trace macro
 *
 ******************************************************************************
 */
/* header file that include stm32f401xe.h (in Drivers/CMSIS/Device/ST/Include/)
 * in the latest file are defined data structure and constant
 */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "timer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


// profiler counter
uint32_t T1_count;
uint32_t T2_count;
uint32_t T3_count;
uint32_t Idle_count;


BaseType_t T1callback(void* pvParameter) {
    T1_count++;    // led on
    return 0;
}
BaseType_t T2callback(void* pvParameter) {
    T2_count++;    // led on
    return 0;
}

void task_to_ready(void * pvParameter) {
    asm("nop");
}


/**
 * @brief   to know which task will run
 *             task tag is used here, last_TaskTag is set to
 *             the last task switched out in FreeRTOSConfig.h with trace macro
 * @parm       task tag
 */

void switched_in(int task_tag) {
//    if (last_TaskTag != task_tag){
        // context switch has occured : preemption or time-slice
        // do whatever ..
        // increase profiling counter
        switch (task_tag) {
            case 0 :
                Idle_count++;
                break;
            case 1 :
                T1_count++;
                break;
            case 2 :
                T2_count++;
                break;
            case 3 :
                T3_count++;
                break;
            default :

                break;
        }
//    }
}
/**
 * @brief   to know which task will be switched out
 *             task tag is used here, last_TaskTag is set to
 *             the last task switched out in FreeRTOSConfig.h with trace macro
 * @parm       task tag
 */
void switched_out(int task_tag) {
    switch (task_tag) {
        case 0 :
            break;
        case 1 :
            break;
        case 2 :
            break;
        case 3 :
            break;
        default :
            break;
    }
}
