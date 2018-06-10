/*
 * timer.h
 *
 *  Created on: 8 Jun 2018
 *      Author: Dmitri
 */

#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_


/*======================= I N C L U D E S =========================*/

#include <msp430.h>
#include <stdint.h>


/*======================= D E F I N E S ===========================*/

/* Number of supported timers in Timer Module */
#define MAX_TIMERS                  (4U)

/* Initial value for Timer Module timer counter */
#define TM_CNT_INIT                 (1U)

/* Return code on success */
#define RC_SUCCESS                  (0U)

/* Return code on failure */
#define RC_FAILURE                  (-1)

/* Null pointer */
#define NULL                        ((void*)0)

/* MSP430 Master clock frequency */
#define FCPU                        (16000000UL)

/* MSP430 Timer A0 prescaler */
#define TIMER_A0_PRESCALER          (8U)

/* MSP430 Timer A0 input clock frequency */
#define TIMER_A0_FREQ               ((uint32_t) (FCPU / TIMER_A0_PRESCALER))

/* MSP430 Timer A0 compare period [ms] */
#define TIMER_A0_CCR0_PERIOD_MS     (1U) // 1 [ms]

/* MSP430 Timer A0 compare counter value */
#define TIMER_A0_SET_CCR0_VALUE     ((uint32_t) (TIMER_A0_CCR0_PERIOD_MS * TIMER_A0_FREQ / 1000UL))


/*======================= D A T A T Y P E S =======================*/

/* User defined timer callback function */
typedef void (*timer_cb_f)(void*);

/* Available Timer Module modes */
typedef enum
{
    FREE = 0,
    SINGLE,
    CYCLIC,
    SINGLE_CYCLIC
}TimerMode;


/* Timer resolution */
typedef enum
{
    MILLISECOND=0,
    SECOND
}TimerResolution;


/* Timer Module structure */
typedef struct
{
    /* Timer mode */
    TimerMode mode;

    /* Timer resolution in [s] or [ms]*/
    TimerResolution resoluton;

    /* Incremented on every HW timer interrupt */
    uint32_t counter;

    /* User defined timeout for one-shot timer mode */
    uint32_t single_timeout;

    /* Calculated number of HW timer ticks for user defined single timer */
    uint32_t single_ticks_compare;

    /* User defined timeout for cyclic timer mode */
    uint32_t cyclic_timeout;

    /* Calculated number of HW timer ticks for user defined timer */
    uint32_t cyclic_ticks_compare;

    /* User defined callback function, called within HW timer ISR */
    timer_cb_f handler;

    /* User defined argument for callback function */
    void* handler_arg;
}TimerModule_t;


/*======================= D A T A  ================================*/



/*======================= P U B L I C  P R O T O T Y P E S ========*/

void HwTimerStart(void);
void HwTimerStop(void);
int8_t AddNewTimer(uint32_t single_timeout,
                   uint32_t cyclic_timeout,
                   TimerMode mode,
                   TimerResolution resolution,
                   timer_cb_f user_cb_f,
                   void *userdata
                   );
uint32_t GetMilliSeconds(void);


#endif /* INCLUDE_TIMER_H_ */
