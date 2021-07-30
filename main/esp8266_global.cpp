/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   esp8266_global.cpp
@brief  Global definitions for this project and something not sure where to put
@author Mickey
@date   2021.7.30
@note

Description:
*/

/*=============================================================================
System Includes
=============================================================================*/

#include <cstdlib>
#include <cstdarg>
#include <malloc.h>

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

/*=============================================================================
Local Includes
=============================================================================*/

#include "esp8266_global.h"

/*=============================================================================
Definitions
=============================================================================*/

#ifndef STASSID
#define STASSID "ZXG_Tech1"
#define STAPSK  "ZXG86968188"
#endif

#ifndef APSSID
#define APSSID "ESP_AP"
#define APPSK  "88888888"
#endif

/*--------------------------------------------------------------------------*/

/*! @brief Message structure for task TSK_Log */
typedef struct
{
  /*! Log category ID (see LOG_ID_xxx definitions) */
  UINT8         Log_ID;

  /*! Log level (importance) of log message (see DBG_xxx definitions) */
  UINT8         Log_Level;

  /*! File */
  const char    *pFile;

  /*! Function */
  const char    *pFunction;

  /*! Line */
  int           Line;

  /*! Log timestamp - ms */
  UINT32        Timestamps_ms;

  /*! Log timestamp - ms */
  UINT32        UTC_Timestamps_us;

  /*! Log string length */
  UINT16        LogStringLength;

  /*! Log string content */
  char          *pLogString;

} LOG_INPUT_RECORD;

/* This is the maximum length of the string obtained when FormatString is printed: LOG(,FormatString,...) */
#define MAX_LOG_MESSAGE_LENGTH                128

/* Don't use malloc() or relative functions which will cause stack error!!! */
#define MY_MALLOC(id, ptr, size, buffer)  malloc(size)
#define MY_FREE(id, ptr, size, buffer)    free(ptr)
#define MY_VSNPRINTF                      vsnprintf

/*=============================================================================
Static Variables
=============================================================================*/

/* Index into this array is the debug level as an integer,
   this array is used to create prefix for output log strings */
static char    LogLevelCharacters[DBG_3+1] =
{
  'P', 'C', 'A', 'E', 'W', 'N', 'I', '1', '2', '3'
};

/*=============================================================================
Global Variables
=============================================================================*/

MY_CONFIG_RECORD My_Config;

/*=============================================================================
Static Prototypes
=============================================================================*/

/*=============================================================================
Function Definitions
=============================================================================*/

/*===========================================================================*/

/*!
Generate a log message

@param  Log_ID           Log message category, (I)
@param  Log_Level        Log level, (I)
@param  pFormatString    Format string for log message, (I)
@param  ...              Arguments to format string, (I)
@return None

*/
void
LOG_ID_Handle(  UINT16      Log_ID,
                UINT8       Log_Level,
                const char  *pFile,
                const char  *pFunction,
                int         Line,
                const char  *pFormatString,
                ... )
{
  va_list                   ap;
  UINT16                    LogStringLength;
  char                      pLogString[MAX_LOG_MESSAGE_LENGTH];
  String                    Print_String;
  UINT32                    Now_us;

  /* Ensure Log_ID in valid range */
  if ( Log_ID >= NUM_LOG_IDS )
  {
    Log_ID = LOG_ID_INVALID;
  }

  /* Ensure Log_Level in valid range */
  if ( Log_Level > DBG_3 )
  {
    Log_Level = DBG_3;
  }

  /* Ensure input pointer valid */
  if ( !pFormatString )
  {
    pFormatString = "";
  }

  /* Get current timestamps */
  Now_us = micros();

#if 0
  /* Save information about the log; this info will be sent to TSK_Log
     (which will handle formatting and outputting of the log) */
  Msg_Log.Log_ID            = Log_ID;
  Msg_Log.Log_Level         = Log_Level;
  Msg_Log.pFile             = pFile;
  Msg_Log.pFunction         = pFunction;
  Msg_Log.Line              = Line;
  Msg_Log.Timestamps_ms     = Now_us;

  /* Allocate memory to be used for creation of the log string */
  MY_MALLOC( 0, pLogString, MAX_LOG_MESSAGE_LENGTH, NULL );
#endif

  /* Null terminate array so we can detect if vsnprintf creates string
     which is too long */
  pLogString[MAX_LOG_MESSAGE_LENGTH-1] = 0;

  /* Print log message to buffer */
  va_start(ap, pFormatString);
  (void) MY_VSNPRINTF( pLogString, MAX_LOG_MESSAGE_LENGTH, pFormatString, ap);
  va_end(ap);

  /* If string was too long, then fatal error (should never happen since we used vsnprintf) */
  if ( pLogString[MAX_LOG_MESSAGE_LENGTH-1] != 0 )
  {
    pLogString[MAX_LOG_MESSAGE_LENGTH-1] = 0;
  }

#if 0
  /* Handy for debugging multi-threaded app to sometimes to print logs immediately to /tmp/logfile */
  if ( pDebugFile )
  {
    fprintf( pDebugFile,  "%c,%u,%u.%06u : %s",
                          LogLevelCharacters[Log_Level],
                          Log_ID,
                          (UINT32)Now_us.tv_sec,
                          (UINT32)Now_us.tv_usec,
                          pLogString );
    fflush( pDebugFile );
  }
#elif 1
  Print_String += String(LogLevelCharacters[Log_Level]) + ",";
  Print_String += String(Log_ID) + ",";
  Print_String += "[" + String(float(Now_us)/1000,3) + "]: ";
  Print_String += pLogString;

//  Print_String.format( "%c,%u,[%u.%3d] : %s",
//                        LogLevelCharacters[Log_Level],
//                        Log_ID,
//                        (UINT32)Now_us/1000,
//                        (UINT32)Now_us%1000,
//                        pLogString );

  /* Print logs immediately to console */
  Serial.print(Print_String);
#else
  /* Get string length and add one for the null termination character */
  LogStringLength = strlen(pLogString) + 1;

  /* Allocate memory for log string - we already know the string length
      This memory will be freed in TSK_Log after log is processed */
  MY_MALLOC( 0, Msg_Log.pLogString, LogStringLength, NULL );

  /* Copy string (including null termination byte) to the newly allocated memory */
  memcpy( Msg_Log.pLogString, pLogString, LogStringLength );

  /* Pass string length to Log task */
  Msg_Log.LogStringLength = LogStringLength;

  /* Pass count of number of logs we discarded immediately prior
     to this log */
  Msg_Log.DiscardedLogCount = DiscardedLogCount;

  /* Place the message into the buffer being checked by TSK_Log */
  /* Note that we do not wait: if there is no space in the TSK_Log input
     buffer immediately, then the log will be discarded */
  if ( PACKETBUF_Write( &TSK_Log_Input_Buffer, (UINT8*)&Msg_Log, sizeof(TSK_LOG_INPUT_RECORD), 0 ) == FALSE )
  {
    /* Buffer is full, write to buffer failed */

    /* Increment count of discarded logs (unless it is already at maximum UINT8 value) */
    if ( DiscardedLogCount != 0xFF )
    {
#warning todo change to atomic increment
      DiscardedLogCount++;
    }

    /* Free memory to hold log message string - this would normally be
       freed in TSK_Log - however there is currently no space in the
       TSK_Log input buffer */
    MY_FREE( 0, Msg_Log.pLogString, Msg_Log.LogStringLength, NULL );
  }
  else
  {
    /* Write to buffer was successful - reset count of discarded logs */
    DiscardedLogCount = 0;
  }
#endif

#if 0
  /* Free space for memory in which we formed the log string */
  MY_FREE( 0, pLogString, MAX_LOG_MESSAGE_LENGTH, NULL );
#endif
}

/*===========================================================================*/

void
My_Config_Set_Defaults( MY_CONFIG_RECORD *pConfig )
{
  /* Use the compile-time defaults */
//  memcpy( pConfig, &My_Config_Default, sizeof(MY_CONFIG_RECORD) );.
  pConfig->format_version   = MY_CONFIG_FORMAT_VERSION;
  strcpy( pConfig->sta_ssid,  (CHAR *)STASSID );
  strcpy( pConfig->sta_pwd,   (CHAR *)STAPSK );
  strcpy( pConfig->ap_ssid,   (CHAR *)APSSID );
  strcpy( pConfig->ap_pwd,    (CHAR *)APPSK );
}

/*===========================================================================*/

UINT8
My_Config_Validate( MY_CONFIG_RECORD  *pConfig )
{
  /* Format of this file */
  if ( pConfig->format_version != MY_CONFIG_FORMAT_VERSION)
  {
    LOG( DBG_E, "Different HI format version, old is %d , new is %d\n",
                 MY_CONFIG_FORMAT_VERSION,
                 pConfig->format_version );
    return(FN_RETURN_ERROR);
  }

  /* No problems discovered above - return OK */
  return(FN_RETURN_OK);
}

/*============================================================================*/

UINT8
My_Config_Save( MY_CONFIG_RECORD  *pConfig )
{
  UINT8   data_buff[512];
  UINT16  data_len = sizeof(MY_CONFIG_RECORD);
  UINT16  count;

  EEPROM.begin(512);

  /* Check length */
  if( data_len >= 512 )
  {
    LOG( DBG_E, "My Config is too large, size=%d bytes\n", data_len );
    return(FN_RETURN_ERROR);
  }

  /* Write into buff */
  memcpy( data_buff, pConfig, sizeof(MY_CONFIG_RECORD) );
  for ( count = 0; count < data_len; count++ )
  {
    EEPROM.write(count, data_buff[count]);
  }

  /* Commit into flash */
  if (EEPROM.commit())
  {
    LOG( DBG_N, "EEPROM successfully committed\n");
    return(FN_RETURN_OK);
  }
  else
  {
    LOG( DBG_E, "ERROR! EEPROM commit failed\n");
    return(FN_RETURN_ERROR);
  }

}

/*============================================================================*/

UINT8
My_Config_Load( MY_CONFIG_RECORD  *pConfig )
{
  UINT8   data_buff[512];
  UINT16  data_len = sizeof(MY_CONFIG_RECORD);
  UINT16  count;

  EEPROM.begin(512);

  /* Check length */
  if( data_len >= 512 )
  {
    LOG( DBG_E, "My Config is too large, size=%d bytes\n", data_len );
    return(FN_RETURN_ERROR);
  }

  /* Read from flash */
  for ( count = 0; count < data_len; count++ )
  {
    data_buff[count] = EEPROM.read(count);
  }
  memcpy( pConfig, data_buff, sizeof(MY_CONFIG_RECORD) );

  /* Validate the config from flash */
  if ( My_Config_Validate(pConfig) )
  {
    LOG( DBG_N, "Available Config read from EEPROM\n");
    return(FN_RETURN_OK);
  }
  else
  {
    LOG( DBG_E, "Bad Config read from EEPROM\n");
    return(FN_RETURN_ERROR);
  }

}

/*============================================================================*/

void
My_Config_Initialise(void)
{
  MY_CONFIG_RECORD  Config;
  UINT8             Status;

  Status = My_Config_Load( &Config );

  if ( Status != FN_RETURN_OK )
  {
    LOG( DBG_P, "My config from flash not found or not parsed OK, using default config\n" );

    My_Config_Set_Defaults( &Config );

    My_Config_Save( &Config );
  }

  memcpy( &My_Config, &Config, sizeof(MY_CONFIG_RECORD) );

  LOG( DBG_P, "My config Initialise Complete.\n" );
}

/*============================================================================*/

void
Wifi_Initialise( void )
{
  /* Set wifi mode to support both AP and STA */
  WiFi.mode(WIFI_AP_STA);

/*---------------------------------------------------------------------------*/

  /* Init the wifi AP function */
  LOG( DBG_A, "Configuring Access Point...\n" );
  /* You can remove the password parameter if you want the AP to be open. */
//  WiFi.softAP(ap_ssid, ap_password);
  WiFi.softAP( My_Config.ap_ssid );

  IPAddress myIP = WiFi.softAPIP();
  LOG( DBG_A, "AP IP address: %s\n", myIP.toString().c_str() );

/*---------------------------------------------------------------------------*/

  /* Init the wifi STA function */
  WiFi.begin( My_Config.sta_ssid , My_Config.sta_pwd );

  /* Wait for connection */
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");

  LOG( DBG_A, "Connected to: %s\n", My_Config.sta_ssid );
  LOG( DBG_A, "IP address: %s\n", WiFi.localIP().toString().c_str() );
}

/*============================================================================*/

