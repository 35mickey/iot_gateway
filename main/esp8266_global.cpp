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
#define STASSID "ZXG"
#define STAPSK  "86968188"
#endif

#ifndef APSSID
#define APSSID "ESP_AP"
#define APPSK  "88888888"
#endif

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
MY_STATUS_RECORD My_Status;

/*=============================================================================
Static Prototypes
=============================================================================*/

/*=============================================================================
Function Definitions
=============================================================================*/

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
    LOG( DBG_N, "Avaliable Config read from EEPROM\n");
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
  u32 std_init_timestamp_ms = 0;

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
  std_init_timestamp_ms = millis();

  /* Wait for connection */
  while (WiFi.status() != WL_CONNECTED)
  {
    /* Wait connect for at most 10 seconds */
    if ( (millis() - std_init_timestamp_ms ) > 10000 )
    {
      break;
    }

    digitalWrite(GPIO_LED_BLUE, !digitalRead(GPIO_LED_BLUE));

    delay(300);
    Serial.print(".");
  }

  Serial.println("");

  if ( WiFi.status() != WL_CONNECTED )
  {
    LOG( DBG_A, "Connect failed, bad SSID %s!\n", My_Config.sta_ssid );
    digitalWrite(GPIO_LED_RED, HIGH);
  }
  else
  {
    LOG( DBG_A, "Connected to: %s\n", My_Config.sta_ssid );
    LOG( DBG_A, "IP address: %s\n", WiFi.localIP().toString().c_str() );
    digitalWrite(GPIO_LED_RED, LOW);
  }

  LOG( DBG_P, "Wifi Initialise Complete.\n" );
}

/*============================================================================*/

void
GPIO_Initialise( void )
{
  pinMode(GPIO_LED_GREEN,  OUTPUT);
  pinMode(GPIO_LED_BLUE,  OUTPUT);
  pinMode(GPIO_LED_RED,  OUTPUT);
  pinMode(GPIO_RELAY,     OUTPUT);
  pinMode(GPIO_ECHO,      OUTPUT);

  pinMode(GPIO_ECHO,      INPUT);

  /* Disable all GPIOs */
  digitalWrite(GPIO_LED_GREEN,   LOW);
  digitalWrite(GPIO_LED_BLUE,   LOW);
  digitalWrite(GPIO_LED_RED,   LOW);
  digitalWrite(GPIO_RELAY,      LOW);
  digitalWrite(GPIO_ECHO,       LOW);
}
