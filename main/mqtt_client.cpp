/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   mqtt_client.cpp
@brief  MQTT initialise and callback operations
@author Mickey
@date   2022.5.12
@note

Description:
*/

/*=============================================================================
System Includes
=============================================================================*/

#include <ESP8266WiFi.h>

/*=============================================================================
Local Includes
=============================================================================*/

#include "mqtt_client.h"
#include "esp8266_global.h"

/*=============================================================================
Definitions
=============================================================================*/

/* MQTT server configs */
#define MQTT_HOST "149.129.92.172"
#define MQTT_PORT 1883
#define MQTT_ID   "esp8266"
#define MQTT_USER "zhuzhong"
#define MQTT_PWD  "159357258"

/*=============================================================================
Static Variables
=============================================================================*/

/*=============================================================================
Global Variables
=============================================================================*/

EspMQTTClient mqtt_client(
  MQTT_HOST,    // MQTT Broker server ip
  MQTT_PORT,    // The MQTT port, default to 1883. this line can be omitted
  MQTT_USER,    // Can be omitted if not needed
  MQTT_PWD,     // Can be omitted if not needed
  MQTT_ID       // Client name that uniquely identify your device
);

/*=============================================================================
Static Prototypes
=============================================================================*/

static void mqtt_subscribe_callback(const String &topicStr, const String &message);

/*=============================================================================
Function Definitions
=============================================================================*/

/*===========================================================================*/

/* This function is called once everything is connected (Wifi and MQTT)
   WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient */
void onConnectionEstablished()
{
#if 0
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("mytopic/test", [](const String & payload) {
    Serial.println(payload);
  });

  // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
  client.subscribe("mytopic/wildcardtest/#", [](const String & topic, const String & payload) {
    Serial.println("(From wildcard) topic: " + topic + ", payload: " + payload);
  });

  // Publish a message to "mytopic/test"
  client.publish("mytopic/test", "This is a message"); // You can activate the retain flag by setting the third parameter to true

  // Execute delayed instructions
  client.executeDelayed(5 * 1000, []() {
    client.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later");
  });
#endif

  /* Subscribe the topics, this operation must after connected */
  mqtt_client.subscribe("relay_status", mqtt_subscribe_callback);
  mqtt_client.subscribe("relay_timing_on_enable", mqtt_subscribe_callback);
  mqtt_client.subscribe("relay_timing_on_time", mqtt_subscribe_callback);
  mqtt_client.subscribe("relay_timing_off_enable", mqtt_subscribe_callback);
  mqtt_client.subscribe("relay_timing_off_time", mqtt_subscribe_callback);
  mqtt_client.subscribe("auto_control_relay", mqtt_subscribe_callback);
  mqtt_client.subscribe("high_distance", mqtt_subscribe_callback);
  mqtt_client.subscribe("low_distance", mqtt_subscribe_callback);

  /* Publish must done once here, othrewise publish in loop will block */
  mqtt_publish("relay_status", My_Status.relay_status?"on":"off");

  LOG( DBG_W, "MQTT broker connected.\n" );
}

/*===========================================================================*/

/* MQTT client initialise */
void mqtt_client_init(void)
{
  /* Optional functionalities of EspMQTTClient */
//  mqtt_client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
//  mqtt_client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
//  mqtt_client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
//  mqtt_client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true

  LOG( DBG_P, "MQTT client Initialise Complete.\n" );
}

/*===========================================================================*/

/* Check the MQTT connecttion before publish */
bool mqtt_publish(const String &topic, const String &payload)
{
  bool ret;

  ret = mqtt_client.publish( topic.c_str(), payload.c_str() );

  LOG( DBG_N, "MQTT: Pub, topic(%s), message(%s)\n", topic.c_str(), payload.c_str() );

  return ret;
}

/*===========================================================================*/

/* Main loop, to call at each sketch loop() */
void mqtt_handle_client(void)
{
  mqtt_client.loop();
}

/*===========================================================================*/

/* MQTT callback function for all subscribed topics */
void mqtt_subscribe_callback(const String &topicStr, const String &message)
{
  MY_CONFIG_RECORD  Config;
  FLOAT             Time_hour;
  FLOAT             Distance_cm;

  /* Copy the global config to local config.
     Compare in the end of function, if config is modified, save the config */
  memcpy( &Config, &My_Config, sizeof(MY_CONFIG_RECORD) );

  /*---------------------------------------------------------------------------*/

  /* Topic: 'test' */
  if ( topicStr == "test" )
  {
    /* Do nothing, only leave for test */
  }

  /*---------------------------------------------------------------------------*/

  /* Topic: 'relay_status' */
  if ( topicStr == "relay_status" )
  {
    if ( message == "on" )
    {
      My_Status.relay_status = TRUE;
    }
    else
    {
      My_Status.relay_status = FALSE;
    }
  }

  /*---------------------------------------------------------------------------*/

  /* Topic: 'relay_timing_on_enable' and 'relay_timing_on_time' */
  if ( topicStr == "relay_timing_on_enable" )
  {
    if ( message == "true" )
    {
      Config.relay_on_timing.valid = TRUE;
    }
    else
    {
      Config.relay_on_timing.valid = FALSE;
    }
  }

  if ( topicStr == "relay_timing_on_time" )
  {
    Time_hour = atof(message.c_str());

    /* 24 hours */
    if( (Time_hour >= 0) && (Time_hour < 24) )
    {
      Config.relay_on_timing.hh = (UINT8)Time_hour;
      Config.relay_on_timing.mm = (UINT8)((Time_hour - (FLOAT)Config.relay_on_timing.hh)*60);
    }
  }

  /*---------------------------------------------------------------------------*/

  /* Topic: 'relay_timing_off_enable' and 'relay_timing_off_time' */
  if ( topicStr == "relay_timing_off_enable" )
  {
    if ( message == "true" )
    {
      Config.relay_off_timing.valid = TRUE;
    }
    else
    {
      Config.relay_off_timing.valid = FALSE;
    }
  }

  if ( topicStr == "relay_timing_off_time" )
  {
    Time_hour = atof(message.c_str());

    /* 24 hours */
    if( (Time_hour >= 0) && (Time_hour < 24) )
    {
      Config.relay_off_timing.hh = (UINT8)Time_hour;
      Config.relay_off_timing.mm = (UINT8)((Time_hour - (FLOAT)Config.relay_off_timing.hh)*60);
    }
  }

  /*---------------------------------------------------------------------------*/

  /* Topic: 'auto_control_relay', 'high_distance' and 'low_distance' */
  if ( topicStr == "auto_control_relay" )
  {
    if ( message == "true" )
    {
      Config.relay_auto = TRUE;
    }
    else
    {
      Config.relay_auto = FALSE;
    }
  }

  if ( topicStr == "high_distance" )
  {
    Distance_cm = atof(message.c_str());
    if ( (Distance_cm > 0) && (Distance_cm <= 300) )
    {
      Config.high_distance_cm = Distance_cm;
    }
  }

  if ( topicStr == "low_distance" )
  {
    Distance_cm = atof(message.c_str());
    if ( (Distance_cm > 0) && (Distance_cm <= 300) )
    {
      Config.low_distance_cm = Distance_cm;
    }
  }

  /*---------------------------------------------------------------------------*/

  /* Check if Config was modified. If so, save to EEPROM and update to My_Config */
  if ( memcmp(&Config, &My_Config, sizeof(MY_CONFIG_RECORD)) != 0 )
  {
    if( My_Config_Save( &Config ) == FN_RETURN_OK )
    {
      My_Config = Config;
    }
  }

  LOG( DBG_N, "MQTT: Sub, topic(%s), message(%s)\n", topicStr.c_str(), message.c_str() );
}

