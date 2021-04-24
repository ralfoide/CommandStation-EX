// /*
//     © 2021, ralf.

//     This file is part of CommandStation-EX

//     This is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     It is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
// */
// #include "config.h"               // force PlatformIO C completion to believe Win32 is defined
// #if defined(ESP32)
// #include <SPI.h>
// #include <WiFi.h>

// #include "WifiInterface.h"        /* config.h included there */
// #include "DIAG.h"
// #include "StringFormatter.h"

// #include "WifiInboundHandler.h"



// WiFiServer *__wifiServer = NULL;
// WiFiClient __wifiClient;

// const unsigned long LOOP_TIMEOUT = 2000;
// bool WifiInterface::connected = false;
// Stream * WifiInterface::wifiStream;

// #ifndef WIFI_CONNECT_TIMEOUT
// // Tested how long it takes to FAIL an unknown SSID on firmware 1.7.4.
// // The ES should fail a connect in 15 seconds, we don't want to fail BEFORE that
// // or ot will cause issues with the following commands. 
// #define WIFI_CONNECT_TIMEOUT 16000
// #endif

// ////////////////////////////////////////////////////////////////////////////////

// bool WifiInterface::setup(long serial_link_speed, 
//                           const FSH *wifiESSID,
//                           const FSH *wifiPassword,
//                           const FSH *hostname,
//                           const int port,
//                           const byte channel) {
//   DIAG(F("@@@ Wifi setup"));

//   __wifiServer = NULL;
//   connected = false;
//   wifiStream = NULL;

//   WiFi.begin((const char *)wifiESSID, (const char *)wifiPassword);
//   connectToClient();
//   return connected; 
// }

// void WifiInterface::connectToClient() {
//   if (__wifiServer == NULL && WiFi.status() == WL_CONNECTED) {
//     __wifiServer = new WiFiServer(IP_PORT);
//     __wifiServer->begin();
//     DIAG(F("@@@ Wifi connected, IP %s, port %d"), WiFi.localIP().toString().c_str(), IP_PORT);
//   }

//   if (!connected && __wifiServer != NULL) {
//     __wifiClient = __wifiServer->available();
//     if (__wifiClient.connected()) {
//       DIAG(F("@@@ Wifi client connected from %s"), __wifiClient.remoteIP().toString().c_str());
//       connected = true;
//       wifiStream = &__wifiClient;
//       WifiInboundHandler::setup(wifiStream);
//       DIAG(F("@@@ wifiStream = %p"), wifiStream);
//     }
//   }
// }

// void WifiInterface::loop() {
//   if (!connected) {
//     connectToClient();
//   } 
  
//   if (connected) {
//     if (__wifiClient.connected()) {
//       WifiInboundHandler::loop(); 
//     } else {
//       // Lost the connection
//       wifiStream = NULL;
//       connected = false;
//       DIAG(F("@@@ Wifi client disconnected"));
//     }
//   }
// }

// #endif


/*
 *  © 2020,Gregor Baues,  Chris Harlow. All rights reserved.
 *  
 *  This file is part of DCC-EX/CommandStation-EX
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */
#if __has_include ( "config.h")
  #include "config.h"
#else
  #warning config.h not found. Using defaults from config.example.h 
  #include "config.example.h"
#endif
#include "defines.h" 
#if defined(ESP32)
#include "WifiIfESP32.h"
#include "DIAG.h"
#include "CommandDistributor.h"
#include "DCCTimer.h"

WifiIfESP32 * WifiIfESP32::singleton = NULL;

void WifiIfESP32::setup(const FSH *wifiESSID, const FSH *wifiPassword) {
    DIAG(F("@@@ Wifi ESP32 setup"));
    singleton = new WifiIfESP32();
    singleton->begin(wifiESSID, wifiPassword);
};


WifiIfESP32::WifiIfESP32() {
  server = NULL;
  connected = false;
}

void WifiIfESP32::begin(const FSH *wifiESSID, const FSH *wifiPassword) {
  DIAG(F("@@@ Wifi ESP32 begin with SSID %s"), (const char *)wifiESSID);
  WiFi.begin((const char *)wifiESSID, (const char *)wifiPassword);
  connectToClient();
}

static int __lastStatus = -1;
void WifiIfESP32::connectToClient() {
  if (WiFi.status() != __lastStatus) {
    __lastStatus = (int) WiFi.status();
    DIAG(F("@@@ Wifi status: %d"), __lastStatus);
  }
  if (server == NULL && WiFi.status() == WL_CONNECTED) {
    server = new WiFiServer(IP_PORT);
    server->begin();
    IPAddress ip = WiFi.localIP();
    DIAG(F("@@@ Wifi connected, IP %s, port %d"), ip.toString().c_str(), IP_PORT);
    LCD(4,F("IP: %s"), ip.toString().c_str());
    LCD(5,F("Port:%d"), IP_PORT);

    outboundRing = new RingStream(OUTBOUND_RING_SIZE);
    connected = true;
    DIAG(F("@@@ Wifi connected"));
  }
}

void WifiIfESP32::loop() {
  if (singleton != NULL) {
    if (singleton->server != NULL) {
      singleton->loop2();
    } else {
      singleton->connectToClient();
    }
  }
}

void WifiIfESP32::loop2() {
    // get client from the server
    WiFiClient client = server->accept();

    // check for new client
    if (client) {
        if (Diag::WIFI) DIAG(F("WiFi: New client from %s"), client.remoteIP().toString().c_str());
        byte socket;
        for (socket = 0; socket < MAX_SOCK_NUM; socket++) {
            if (!clients[socket]) {
                // On accept() the WiFiServer doesn't track the client anymore
                // so we store it in our client array
                if (Diag::WIFI) DIAG(F("Socket %d"),socket);
                clients[socket] = client;
                break;
            }
        }
        if (socket == MAX_SOCK_NUM) DIAG(F("WiFi: New Client OVERFLOW"));
    }

    // check for incoming data from all possible clients
    for (byte socket = 0; socket < MAX_SOCK_NUM; socket++) {
        if (clients[socket]) {
          int available = clients[socket].available();
          if (available > 0) {
            if (Diag::WIFI)  DIAG(F("WiFi: available socket=%d,avail=%d"), socket, available);
            // read bytes from a client
            int count = clients[socket].read(buffer, MAX_ETH_BUFFER);
            buffer[count] = '\0'; // terminate the string properly
            if (Diag::WIFI) DIAG(F(",count=%d:%e"), socket,buffer);
            // execute with data going directly back
            outboundRing->mark(socket); 
            CommandDistributor::parse(socket,buffer,outboundRing);
            outboundRing->commit();
            return; // limit the amount of processing that takes place within 1 loop() cycle. 
          }
        }
    }

    // stop any clients which disconnects
   for (int socket = 0; socket < MAX_SOCK_NUM; socket++) {
     if (clients[socket] && !clients[socket].connected()) {
      clients[socket].stop();
      if (Diag::WIFI) DIAG(F("WiFi: disconnect %d"), socket);             
     }
    }
    
    // handle at most 1 outbound transmission 
    int socketOut = outboundRing->read();
    if (socketOut >= 0) {
      int count=outboundRing->count();
      if (Diag::WIFI) DIAG(F("WiFi reply socket=%d, count=:%d"), socketOut,count);
      for (;count>0;count--) clients[socketOut].write(outboundRing->read());
      clients[socketOut].flush(); //maybe 
    }
}
#endif
