/*
 * main.h
 *
 *  Created on: 8 Jun 2018
 *      Author: Dmitri Ivanov
 */

#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_

/*======================= I N C L U D E S =========================*/

#include <msp430.h>

/*======================= D E F I N E S ===========================*/

/*======================= D A T A T Y P E S =======================*/

/*======================= D A T A  ================================*/

/*======================= P U B L I C  P R O T O T Y P E S ========*/

void ConfCpuClk(void);
void ConfPort(void);
void TestTimerModule(void);

void cb_fun1(void *arg);
void cb_fun2(void *arg);
void cb_fun3(void *arg);


#endif /* INCLUDE_MAIN_H_ */
