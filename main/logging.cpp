/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   logging.cpp
@brief  Logging function implement
@author Mickey
@date   2021.7.31
@note

Description:
*/

/*=============================================================================
System Includes
=============================================================================*/

#include <cstdlib>
#include <cstdarg>
#include <malloc.h>
#include <Arduino.h>

/*=============================================================================
Local Includes
=============================================================================*/

#include "logging.h"

/*=============================================================================
Definitions
=============================================================================*/

/*! @brief Message structure for task TSK_Log */
typedef struct
{
  /*! Log category ID (see LOG_ID_xxx definitions) */
  unsigned char Log_ID;

  /*! Log level (importance) of log message (see DBG_xxx definitions) */
  unsigned char Log_Level;

  /*! File */
  const char    *pFile;

  /*! Function */
  const char    *pFunction;

  /*! Line */
  int           Line;

  /*! Log timestamp - ms */
  unsigned long Timestamps_ms;

  /*! Log timestamp - ms */
  unsigned long UTC_Timestamps_us;

  /*! Log string length */
  unsigned int  LogStringLength;

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
LOG_ID_Handle(  unsigned int  Log_ID,
                unsigned char Log_Level,
                const char    *pFile,
                const char    *pFunction,
                int           Line,
                const char    *pFormatString,
                ... )

{
  va_list                   ap;
  unsigned int              LogStringLength;
  char                      pLogString[MAX_LOG_MESSAGE_LENGTH];
  String                    Print_String;
  unsigned long             Now_us;

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
