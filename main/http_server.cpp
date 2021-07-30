/*=============================================================================
Copyright Mickey
=============================================================================*/
/*!
@file   http_server.cpp
@brief  handle the request from http client
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

#include "http_server.h"
#include "esp8266_global.h"

/*=============================================================================
Definitions
=============================================================================*/

#define HTML_INDEX ("\
<!DOCTYPE html>\
<html>\
<head><meta charset='utf-8'><title>ESP8266</title></head>\
<body>\
<h1>ESP8266 状态</h1>\
<table border='3'>\
<tr><th>&emsp;&emsp;状态名称&emsp;&emsp;</th><th>&emsp;&emsp;当前状态&emsp;&emsp;</th></tr>\
<tr><td>系统时间:</td><td>{{ localtime_str }}</td></tr>\
<tr><td>Wifi状态:</td><td>{{ wifi_status }}</td></tr>\
<tr><td>Internet状态:</td><td>{{ internet_status }}</td></tr>\
<tr><td>SSID:</td><td>{{ current_ssid }}</td></tr>\
<tr><td>看门狗:</td><td>{{ wdt_status }}</td></tr>\
<tr><td>原始距离(cm):</td><td>{{ original_distance }}</td></tr>\
</table><br><br>\
<form action='wifi' method='get'><input type='submit' value='WIFI配置页面'></form><br>\
<form action='control' method='get'><input type='submit' value='状态和看门狗设置'></form>\
</body>\
</html>\
")

#define HTML_WIFI ("\
<!DOCTYPE html>\
<html>\
<head><meta charset='utf-8'><title>ESP8266 Wifi设置</title></head>\
<body>\
<h1>ESP8266 Wifi设置</h1>\
<form action='/' method='get'><input type='submit' value='主页'></form><br>\
<form action='wifi' method='get'><input type='submit' value='刷新'></form><br>\
<form action='wifi' method='post'>\
WIFI名称: <select name=\"ssid\">\
{{ ssid_datalist }}\
</select><br><br>\
WIFI密码: <input type='password' name=\"pwd\"><br><br>\
<input type='submit' value='提交'>\
</form>\
</body>\
</html>\
")

#define HTML_CONTROL ("\
<!DOCTYPE html>\
<html>\
<head><meta charset='utf-8'><title>ESP8266 设置和状态控制</title></head>\
<body>\
<h1>ESP8266 设置和状态控制</h1>\
<form action='/' method='get'><input type='submit' value='主页'></form><br>\
<form action='control' method='get'><input type='submit' value='刷新'></form><br>\
<form action='control' method='post'>\
<!-- input type='hidden' name='wdt_enable' value='false' --> <!-- 此处为隐藏域，POST时会显示，不能用!!! -->\
看门狗: <input type='checkbox' name='wdt_enable' value='true' {{ wdt_check_status }}>(重启生效)<br><br>\
<!-- input type='hidden' name='relay' value='false' --> <!-- 不能用!!! -->\
继电器: <input type='checkbox' name='relay' value='true'  {{ relay_check_status }}><br><br>\
<input type='submit' value='提交'></form>\
</body>\
</html>\
")


/*=============================================================================
Static Variables
=============================================================================*/

/*=============================================================================
Global Variables
=============================================================================*/

/* Server class, interesting guy */
ESP8266WebServer server(80);

/*=============================================================================
Static Prototypes
=============================================================================*/

/*=============================================================================
Function Definitions
=============================================================================*/

/*===========================================================================*/

void handle_index()
{
  server.send( 200, "text/html", HTML_INDEX );
}

/*===========================================================================*/

void handle_wifi()
{
  String  response_msg;

  String  ssid;
  String  ssid_list;
  int32_t rssi;
  int32_t channel;
  INT8    Scan_Result;

  String  new_ssid;
  String  new_psk;
  UINT32  connect_start_timestamp_ms;

  /*---------------------------------------------------------------------------*/

  switch ( server.method() )
  {
    /* User wants the avaliable SSID list */
    case HTTP_GET:

      LOG( DBG_N, "Starting WiFi scan..." );

      /* Scan wifi first */
      Scan_Result = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);
      if (Scan_Result == 0)
      {
        LOG( DBG_W, "No networks found" );
      }
      else if (Scan_Result > 0)
      {

        Serial.printf(PSTR("%d networks found:\n"), Scan_Result);

        // Print unsorted scan results
        for (int8_t i = 0; i < Scan_Result; i++)
        {
          ssid    = WiFi.SSID(i);
          rssi    = WiFi.RSSI(i);
          channel = WiFi.channel(i);
#if 0
          WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);
#endif
          Serial.printf(PSTR(" %02d: [CH %02d] %ddBm %s\n"),
                        i,
                        channel,
                        rssi,
                        ssid.c_str());

          /* Ignore the empty ssid */
          if ( ssid != "")
          {
            /* Using [\"] to replace ['] on important elements,
               otherwise you may meet some unexpected troubles */
            ssid_list += "<option value=\""+ssid+"\">"+ssid+"</option>\n";
          }
        }
      }
      else
      {
        LOG( DBG_E, "WiFi scan error %d", Scan_Result);
      }

      /* Add the ssid list into html body */
      response_msg += HTML_WIFI;
      response_msg.replace("{{ ssid_datalist }}", ssid_list);

      /* Send the html body back */
      server.send( 200, "text/html", response_msg );

      break;

    /*---------------------------------------------------------------------------*/

    /* User wants to connect to a new SSID */
    case HTTP_POST:

      new_ssid  = server.arg("ssid");
      new_psk   = server.arg("pwd");

      LOG( DBG_I, "New ssid: %s\n", new_ssid.c_str() );
      LOG( DBG_I, "New psk: %s\n", new_psk.c_str() );

      /* Try connect the new SSID first */
      connect_start_timestamp_ms = millis();
      WiFi.begin( new_ssid , new_psk );

      /* Wait for connection */
      while (WiFi.status() != WL_CONNECTED)
      {
        delay(500);
        Serial.print(".");
        /* Wait for 10 sec */
        if ( (millis() - connect_start_timestamp_ms) > 10*1000 )
        {
          LOG( DBG_E, "Connect to %s timeout\n", new_ssid);
          break;
        }
      }

      /* Save the config if connect new SSID successfully */
      if ( WiFi.status() == WL_CONNECTED )
      {
        strcpy( My_Config.sta_ssid,  new_ssid.c_str() );
        strcpy( My_Config.sta_pwd,   new_psk.c_str() );
        My_Config_Save(&My_Config);
      }

      /* Amyway, send index html body back */
      handle_index();

      break;

    /*---------------------------------------------------------------------------*/

    default:

      response_msg += server.method();
      server.send( 200, "text/plain", response_msg );

      LOG( DBG_I, "Unknown request method: %s\n", response_msg );
      break;
  }
}

/*===========================================================================*/

void handle_control()
{
  server.send( 200, "text/html", HTML_CONTROL );
}

/*===========================================================================*/

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

/*===========================================================================*/

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
   connected to this access point to see it.
*/
void http_server_init(void)
{
  Serial.println("");

  /* Register the page to handle fucntions
     Emmm...you can NOT use static for these functions */
  server.on("/", handle_index);
  server.on("/wifi", handle_wifi);
  server.on("/control", handle_control);
  server.onNotFound(handleNotFound);

  /* Start server */
  server.begin();
  Serial.println("HTTP server started");
}

/*===========================================================================*/

void http_handle_client(void)
{
  server.handleClient();
}
