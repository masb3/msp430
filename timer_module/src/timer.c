/*
 * timer.c
 *
 *  Created on: 8 Jun 2018
 *      Author: Dmitri Ivanov
 */


#include "include/timer.h"

/** Start local data **/

/* Timer Module */
volatile static TimerModule_t TimerModule[MAX_TIMERS];

/* Counter incremented on every HW timer ISR. Keeps time since system start-up. */
volatile static uint32_t sys_timer_tick_cnt;

/** End local data **/


/** Start private functions **/

/*!****************************************************************************
*
*  \b               ConfigureTimer_A0
*
*  Configures and starts HW Timer_A0.
*
*
*  \param           [in]        -
*
*
*  \pre             None
*
*  \post            HW Timer_A0 started and Capture/compare interrupt enabled.
*
*  \return          -
*
******************************************************************************/
static void ConfigureTimer_A0()
{
    TA0CCR0 = TIMER_A0_SET_CCR0_VALUE;      // HW timer compare value
    TA0CCTL0 = CCIE;                        // CCR0 interrupt enabled
    TA0CTL = TASSEL_2 + MC_1 + ID_3;        // SMCLK, Up to CCR0, Timer A input divider /8
    __enable_interrupt();
}


/*!****************************************************************************
*
*  \b               StopTimer_A0
*
*  Stops HW Timer_A0.
*
*
*  \param           [in]        -
*
*
*  \pre             None
*
*  \post            HW Timer_A0 is halted.
*
*  \return          -
*
******************************************************************************/
static void StopTimer_A0(void)
{
    TA0CCTL0 &= ~CCIE; // CCR0 interrupt disabled
    TA0CTL = MC_0; // Timer A mode control: 0 - Stop
}


/*!****************************************************************************
*
*  \b               ResetTimerModule
*
*  Resets all timers in Timer Module to FREE mode.
*
*
*  \param           [in]        -
*
*
*  \pre             None
*
*  \post            None
*
*  \return          -
*
******************************************************************************/
static void ResetTimerModule(void)
{
    uint8_t i;
    for(i = 0; i < MAX_TIMERS; i++)
        TimerModule[i].mode = FREE;
}


/*!****************************************************************************
*
*  \b               TestDefinedTimeouts
*
*  Tests if user defined timeouts are correct.
*
*
*  \param           [in]        single_timeout      User defined single-shot
*                                                   timer timeout
*
*  \param           [in]        cyclic_timeout      User defined cyclic
*                                                   timer timeout
*
*  \param           [in]        mode                User defined timer mode
*
*
*  \pre             None
*
*  \post            None
*
*  \return          Returns zero on success or a non-zero value on failure.
*
******************************************************************************/
static int8_t TestDefinedTimeouts(uint32_t single_timeout, uint32_t cyclic_timeout, TimerMode mode)
{
    int8_t ret;

    switch(mode)
    {
        case FREE:
            ret = RC_FAILURE; // User is not allowed to define Timer with mode FREE
            break;

        case SINGLE:
            ret = (single_timeout > 0) ? RC_SUCCESS : RC_FAILURE;
            break;

        case CYCLIC:
            ret = (cyclic_timeout > 0) ? RC_SUCCESS : RC_FAILURE;
            break;

        case SINGLE_CYCLIC:
            ret = ((single_timeout > 0) && (cyclic_timeout > 0)) ? RC_SUCCESS : RC_FAILURE;
            break;

        default:
            ret = RC_FAILURE; // Unknown Timer mode
            break;
    }

    return ret;
}

/** End private functions **/

/** Start Timer public interface **/

/*!****************************************************************************
*
*  \b               HwTimerStart
*
*   Starts HW Timer which is used by Timer Module and counts time since system start-up.
*   Must be called right after system starts.
*
*
*  \param           [in]        -
*
*
*  \pre             None
*
*  \post            None
*
*  \return          -
*
******************************************************************************/
void HwTimerStart(void)
{
    sys_timer_tick_cnt = 0;
    ResetTimerModule(); // Set all timers to FREE mode
    ConfigureTimer_A0(); // Configure platform specific HW timer
    __enable_interrupt();
}


/*!****************************************************************************
*
*  \b               HwTimerStop
*
*  Stops the HW timer from counting down.
*
*
*  \param           [in]        -
*
*
*  \pre             None
*
*  \post            None
*
*  \return          -
*
******************************************************************************/
void HwTimerStop(void)
{
    StopTimer_A0(); // Stop platform specific HW timer
    ResetTimerModule();
    sys_timer_tick_cnt = 0;
}


/*!****************************************************************************
*
*  \b               AddNewTimer
*
*  Adds new timer to Timer Module. User defines timer mode: single, cyclic or
*  single-cyclic.
*  Timeout value is defined with the resolution of 1 millisecond or 1 second.
*
*
*  \param           [in]    single_ms       One-shot timeout
*
*  \param           [in]    cyclic_ms       Cyclic timeout
*
*  \param           [in]    mode            Timer mode
*
*  \param           [in]    resolution      Timeout resolution
*
*  \param           [in]    handler         Timer callback function
*
*  \param           [in]    userdata        User defined data to be passed to
*                                           timer callback function.
*
*  \pre             HW Timer is started.
*
*  \post            User timer started.
*
*  \return          Returns timer Id in range [0...MAX_TIMERS-1] on success
*                   or less then zero value on failure.
*
******************************************************************************/
int8_t AddNewTimer(uint32_t single_timeout,
                   uint32_t cyclic_timeout,
                   TimerMode mode,
                   TimerResolution resolution,
                   timer_cb_f handler,
                   void* userdata)
{
    if(RC_SUCCESS == TestDefinedTimeouts(single_timeout, cyclic_timeout, mode)) // Test if user defined timeouts correspond to defined mode
    {
        uint8_t i;
        for(i = 0; i < MAX_TIMERS; i++) // Search for available timer in Timer Module
        {
            if(TimerModule[i].mode == FREE) // Found free timer in Timer Module
            {
                TimerModule[i].resoluton = resolution;
                TimerModule[i].counter = TM_CNT_INIT;
                TimerModule[i].single_timeout = single_timeout;
                TimerModule[i].single_ticks_compare =
                        (SECOND == resolution) ? ((single_timeout * 1000) / TIMER_A0_CCR0_PERIOD_MS) : (single_timeout / TIMER_A0_CCR0_PERIOD_MS);
                TimerModule[i].cyclic_timeout = cyclic_timeout;
                TimerModule[i].cyclic_ticks_compare =
                        (SECOND == resolution) ? ((cyclic_timeout * 1000) / TIMER_A0_CCR0_PERIOD_MS) : (cyclic_timeout / TIMER_A0_CCR0_PERIOD_MS);
                TimerModule[i].handler = handler;
                TimerModule[i].handler_arg = userdata;
                TimerModule[i].mode = mode; // Mode must be set last
                return i; // Return Id of the new timer in Timer Module
            }
        }
    }

    return RC_FAILURE; // No FREE timer in Timer Module or defined timeouts are not correct
}


/*!****************************************************************************
*
*  \b               GetMilliSeconds
*
*  Returns the time that has passed from system startup in milliseconds.
*
*
*  \param           [in]        -
*
*
*  \pre             HW Timer is started.
*
*  \post            None
*
*  \return          Returns system time in milliseconds.
*
******************************************************************************/
uint32_t GetMilliSeconds(void)
{
    return (sys_timer_tick_cnt * TIMER_A0_CCR0_PERIOD_MS);
}

/** End Timer public interface **/

/** Start Timer ISR **/

/*!****************************************************************************
*
*  \b               HwTimerIsr
*
*  Interrupt service routine for the HW Timer A0 of the MSP430.
*
*
*  \param           [in]        -
*
*
*  \pre             None
*
*  \post            None
*
*  \return          -
*
******************************************************************************/
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void HwTimerIsr (void)
{
    ++sys_timer_tick_cnt;

    uint8_t i;
    for(i = 0; i < MAX_TIMERS; i++)
    {
        if(TimerModule[i].mode > FREE) // Found enabled timer
        {
            if(TimerModule[i].mode == SINGLE)
            {
                if(TimerModule[i].counter >= TimerModule[i].single_ticks_compare)
                {
                    TimerModule[i].mode = FREE;
                    TimerModule[i].handler(TimerModule[i].handler_arg);
                }
                else
                {
                    TimerModule[i].counter++;
                }
            }
            else if(TimerModule[i].mode == CYCLIC)
            {
                if(TimerModule[i].counter >= TimerModule[i].cyclic_ticks_compare)
                {
                    TimerModule[i].counter = TM_CNT_INIT;
                    TimerModule[i].handler(TimerModule[i].handler_arg);
                }
                else
                {
                    TimerModule[i].counter++;
                }
            }
            else if(TimerModule[i].mode == SINGLE_CYCLIC)
            {
                if(TimerModule[i].counter >= TimerModule[i].single_ticks_compare)
                {
                    TimerModule[i].mode = CYCLIC;
                    TimerModule[i].counter = TM_CNT_INIT;
                    TimerModule[i].handler(TimerModule[i].handler_arg);
                }
                else
                {
                    TimerModule[i].counter++;
                }
            }
        }
    }
}

/** End Timer ISR **/

