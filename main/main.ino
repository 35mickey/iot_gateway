
/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   main.ino
@brief  main file of the iot_gateway
@author Mickey
@date   2021.7.28
@note

Description:
*/

/*=============================================================================
System Includes
=============================================================================*/

#include <stdio.h>

#include <ESPDateTime.h>
#include <DateTime.h>

/*=============================================================================
Local Includes
=============================================================================*/

#include "http_server.h"
#include "mqtt_client.h"
#include "esp8266_global.h"

/*=============================================================================
Definitions
=============================================================================*/

/*=============================================================================
Static Variables
=============================================================================*/

/*=============================================================================
Global Variables
=============================================================================*/

/* All update timestamps */
u32 last_led_flash_timestamp_ms       = 0;
u32 last_measure_sonar_timestamp_ms   = 0;
u32 last_average_sonar_timestamp_ms   = 0;
u32 last_time_str_update_timestamp_ms = 0;
u32 last_sync_ntp_timestamp_ms        = 0;
u32 last_mqtt_report_timestamp_ms     = 0;
u32 last_mqtt_connect_timestamp_ms    = 0;

/*=============================================================================
Static Prototypes
=============================================================================*/

/*=============================================================================
Function Definitions
=============================================================================*/

/*===========================================================================*/

static void Setup_DateTime()
{
  // setup this after wifi connected
  // you can use custom timeZone,server and timeout
  // DateTime.setTimeZone(-4);
  //   DateTime.setServer("asia.pool.ntp.org");
  //   DateTime.begin(15 * 1000);
  DateTime.setServer("time.pool.aliyun.com");
  DateTime.setTimeZone("CST-8");
  DateTime.begin();
  if (!DateTime.isTimeValid())
  {
    LOG( DBG_E, "DateTime: Failed to get time from server.\n");
  }
  else
  {
    LOG( DBG_E, "DateTime: DateTime is %s\n", DateTime.toString().c_str() );
    LOG( DBG_E, "DateTime: Timestamp is %ld\n", DateTime.now() );
  }
}

/*===========================================================================*/

void setup()
{
  delay(100);

  /*-----------------------------------------------------------------------------
   Power On Initialisation
  -----------------------------------------------------------------------------*/
  Serial.begin(115200);

  /* Init GPIOs */
  GPIO_Initialise();

  LOG( DBG_P, "ESP8266 Iot gateway starting, version %s\n", SW_REVISION );

  /*-----------------------------------------------------------------------------
   System Initialisation
  -----------------------------------------------------------------------------*/

  /* Clean all the status */
  memset( &My_Status, 0 ,sizeof(MY_STATUS_RECORD) );

  /* Init all local configs */
  My_Config_Initialise();

  /* Init wifi configs */
  Wifi_Initialise();

  /* Start NTP client and do update once */
  Setup_DateTime();

  /* Init the HTTP server */
  http_server_init();

  /* Init the MQTT client */
  mqtt_client_init();

  /* Init sonar HC-SR04 */
  SR04_Initialise();

  /*---------------------------------------------------------------------------*/

}

/*===========================================================================*/

void loop()
{
  /* Every 1 sec, flash LED */
  if ( (millis() - last_led_flash_timestamp_ms ) > 1000 )
  {
    last_led_flash_timestamp_ms = millis();
    int led_state = digitalRead(GPIO_LED_BLUE);
    digitalWrite(GPIO_LED_BLUE, !led_state);
  }

  /*---------------------------------------------------------------------------*/

  /* Apply the status of relay */
  if ( My_Status.relay_status == TRUE )
  {
    digitalWrite(GPIO_RELAY,    HIGH);
    digitalWrite(GPIO_LED_RED,  HIGH);
  }
  else
  {
    digitalWrite(GPIO_RELAY,    LOW);
    digitalWrite(GPIO_LED_RED,  LOW);
  }

  /*---------------------------------------------------------------------------*/

#if 0
  /* Every 10 mins, update NTP time */
  if ( (millis() - last_sync_ntp_timestamp_ms ) > (1000*60*10) )
  {
    last_sync_ntp_timestamp_ms = millis();
    NTP_Client.update();
  }
#endif

  /*---------------------------------------------------------------------------*/

  /* Every 1 sec, transform the timestamp to string */
  if ( (millis() - last_time_str_update_timestamp_ms ) > (1000) )
  {
    last_time_str_update_timestamp_ms = millis();
    if ( !DateTime.isTimeValid() )
    {
      LOG( DBG_E, "Failed to get time from server, retry.\n");
      DateTime.begin();
    }
    else
    {
      My_Status.local_timestamp_s = DateTime.now();
      sprintf( My_Status.local_time_str, "%s", DateTime.toString().c_str() );
      LOG( DBG_N, "DateTime: %s\n", My_Status.local_time_str );
    }
  }

  /*---------------------------------------------------------------------------*/

//  /* Update sonar */
//  if ( (millis() - last_measure_sonar_timestamp_ms ) > 1000 )
//  {
//    last_measure_sonar_timestamp_ms = millis();
//    My_Status.raw_distance_cm = SR04_Get_Distance();
//  }

  /* Web page handle */
  http_handle_client();

  /* MQTT server communication and subscribe handle */
  mqtt_handle_client();
}

