/*
 * cpu_variante_liste.h
 *
 *  Created on: 7 févr. 2022
 *      Author: Mehmed Blazevic
 */

#ifndef CPU_VARIANTE_LISTE_H_
#define CPU_VARIANTE_LISTE_H_

#include "ti_drivers_config.h"

#ifdef CONFIG_CC2642R1FRGZ
#define CPU_VARIANTE 1
#endif

#ifdef CONFIG_CC2652RB1FRGZ
#define CPU_VARIANTE 2
#endif



#endif /* CPU_VARIANTE_LISTE_H_ */
