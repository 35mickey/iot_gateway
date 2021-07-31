/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   logging.h
@brief  Logging definitions
@author Mickey
@date   2021.7.31
@note

Description:
*/

#ifndef __LOGGING_H__
#define __LOGGING_H__

/*=============================================================================
System Includes
=============================================================================*/

/*=============================================================================
Local Includes
=============================================================================*/

/*=============================================================================
Definitions
=============================================================================*/

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

/*=============================================================================
Global References
=============================================================================*/

/*=============================================================================
Prototypes
=============================================================================*/

extern void
LOG_ID_Handle(  unsigned int  Log_ID,
                unsigned char Log_Level,
                const char    *pFile,
                const char    *pFunction,
                int           Line,
                const char    *pFormatString,
                ... );

#endif  /* __LOGGING_H__ */

/*===========================================================================*/
