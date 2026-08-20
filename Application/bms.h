/*
 * bms.h
 *
 *  Created on: 28 nov. 2022
 *      Author: Mehmed Blazevic
 */

#ifndef APPLICATION_BMS_H_
#define APPLICATION_BMS_H_


void bmsLbInit();
void bmsLbInteg(bool active, float dt);
float bmsLbGetSoc();

#endif /* APPLICATION_BMS_H_ */
