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

// OLED Implementation of LCDDisplay class
// Note: this file is optionally included by LCD_Implementation.h
// It is NOT a .cpp file to prevent it being compiled and demanding libraries
// even when not needed.

#include "I2CManager.h"
#include "SSD1306Ascii.h"
SSD1306AsciiWire LCDDriver;

// DEVICE SPECIFIC LCDDisplay Implementation for OLED
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

LCDDisplay::LCDDisplay() {
  // Scan for device on 0x3c and 0x3d.
  I2CManager.begin();
  I2CManager.setClock(400000L);  // Set max supported I2C speed
  for (byte address = 0x3c; address <= 0x3d; address++) {
    if (I2CManager.exists(address)) {
      // Device found
      DIAG(F("OLED display found at 0x%x"), address);
      interfake(OLED_DRIVER, 0);
      const DevType *devType;
      if (lcdCols == 132)
        devType = &SH1106_128x64;  // Actually 132x64 but treated as 128x64
      else if (lcdCols == 128 && lcdRows == 4)
        devType = &Adafruit128x32;
      else
        devType = &Adafruit128x64;
      LCDDriver.begin(devType, address);
      lcdDisplay = this;
      LCDDriver.setFont(System5x7);  // Normal 1:1 pixel scale, 8 bits high
      clear();
      return;
    }
  }
  DIAG(F("OLED display not found"));

  DIAG(F("OLED using U8G2 lib"));
  u8g2.begin();
  u8g2.setFont(u8g2_font_5x7_tf);
  //u8g2.setFont(u8g2_font_6x10_tf);
#define U8G2_SX 6
#define U8G2_SY 8
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
  clear();
  //u8g2.clearBuffer();
  //u8g2.drawStr(0, 0, "OLED Init");
  //u8g2.sendBuffer();
  lcdDisplay = this;
}

void LCDDisplay::interfake(int p1, int p2, int p3) {
  lcdCols = p1;
  lcdRows = p2 / 8;
  (void)p3;
}

void LCDDisplay::clearNative() { 
  //-- LCDDriver.clear(); 
  u8g2.clearBuffer();
  u8g2.sendBuffer();
}

static u8g2_uint_t __u8g2_y = 0;
static u8g2_uint_t __u8g2_x = 0;
// static char __u8g2_str[2] = "\0";
void LCDDisplay::setRowNative(byte row) {
  // Positions text write to start of row 1..n
  int y = row;
  //-- LCDDriver.setCursor(0, y);
  __u8g2_y = row * U8G2_SY;
  __u8g2_x = 0;
}

void LCDDisplay::writeNative(char b) { 
  //-- LCDDriver.write(b); 
  // __u8g2_str[0] = b;
  // __u8g2_str[1] = 0;
  u8g2.drawGlyph(__u8g2_x, __u8g2_y, (uint16_t) b);
  // u8g2.drawStr(__u8g2_x, __u8g2_y, __u8g2_str);
  u8g2.sendBuffer();
  __u8g2_x += U8G2_SX;
  if (__u8g2_x >= 128) {
    __u8g2_x = 0;
    __u8g2_y += U8G2_SY;
    if (__u8g2_y >= 64) {
      __u8g2_y = 0;
    }
  }
}

void LCDDisplay::displayNative() {

}
