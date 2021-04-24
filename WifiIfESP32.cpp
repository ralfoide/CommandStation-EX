/*
    © 2021, ralf.

    This file is part of CommandStation-EX

    This is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    It is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "config.h"               // force PlatformIO C completion to believe Win32 is defined
#if defined(ESP32)
#include <SPI.h>
#include <WiFi.h>

#include "WifiInterface.h"        /* config.h included there */
#include "DIAG.h"
#include "StringFormatter.h"

#include "WifiInboundHandler.h"



WiFiServer *__wifiServer = NULL;
WiFiClient __wifiClient;

const unsigned long LOOP_TIMEOUT = 2000;
bool WifiInterface::connected = false;
Stream * WifiInterface::wifiStream;

#ifndef WIFI_CONNECT_TIMEOUT
// Tested how long it takes to FAIL an unknown SSID on firmware 1.7.4.
// The ES should fail a connect in 15 seconds, we don't want to fail BEFORE that
// or ot will cause issues with the following commands. 
#define WIFI_CONNECT_TIMEOUT 16000
#endif

////////////////////////////////////////////////////////////////////////////////

bool WifiInterface::setup(long serial_link_speed, 
                          const FSH *wifiESSID,
                          const FSH *wifiPassword,
                          const FSH *hostname,
                          const int port,
                          const byte channel) {
  DIAG(F("@@@ Wifi setup"));

  __wifiServer = NULL;
  connected = false;
  wifiStream = NULL;

  WiFi.begin((const char *)wifiESSID, (const char *)wifiPassword);
  connectToClient();
  return connected; 
}

void WifiInterface::connectToClient() {
  if (__wifiServer == NULL && WiFi.status() == WL_CONNECTED) {
    __wifiServer = new WiFiServer(IP_PORT);
    __wifiServer->begin();
    DIAG(F("@@@ Wifi connected, IP: %s"), WiFi.localIP());
  }

  if (!connected && __wifiServer != NULL) {
    __wifiClient = __wifiServer->available();
    if (__wifiClient.connected()) {
      DIAG(F("@@@ Wifi client connected from %s"), __wifiClient.remoteIP());
      connected = true;
      wifiStream = &__wifiClient;
    }
  }
}

void WifiInterface::loop() {
  if (!connected) {
    connectToClient();
  } 
  
  if (connected) {
    if (__wifiClient.connected()) {
      WifiInboundHandler::loop(); 
    } else {
      // Lost the connection
      wifiStream = NULL;
      connected = false;
      DIAG(F("@@@ Wifi client disconnected"));
    }
  }
}

#endif
