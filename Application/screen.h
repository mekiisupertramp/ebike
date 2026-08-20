/*
 * screen.h
 *
 *  Created on: 18 oct. 2024
 *      Author: Mehmed Blazevic
 */

#ifndef APPLICATION_SCREEN_H_
#define APPLICATION_SCREEN_H_

#include "stdbool.h"

void screenThread_createTask(void);
// function for display
void startupScreen();
void computeDisplaySpeed();
void computeDisplayBattery(bool poweringUp);
void computeDisplayTrips(bool force);
void computeDisplayDate();



#endif /* APPLICATION_SCREEN_H_ */
