
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

/*=============================================================================
Local Includes
=============================================================================*/

#include "http_server.h"
#include "mqtt_client.h"
#include "esp8266_global.h"

/*=============================================================================
Definitions
=============================================================================*/

/* The deepth of sonar distance window LPF */
#define DISTANCE_WINDOW_LPF_WIDTH 10

/*=============================================================================
Static Variables
=============================================================================*/

static  FLOAT Sonar_Distance_History[DISTANCE_WINDOW_LPF_WIDTH];
static  UINT8 Sonar_Distance_History_Index=0;

/*=============================================================================
Global Variables
=============================================================================*/

/* The interval time in LED flashing
   Differnt interval for different states */
u32 led_flash_interval_ms             = 500;

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
  DateTime.begin(3 * 1000);
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

  /* Init the HTTP server */
  http_server_init();

  /* Init the MQTT client */
  mqtt_client_init();

  /* Init sonar HC-SR04 */
  SR04_Initialise();

  /*---------------------------------------------------------------------------*/

  delay(3000);

  /* Start NTP client and do update once, delay sometime after Wifi initlised */
  Setup_DateTime();

  /* Initlise all the variables */
  My_Status.raw_distance_cm = SR04_Get_Distance();
  memset( Sonar_Distance_History, My_Status.raw_distance_cm, DISTANCE_WINDOW_LPF_WIDTH);

}

/*===========================================================================*/

void loop()
{
  UINT32  Count;
  BOOL    Ret = TRUE;

  UINT32        Current_Hour;
  UINT32        Current_Minute;
  UINT32        Today_Minutes;        /* Mintues since the beginning of today */
  UINT32        Timing_On_Minutes;
  UINT32        Timing_Off_Minutes;
  DateTimeParts Current_Parts = DateTime.getParts();

  /* Every 1 sec, flash LED and check wifi state
     If Wifi is disconnected, flash quickly */
  if ( (millis() - last_led_flash_timestamp_ms ) > led_flash_interval_ms )
  {
    last_led_flash_timestamp_ms = millis();
    int led_state = digitalRead(GPIO_LED_BLUE);
    digitalWrite(GPIO_LED_BLUE, !led_state);
  }

  /*---------------------------------------------------------------------------*/

  /* Every 5 sec, Publish MQTT messages */
   if ( (millis() - last_mqtt_report_timestamp_ms ) > (1000*5) )
  {
    last_mqtt_report_timestamp_ms = millis();

    /* Alive status */
    Ret &= mqtt_publish("alive_status", "on");

    /* Relay status */
    Ret &= mqtt_publish("relay_status", My_Status.relay_status?"on":"off");

    /* Relay auto config */
    Ret &= mqtt_publish("auto_control_relay", My_Config.relay_auto?"true":"false");

    /* Publish timing config or sonar distance according to relay_auto config */
    if ( My_Config.relay_auto == TRUE )
    {
      /* Sonar raw and average distance */
      Ret &= mqtt_publish("raw_distance", String(My_Status.raw_distance_cm, 2) );
      Ret &= mqtt_publish("avg_distance", String(My_Status.avg_distance_cm, 2) );
    }
    else
    {
      Ret &= mqtt_publish("relay_timing_on_enable", My_Config.relay_on_timing.valid?"true":"false");
      Ret &= mqtt_publish("relay_timing_on_time", String( (My_Config.relay_on_timing.hh +
                                                     (FLOAT)My_Config.relay_on_timing.mm/60), 1));

      Ret &= mqtt_publish("relay_timing_off_enable", My_Config.relay_off_timing.valid?"true":"false");
      Ret &= mqtt_publish("relay_timing_off_time", String( (My_Config.relay_off_timing.hh +
                                                      (FLOAT)My_Config.relay_off_timing.mm/60), 1));
    }

    LOG( DBG_N, "MQTT: Publish %s every 5 sec.\n", Ret?"success":"failed" );

    /* Publish failed then flash quickly */
    if ( Ret == TRUE )
    {
      led_flash_interval_ms = 500;
    }
    else
    {
      led_flash_interval_ms = 200;
    }

  }

  /*---------------------------------------------------------------------------*/

  /* Every 30 mins, update NTP time */
  if ( (millis() - last_sync_ntp_timestamp_ms ) > (1000*60*30) )
  {
    last_sync_ntp_timestamp_ms = millis();
    if( DateTime.begin(500) == false )
    {
      LOG( DBG_E, "NTP: Sync NTP failed.\n");
    }
    else
    {
      LOG( DBG_N, "NTP: Sync NTP success.\n");
    }
  }

  /*---------------------------------------------------------------------------*/

  /* Every 10 secs, transform the timestamp to string */
  if ( (millis() - last_time_str_update_timestamp_ms ) > (1000*10) )
  {
    last_time_str_update_timestamp_ms = millis();
    if ( !DateTime.isTimeValid() )
    {
      LOG( DBG_E, "NTP: Failed to get time from server, retry.\n");
      if( DateTime.begin(500) == false )
      {
        LOG( DBG_E, "NTP: Sync NTP failed.\n");
      }
    }
    else
    {
      My_Status.local_timestamp_s = DateTime.now();
      sprintf( My_Status.local_time_str, "%s", DateTime.toString().c_str() );
      LOG( DBG_I, "NTP: DateTime: %s\n", My_Status.local_time_str );
    }
  }

  /*---------------------------------------------------------------------------*/

#if 1
  /* Every 1 sec, update sonar distance and average the sonar distance */
  if ( (millis() - last_measure_sonar_timestamp_ms ) > 1000 )
  {
    last_measure_sonar_timestamp_ms = millis();
    My_Status.raw_distance_cm = SR04_Get_Distance();
    My_Status.distance_valid  = (My_Status.raw_distance_cm==0)?FALSE:TRUE;

    /* Update average distance */
    if ( My_Status.distance_valid == TRUE )
    {
      /* Update history records */
      Sonar_Distance_History[Sonar_Distance_History_Index] = My_Status.raw_distance_cm;
      Sonar_Distance_History_Index = CIRCULAR_INC( Sonar_Distance_History_Index, DISTANCE_WINDOW_LPF_WIDTH);

      /* Do average */
      My_Status.avg_distance_cm = 0;
      for (Count = 0; Count < DISTANCE_WINDOW_LPF_WIDTH; ++Count)
      {
        My_Status.avg_distance_cm += Sonar_Distance_History[Count];
      }
      My_Status.avg_distance_cm = My_Status.avg_distance_cm/DISTANCE_WINDOW_LPF_WIDTH;
    }
  }
#endif

  /*---------------------------------------------------------------------------*/

  /* Operate the delay according to sonar distance when relay_auto is on
     Operate the delay according to timing when relay_auto is off */
  if ( My_Config.relay_auto == TRUE )
  {
    if ( My_Status.distance_valid == TRUE )
    {
      /* More than high distance means low water level, turn off relay
         Less than low distance means high water level, turn on relay
         To avoid switch relay frequently */
      if ( My_Status.avg_distance_cm > My_Config.high_distance_cm )
      {
        My_Status.relay_status = FALSE;
      }

      if ( My_Status.avg_distance_cm < My_Config.low_distance_cm )
      {
        My_Status.relay_status = TRUE;
      }
    }
    else
    {
      My_Status.relay_status = FALSE;
    }
  }
  else
  {
    Current_Hour    = (UINT32)Current_Parts.getHours();
    Current_Minute  = (UINT32)Current_Parts.getMinutes();

    /* If both timing on and off are enabled, operate relay in time range
       else operate relay by time point.
       Because if only refer to time point, the relay status may be wrong after ESP reboot */
    if ( (My_Config.relay_on_timing.valid == TRUE) && (My_Config.relay_off_timing.valid == TRUE) )
    {
      Today_Minutes       = Current_Hour*60 + Current_Minute;
      Timing_On_Minutes   = My_Config.relay_on_timing.hh*60 + My_Config.relay_on_timing.mm;
      Timing_Off_Minutes  = My_Config.relay_off_timing.hh*60 + My_Config.relay_off_timing.mm;

      if( Timing_Off_Minutes >= Timing_On_Minutes )
      {
        /* NOT include 00:00 */
        if ( (Today_Minutes >= Timing_On_Minutes) && (Today_Minutes < Timing_Off_Minutes) )
        {
          My_Status.relay_status = TRUE;
        }
        else
        {
          My_Status.relay_status = FALSE;
        }
      }
      else
      {
        /* Include 00:00 */
        if ( (Today_Minutes >= Timing_On_Minutes) || (Today_Minutes < Timing_Off_Minutes) )
        {
          My_Status.relay_status = TRUE;
        }
        else
        {
          My_Status.relay_status = FALSE;
        }
      }
    }
    else
    {
      /* Check if it is time to turn on/off relay */

      if ( My_Config.relay_on_timing.valid )
      {
        if ( (My_Config.relay_on_timing.hh == Current_Hour) && (My_Config.relay_on_timing.mm == Current_Minute) )
        {
          My_Status.relay_status = TRUE;
        }
      }

      if ( My_Config.relay_off_timing.valid )
      {
        if ( (My_Config.relay_off_timing.hh == Current_Hour) && (My_Config.relay_off_timing.mm == Current_Minute) )
        {
          My_Status.relay_status = FALSE;
        }
      }
    }
  }

  /*---------------------------------------------------------------------------*/

  /* Apply the status of relay */
  if ( My_Status.relay_status == TRUE )
  {
    digitalWrite(GPIO_RELAY,    HIGH);
//    digitalWrite(GPIO_LED_RED,  HIGH);
  }
  else
  {
    digitalWrite(GPIO_RELAY,    LOW);
//    digitalWrite(GPIO_LED_RED,  LOW);
  }

  /*---------------------------------------------------------------------------*/

  /* Web page handle */
  http_handle_client();

  /* MQTT server communication and subscribe handle */
  mqtt_handle_client();
}

