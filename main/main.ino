
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
  /* Flash LED */
  if ( (millis() - last_led_flash_timestamp_ms ) > 1000 )
  {
    last_led_flash_timestamp_ms = millis();
    int led_state = digitalRead(GPIO_LED_BLUE);
    digitalWrite(GPIO_LED_BLUE, !led_state);

    mqtt_publish("original_distance","535");
  }

//  /* Update sonar */
//  if ( (millis() - last_measure_sonar_timestamp_ms ) > 1000 )
//  {
//    last_measure_sonar_timestamp_ms = millis();
//    My_Status.raw_distance_cm = SR04_Get_Distance();
//  }

  http_handle_client();

  mqtt_handle_client();
}
