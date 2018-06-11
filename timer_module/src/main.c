/*
 * main.h
 *
 *  Created on: 8 Jun 2018
 *      Author: Dmitri Ivanov
 */


#include <stdlib.h>
#include "include/main.h"
#include "include/timer.h"


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  ConfCpuClk(); // Configure CPU clock source
  HwTimerStart(); // Start timer which is used by Timer Module; System time is counted from this moment
  ConfPort(); // Configure MSP430 IOs for test purposes

  /* Test timer module */
  uint8_t cb_f1_arg = BIT0;
  if(RC_FAILURE == AddNewTimer(1000, 5, SINGLE_CYCLIC, MILLISECOND, cb_fun1, &cb_f1_arg))
      P1OUT |= BIT6;


  uint8_t cb_f2_arg = BIT5;
  if(RC_FAILURE == AddNewTimer(0, 333, CYCLIC, MILLISECOND, cb_fun2, &cb_f2_arg))
      P1OUT |= BIT6;


  if(RC_FAILURE == AddNewTimer(60, 0, SINGLE, SECOND, cb_fun3, NULL))
      P1OUT |= BIT6;

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
}


void ConfCpuClk(void)
{
    DCOCTL = 0;                             // Select lowest DCOx and MODx settings

#if FCPU == 1000000
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
#elif FCPU == 8000000
  BCSCTL1 = CALBC1_8MHZ;
  DCOCTL = CALDCO_8MHZ;
#elif FCPU == 16000000
  BCSCTL1 = CALBC1_16MHZ;
  DCOCTL = CALDCO_16MHZ;
#endif

}

void ConfPort(void)
{
    P1DIR |= BIT0 | BIT5 | BIT6;    // P1.0 output and P1.5 output and P1.6 output (GREEN LED)
    P1OUT |= BIT0 | BIT5;           // P1.0 output and P1.5 output set High state
    P1OUT &= ~BIT6;                 // Turn Off Green Led
}

void cb_fun1(void *arg)
{
    uint8_t val = *(uint8_t*)arg;
    P1OUT ^= val;                            // Toggle P1.0
}

void cb_fun2(void *arg)
{
    uint8_t val = *(uint8_t*)arg;
    P1OUT ^= val;                            // Toggle P1.5
}

void cb_fun3(void *arg)
{
    if(GetMilliSeconds() >= 60000)
        HwTimerStop();                          // Stop HW Timer
}
