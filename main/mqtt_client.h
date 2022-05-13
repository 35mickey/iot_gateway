/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   mqtt_client.h
@brief  MQTT definitions and configurations
@author Mickey
@date   2022.5.12
@note

Description:
*/

#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

/*=============================================================================
System Includes
=============================================================================*/

#include <EspMQTTClient.h>

/*=============================================================================
Local Includes
=============================================================================*/

/*=============================================================================
Definitions
=============================================================================*/

#if 0
# MQTT Subscribe Topics
mqtt_sub_topics = [
    "relay_timing_on_enable",
    "relay_timing_on_time",
    "relay_timing_off_enable",
    "relay_timing_off_time",
    "relay_status"
    "auto_control_relay",
    "high_distance",
    "low_distance"
]

# MQTT Publish Topics
mqtt_pub_topics = [
    "relay_timing_on_enable",
    "relay_timing_on_time",
    "relay_timing_off_enable",
    "relay_timing_off_time",
    "relay_status",
    "original_distance",
    "average_distance",
    "auto_control_relay",
    "high_distance",
    "low_distance"
]
#endif

/*=============================================================================
Global References
=============================================================================*/

extern EspMQTTClient mqtt_client;

/*=============================================================================
Prototypes
=============================================================================*/

void mqtt_client_init(void);
bool mqtt_publish(const String &topic, const String &payload);
void mqtt_handle_client(void);

#endif  /* __MQTT_CLIENT_H__ */

/*===========================================================================*/

