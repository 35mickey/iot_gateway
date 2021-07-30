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

#include "Arduino.h"

/*=============================================================================
Local Includes
=============================================================================*/

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

/*--------------------------------------------------------------------------*/

/*
The debug levels should be used as follows:
DBG_P - panic - system unstable (normally reserved for kernel processes)
DBG_A - alert - system problem, action must be taken immediately
DBG_C - critical - system critical condition (e.g. lack of resources)
DBG_E - error - unrecoverable error, process will shut down
DBG_W - warning - recoverable error, process will continue
DBG_N - notice - normal but significant event occurred (possibly opposite of DBG_E, e.g. device opened/closed)
DBG_I - info - information (possibly useful for operator to know, e.g. updated init file contents)
DBG_1 - debug 1 - occasional debug info, e.g. 1 per second
DBG_2 - debug 2 - detailed debug info, e.g. 10 per second
DBG_3 - debug 3 - voluminous debug info, as much as required
*/
/* Debug levels */
#define DBG_P 0    /* panic */
#define DBG_A 1    /* alert */
#define DBG_C 2    /* critical */
#define DBG_E 3    /* error */
#define DBG_W 4    /* warning */
#define DBG_N 5    /* notice */
#define DBG_I 6    /* info */
#define DBG_1 7    /* debug 1 */
#define DBG_2 8    /* debug 2 */
#define DBG_3 9    /* debug 3 */

/* Number of log ID's */
/* Each module will store a copy of the log levels for all the log ID's used in the product (including those of other processes) */
#define LOG_ID_INVALID    0
#define LOG_ID_DEFAULT    1
#define NUM_LOG_IDS       2

#define LOG_ID(Log_ID, Log_Level, FormatString, ...)  LOG_ID_Handle( Log_ID, Log_Level, __FILE__, __func__, __LINE__, FormatString, ##__VA_ARGS__ )
#define LOG(Log_Level, FormatString, ...)             LOG_ID_Handle( LOG_ID_DEFAULT, Log_Level, __FILE__, __func__, __LINE__, FormatString, ##__VA_ARGS__ )

/*--------------------------------------------------------------------------*/

/* Relay timing record */
typedef struct
{

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

  /* Distance config */

} MY_CONFIG_RECORD;

/*--------------------------------------------------------------------------*/

/* My status record */
typedef struct
{
  /* Format of this record */
  UINT16     format_version;

//  /* Wifi config */
//  String  sta_ssid;
//  String  sta_pwd;
//  String  ap_ssid;
//  String  ap_pwd;
//
//  /* Relay timing control config */
//  boolean relay_auto;
//
//  /* Distance config */

} MY_STATUS_RECORD;

/*--------------------------------------------------------------------------*/


/*=============================================================================
Global References
=============================================================================*/

extern MY_CONFIG_RECORD My_Config;

/*=============================================================================
Prototypes
=============================================================================*/

extern void
LOG_ID_Handle(  UINT16      Log_ID,
                UINT8       Log_Level,
                const char  *pFile,
                const char  *pFunction,
                int         Line,
                const char  *pFormatString,
                ... );

extern UINT8
My_Config_Save( MY_CONFIG_RECORD  *pConfig );

extern void
My_Config_Initialise(void);

extern void
Wifi_Initialise( void );

#endif  /* __ESP8266_GLOBAL_H__ */

/*===========================================================================*/
