/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   esp8266_global.h
@brief  Global definitions for this project
@author Mickey
@date   2021.7.29
@note

Description:
*/

#ifndef __ESP8266_GLOBAL_H__
#define __ESP8266_GLOBAL_H__

/*=============================================================================
System Includes
=============================================================================*/

/*=============================================================================
Local Includes
=============================================================================*/

#include "esp8266_12f_bsp.h"
#include "logging.h"

/*=============================================================================
Definitions
=============================================================================*/

/* Software Version */
#define SW_REVISION                 "1.0"

/* The configuration format versions understood by this software release */
#define MY_CONFIG_FORMAT_VERSION    2
#define MY_STATUS_FORMAT_VERSION    1

/*--------------------------------------------------------------------------*/

#define TRUE  1
#define FALSE 0

#define FN_RETURN_ERROR FALSE
#define FN_RETURN_OK    TRUE

/*--------------------------------------------------------------------------*/

typedef signed char             INT8;
typedef signed short            INT16;
typedef signed long             INT32;
typedef signed long long int    INT64;
typedef unsigned char           UINT8;
typedef unsigned short          UINT16;
typedef unsigned long           UINT32;
typedef unsigned long long int  UINT64;
typedef char                    CHAR;
typedef float                   FLOAT;
typedef double                  DOUBLE;
typedef bool                    BOOL;

#define CIRCULAR_INC( Var, Size ) ((Var)==((Size)-1)) ? 0 : ((Var)+1)
#define CIRCULAR_DEC( Var, Size ) ((Var)==0) ? ((Size)-1)) : ((Var)-1)

/*--------------------------------------------------------------------------*/

/* Relay timing record */
typedef struct
{
  /* Format of this record */
  UINT16  format_version;

  /* If this timing is valid */
  BOOL    valid;

  /* Hours 0-23 */
  UINT32  hh;

  /* Minutes 0-59 */
  UINT32  mm;

} RELAY_TIMING_RECORD;

/*--------------------------------------------------------------------------*/

#define WIFI_SSID_STR_MAX_SIZE  32
#define WIFI_PWD_STR_MAX_SIZE   16

/* My config record */
typedef struct
{
  /* Format of this record */
  UINT16  format_version;

  /* Wifi config */
  CHAR    sta_ssid[WIFI_SSID_STR_MAX_SIZE];
  CHAR    sta_pwd[WIFI_PWD_STR_MAX_SIZE];
  CHAR    ap_ssid[WIFI_SSID_STR_MAX_SIZE];
  CHAR    ap_pwd[WIFI_PWD_STR_MAX_SIZE];

  /* Relay timing control config */
  BOOL    relay_auto;

  /* Relay on timing */
  RELAY_TIMING_RECORD relay_on_timing;

  /* Relay off timing */
  RELAY_TIMING_RECORD relay_off_timing;

  /* When distance is high than this value(water level is low), turn off relay */
  FLOAT   high_distance_cm;

  /* When distance is low than this value(water level is high), turn on relay */
  FLOAT   low_distance_cm;

} MY_CONFIG_RECORD;

/*--------------------------------------------------------------------------*/

#define LOCALTIME_STR_MAX_SIZE  20

/* My status record */
typedef struct
{
  /* Format of this record */
  UINT16  format_version;

  /* UTC+8 timestamps seconds, get from Internet */
  UINT32  local_timestamp_s;

  /* Local time(UTC+8) string, format 'YYYY-MM-DD hh:mm:ss' */
  CHAR    local_time_str[LOCALTIME_STR_MAX_SIZE];

  /*--------------------------------------------------------------------------*/

  /* Current relay status, TRUE-On;FALSE-Off */
  BOOL    relay_status;

  /* Current led status, TRUE-On;FALSE-Off */
  BOOL    led_green_status;
  BOOL    led_blue_status;
  BOOL    led_red_status;

  /*--------------------------------------------------------------------------*/

  /* Internet status, TRUE-valid;FALSE-invalid */
  BOOL    internet_status;

  /* The wifi(sta) status */
  UINT8   current_wifi_status;

  /* Current SSID when esp8266 is connected */
  CHAR    current_sta_ssid[WIFI_SSID_STR_MAX_SIZE];

  /* Current IP when esp8266 is connected */
  CHAR    current_sta_ip[16];

  /*--------------------------------------------------------------------------*/

//  /* Relay timing control config */
//  boolean relay_auto;

  /*--------------------------------------------------------------------------*/

  /* Wether raw distance is valid or not */
  BOOL    distance_valid;

  /* Raw distance */
  FLOAT   raw_distance_cm;

  /* Average distance */
  FLOAT   avg_distance_cm;

} MY_STATUS_RECORD;

/*--------------------------------------------------------------------------*/


/*=============================================================================
Global References
=============================================================================*/

extern MY_CONFIG_RECORD My_Config;
extern MY_STATUS_RECORD My_Status;

/*=============================================================================
Prototypes
=============================================================================*/

extern UINT8
My_Config_Save( MY_CONFIG_RECORD  *pConfig );

extern void
My_Config_Initialise(void);

extern void
Wifi_Initialise( void );

extern void
GPIO_Initialise( void );

extern void
SR04_Initialise( void );

extern FLOAT
SR04_Get_Distance( void );

#endif  /* __ESP8266_GLOBAL_H__ */

/*===========================================================================*/
