/*
 * crc16Vesc.h
 *
 *  Created on: 27 juin 2022
 *      Author: Mehmed Blazevic
 */

#ifndef PROFILES_CRC16VESC_H_
#define PROFILES_CRC16VESC_H_


unsigned short crc16Vesc(unsigned char *buf, unsigned int len);

void crc16VescInit();
void crc16VescFeed(unsigned char data);
unsigned short crc16VescGet();


#endif /* PROFILES_CRC16VESC_H_ */
