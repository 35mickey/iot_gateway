/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   esp8266_12f_bsp.h
@brief  Board support package definitions for 12F
@author Mickey
@date   2021.7.28
@note

Description:
*/

#ifndef __ESP8266_12F_BSP_H__
#define __ESP8266_12F_BSP_H__

/*=============================================================================
System Includes
=============================================================================*/

#include <EEPROM.h>

/*=============================================================================
Local Includes
=============================================================================*/

/*=============================================================================
Definitions
=============================================================================*/

/* GPIO definitions */
#define GPIO_LED_GREEN  12
#define GPIO_LED_BLUE   13
#define GPIO_LED_RED    15
#define GPIO_RELAY      14
#define GPIO_TRIG       5
#define GPIO_ECHO       4     /* It's user key on eval board */

#define GPIO_HIGH     1
#define GPIO_LOW      0

/* EEPROM definitions */
/* Each byte of the EEPROM can only hold a value from 0 to 255. */
#define EEPROM_CONFIG_SIZE    512
#define EEPROM_CONFIG_ADDR    0

/*=============================================================================
Global References
=============================================================================*/

/*=============================================================================
Prototypes
=============================================================================*/



#endif  /* __ESP8266_12F_BSP_H__ */

/*===========================================================================*/
