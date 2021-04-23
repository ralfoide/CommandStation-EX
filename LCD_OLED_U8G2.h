/*
 *  © 2021, Chris Harlow, Neil McKechnie. All rights reserved.
 *
 *  This file is part of CommandStation-EX
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
 */

// OLED Implementation of LCDDisplay class based on the U8G2 library for ESP32.
// Note: this file is optionally included by LCD_Implementation.h
// It is NOT a .cpp file to prevent it being compiled and demanding libraries
// even when not needed.

#include <U8g2lib.h>
// Customize I2C pin below. These are for the Heltec Wifi Kit ESP32 dev board.
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

#define U8G2_PIX_W 128
#define U8G2_PIX_H 64

// Small font
// #define U8G2_FONT u8g2_font_5x7_tf
// #define U8G2_SX 6
// #define U8G2_SY 8

// Large font
#define U8G2_FONT u8g2_font_6x10_tf
#define U8G2_SX 7
#define U8G2_SY 10

// DEVICE SPECIFIC LCDDisplay Implementation for OLED

LCDDisplay::LCDDisplay() {
  // Scan for device on 0x3c and 0x3d.
  DIAG(F("OLED using U8G2 lib"));
  u8g2.begin();
  u8g2.setFont(U8G2_FONT);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  clear();
  lcdDisplay = this;
}

void LCDDisplay::interfake(int p1, int p2, int p3) {
  lcdCols = p1;
  lcdRows = p2 / 8;
  (void)p3;
}

void LCDDisplay::clearNative() { 
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

static u8g2_uint_t __u8g2_y = 0;
static u8g2_uint_t __u8g2_x = 0;
void LCDDisplay::setRowNative(byte row) {
  __u8g2_y = row * U8G2_SY;
  __u8g2_x = 0;
}

void LCDDisplay::writeNative(char b) { 
  u8g2.drawGlyph(__u8g2_x, __u8g2_y, (uint16_t) b);
  u8g2.sendBuffer();
  __u8g2_x += U8G2_SX;
  if (__u8g2_x >= U8G2_PIX_W) {
    __u8g2_x = 0;
    __u8g2_y += U8G2_SY;
    if (__u8g2_y >= U8G2_PIX_H) {
      __u8g2_y = 0;
    }
  }
}

void LCDDisplay::displayNative() {
  // no-op, not used.
}
