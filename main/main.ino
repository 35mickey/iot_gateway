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

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

/*=============================================================================
Local Includes
=============================================================================*/

#include "esp8266_12f_bsp.h"
#include "http_server.h"

/*=============================================================================
Definitions
=============================================================================*/

#ifndef STASSID
#define STASSID "ZXG_Tech1"
#define STAPSK  "ZXG86968188"
#endif

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK  "thereisnospoon"
#endif

/*=============================================================================
Static Variables
=============================================================================*/

/*=============================================================================
Global Variables
=============================================================================*/

u32 led_flash_timestamp_ms = 0;
u32 led_flash_timestamp_us = 0;

/*=============================================================================
Static Prototypes
=============================================================================*/

/*=============================================================================
Function Definitions
=============================================================================*/

/*===========================================================================*/

void setup() {
  delay(100);
  Serial.begin(115200);
  Serial.println();

  // initialize digital pin led as an output.
  pinMode(GPIO_LED, OUTPUT);

  /* Set wifi mode to support both AP and STA */
  WiFi.mode(WIFI_AP_STA);

/*---------------------------------------------------------------------------*/

  /* Init the wifi AP function */
  Serial.println("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
//  WiFi.softAP(ap_ssid, ap_password);
  WiFi.softAP(APSSID);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

/*---------------------------------------------------------------------------*/

  /* Init the wifi STA function */
  WiFi.begin(STASSID, STAPSK);
  /* Wait for connection */
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(STASSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

/*---------------------------------------------------------------------------*/

  /* Init the HTTP server */
  http_server_init();
}

/*===========================================================================*/

void loop() {

  if ( (millis() - led_flash_timestamp_us ) > 1000 )
  {
    led_flash_timestamp_us = millis();
    int led_state = digitalRead(GPIO_LED);
    digitalWrite(GPIO_LED, !led_state);
  }

  http_handle_client();
}
