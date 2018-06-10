/*
 * main.h
 *
 *  Created on: 8 Jun 2018
 *      Author: Dmitri
 */

#ifndef INCLUDE_MAIN_H_
#define INCLUDE_MAIN_H_


#include <msp430.h>

void ConfCpuClk(void);
void ConfPort(void);
void TestTimerModule(void);

void cb_fun1(void *arg);
void cb_fun2(void *arg);
void cb_fun3(void *arg);


#endif /* INCLUDE_MAIN_H_ */
