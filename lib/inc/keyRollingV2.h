/*
 * keyRollingV2.h
 *
 *  Created on: 11 mars 2023
 *      Author: Mehmed Blazevic
 */

#ifndef APPLICATION_KEYROLLINGV2_H_
#define APPLICATION_KEYROLLINGV2_H_

#define HEADER_SIZE 2
#define KEY_SIZE 10
#define KEY_MAC_SIZE 8
#define NB_KEY 8
#define KEY_NB_MAX_NOT_RECEIVED 64

typedef enum
{
    KEY_UNVALIDE,
    KEY_POWER_ON,
    KEY_POWER_OFF,
    KEY_OPEN,
    KEY_CLOSE,
} t_keyComand;

void keyInit();
void keySendCommand(uint8_t* paquet,t_keyComand cmd);
t_keyComand keyReceiveCommand(uint8_t* paquet);
void crypte(uint8_t* input, uint8_t* output, uint8_t* mac);
bool decrypte(uint8_t* output, uint8_t* input, uint8_t* mac);

#endif /* APPLICATION_KEYROLLINGV2_H_ */
