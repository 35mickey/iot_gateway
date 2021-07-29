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
WIFI名称: <select name='ssid'>\
{{ ssid_datalist }}\
</select><br><br>\
WIFI密码: <input type='password' name='pwd'><br><br>\
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
  server.send( 200, "text/html", HTML_WIFI );
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
  Serial.println("New HTTP server started");
}

/*===========================================================================*/

void http_handle_client(void)
{
  server.handleClient();
}

