/*
 * appBootLoader.h
 *
 *  Created on: 2 mai 2022
 *      Author: Mehmed Blazevic
 */

#ifndef APPLICATION_APPBOOTLOADER_H_
#define APPLICATION_APPBOOTLOADER_H_

#include "cpu_variante_liste.h"

#define VERSION_MAJOR  1 //xx
#define VERSION_MINOR  2 //xxx
#define VERSION_PATCH  47 //xxx
#define VERSIONGCF     1

#define VERSION (VERSION_MAJOR*1000*1000+VERSION_MINOR*1000+VERSION_PATCH)
#define MAGIC_NUMBER (0xDEADBEEF+CPU_VARIANTE) //

#endif /* APPLICATION_APPBOOTLOADER_H_ */
