/*
 *  © 2020,Gregor Baues,  Chris Harlow. All rights reserved.
 *  © 2021,ralfoide
 *  
 *  This file is part of DCC-EX/CommandStation-EX
 *
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
 *  Ethernet Interface added by Gregor Baues
 */

#ifndef WifiIfESP32_h
#define WifiIfESP32_h
#if defined(ESP32) // RM 2021-04-22
#if __has_include ( "config.h")
  #include "config.h"
#else
  #warning config.h not found. Using defaults from config.example.h 
  #include "config.example.h"
#endif

#include <SPI.h>
#include <WiFi.h>

#include "DCCEXParser.h"
#include "RingStream.h"

#define MAX_SOCK_NUM 4
#define MAX_ETH_BUFFER 512
#define OUTBOUND_RING_SIZE 2048

class WifiIfESP32 {
 public:    
    static void setup(const FSH *wifiESSID, const FSH *wifiPassword);
    static void loop();
   
 private:
    static WifiIfESP32 *singleton;

    WifiIfESP32();
    void begin(const FSH *wifiESSID, const FSH *wifiPassword);
    void connectToClient();
    void loop2();
    bool connected;
    WiFiServer *server;
    WiFiClient clients[MAX_SOCK_NUM];                // accept up to MAX_SOCK_NUM client connections at the same time; This depends on the chipset used on the Shield
    uint8_t buffer[MAX_ETH_BUFFER+1];                    // buffer used by TCP for the recv
    RingStream * outboundRing;
  
};

#endif
#endif
