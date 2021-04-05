/* Based on Arduino SSD1306Ascii Library, Copyright (C) 2015 by William Greiman
 * Modifications (C) 2021 Neil McKechnie
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SSD1306Ascii Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include "SSD1306Ascii.h"
#include "I2CManager.h"
#include "FSH.h"

//==============================================================================
// SSD1306/SSD1106 I2C command bytes
//------------------------------------------------------------------------------
/** Set Lower Column Start Address for Page Addressing Mode. */
static const uint8_t SSD1306_SETLOWCOLUMN = 0x00;
/** Set Higher Column Start Address for Page Addressing Mode. */
static const uint8_t SSD1306_SETHIGHCOLUMN = 0x10;
/** Set Memory Addressing Mode. */
static const uint8_t SSD1306_MEMORYMODE = 0x20;
/** Set display RAM display start line register from 0 - 63. */
static const uint8_t SSD1306_SETSTARTLINE = 0x40;
/** Set Display Contrast to one of 256 steps. */
static const uint8_t SSD1306_SETCONTRAST = 0x81;
/** Enable or disable charge pump.  Follow with 0X14 enable, 0X10 disable. */
static const uint8_t SSD1306_CHARGEPUMP = 0x8D;
/** Set Segment Re-map between data column and the segment driver. */
static const uint8_t SSD1306_SEGREMAP = 0xA0;
/** Resume display from GRAM content. */
static const uint8_t SSD1306_DISPLAYALLON_RESUME = 0xA4;
/** Force display on regardless of GRAM content. */
static const uint8_t SSD1306_DISPLAYALLON = 0xA5;
/** Set Normal Display. */
static const uint8_t SSD1306_NORMALDISPLAY = 0xA6;
/** Set Inverse Display. */
static const uint8_t SSD1306_INVERTDISPLAY = 0xA7;
/** Set Multiplex Ratio from 16 to 63. */
static const uint8_t SSD1306_SETMULTIPLEX = 0xA8;
/** Set Display off. */
static const uint8_t SSD1306_DISPLAYOFF = 0xAE;
/** Set Display on. */
static const uint8_t SSD1306_DISPLAYON = 0xAF;
/**Set GDDRAM Page Start Address. */
static const uint8_t SSD1306_SETSTARTPAGE = 0xB0;
/** Set COM output scan direction normal. */
static const uint8_t SSD1306_COMSCANINC = 0xC0;
/** Set COM output scan direction reversed. */
static const uint8_t SSD1306_COMSCANDEC = 0xC8;
/** Set Display Offset. */
static const uint8_t SSD1306_SETDISPLAYOFFSET = 0xD3;
/** Sets COM signals pin configuration to match the OLED panel layout. */
static const uint8_t SSD1306_SETCOMPINS = 0xDA;
/** This command adjusts the VCOMH regulator output. */
static const uint8_t SSD1306_SETVCOMDETECT = 0xDB;
/** Set Display Clock Divide Ratio/ Oscillator Frequency. */
static const uint8_t SSD1306_SETDISPLAYCLOCKDIV = 0xD5;
/** Set Pre-charge Period */
static const uint8_t SSD1306_SETPRECHARGE = 0xD9;
/** Deactivate scroll */
static const uint8_t SSD1306_DEACTIVATE_SCROLL = 0x2E;
/** No Operation Command. */
static const uint8_t SSD1306_NOP = 0xE3;
//------------------------------------------------------------------------------
/** Set Pump voltage value: (30H~33H) 6.4, 7.4, 8.0 (POR), 9.0. */
static const uint8_t SH1106_SET_PUMP_VOLTAGE = 0x30;
/** First byte of set charge pump mode */
static const uint8_t SH1106_SET_PUMP_MODE = 0xAD;
/** Second byte charge pump on. */
static const uint8_t SH1106_PUMP_ON = 0x8B;
/** Second byte charge pump off. */
static const uint8_t SH1106_PUMP_OFF = 0x8A;
//------------------------------------------------------------------------------


// Maximum number of bytes we can send per transmission is 32.
const uint8_t FLASH SSD1306AsciiWire::blankPixels[32] = 
  {0x40,        // First byte specifies data mode
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  

//==============================================================================
// SSD1306AsciiWire Method Definitions
//------------------------------------------------------------------------------
void SSD1306AsciiWire::clear() {
  clear(0, displayWidth() - 1, 0, displayRows() - 1);
}
//------------------------------------------------------------------------------
void SSD1306AsciiWire::clear(uint8_t columnStart, uint8_t columnEnd, 
                             uint8_t rowStart, uint8_t rowEnd) {
  const int maxBytes = sizeof(blankPixels);  // max number of bytes sendable over Wire
  // Ensure only rows on display will be cleared.
  if (rowEnd >= displayRows()) rowEnd = displayRows() - 1;
  for (uint8_t r = rowStart; r <= rowEnd; r++) {
    setCursor(columnStart, r);   // Position at start of row to be erased
    for (uint8_t c = columnStart; c <= columnEnd; c += maxBytes-1) {
      uint8_t len = min((uint8_t)(columnEnd-c+1), maxBytes-1) + 1;
      I2CManager.write_P(m_i2cAddr, blankPixels, len);  // Write up to 31 blank columns
    }
  }
}
//------------------------------------------------------------------------------
void SSD1306AsciiWire::begin(const DevType* dev, uint8_t i2cAddr) {
  m_i2cAddr = i2cAddr;
  m_col = 0;
  m_row = 0;
#ifdef __AVR__
  const uint8_t* table = (const uint8_t*)pgm_read_word(&dev->initcmds);
#else   // __AVR__
  const uint8_t* table = dev->initcmds;
#endif  // __AVR
  uint8_t size = GETFLASH(&dev->initSize);
  m_displayWidth = GETFLASH(&dev->lcdWidth);
  m_displayHeight = GETFLASH(&dev->lcdHeight);
  m_colOffset = GETFLASH(&dev->colOffset);
  I2CManager.write_P(m_i2cAddr, table, size);
  if (m_displayHeight == 64) 
    I2CManager.write(m_i2cAddr, 5, 0, // Set command mode
      SSD1306_SETMULTIPLEX, 0x3F,     // ratio 64
      SSD1306_SETCOMPINS, 0x02);      // sequential COM pins, disable remap
  else 
    I2CManager.write(m_i2cAddr, 5, 0, // Set command mode
      SSD1306_SETMULTIPLEX, 0x1F,     // ratio 32
      SSD1306_SETCOMPINS, 0x02);      // sequential COM pins, disable remap
}
//------------------------------------------------------------------------------
void SSD1306AsciiWire::setContrast(uint8_t value) {
  I2CManager.write(m_i2cAddr, 2, 
    0x00,     // Set to command mode
    SSD1306_SETCONTRAST, value);
}
//------------------------------------------------------------------------------
void SSD1306AsciiWire::setCursor(uint8_t col, uint8_t row) {
  if (row < displayRows() && col < m_displayWidth) {
    m_row = row;
    m_col = col + m_colOffset;
    I2CManager.write(m_i2cAddr, 4,
      0x00,    // Set to command mode
      SSD1306_SETLOWCOLUMN | (col & 0XF), 
      SSD1306_SETHIGHCOLUMN | (col >> 4),
      SSD1306_SETSTARTPAGE | m_row);
  }
}
//------------------------------------------------------------------------------
size_t SSD1306AsciiWire::write(uint8_t ch) {
  const uint8_t* base = m_font;

  if (ch < m_fontFirstChar || ch >= (m_fontFirstChar + m_fontCharCount))
    return 0;
#if defined(NOLOWERCASE)
  // Adjust if lowercase is missing
  if (ch >= 'a') {
    if (ch <= 'z')
      ch = ch - 'a' + 'A';  // Capitalise
    else
      ch -= 26; // Allow for missing lowercase letters
  }
#endif
  ch -= m_fontFirstChar;
  base += fontWidth * ch;
  uint8_t buffer[1+fontWidth+letterSpacing];
  buffer[0] = 0x40;     // set SSD1306 controller to data mode
  uint8_t bufferPos = 1;
  // Copy character pixel columns
  for (uint8_t i = 0; i < fontWidth; i++) 
    buffer[bufferPos++] = GETFLASH(base++);
  // Add blank pixels between letters
  for (uint8_t i = 0; i < letterSpacing; i++) 
    buffer[bufferPos++] = 0;
  // Write the data to I2C display
  I2CManager.write(m_i2cAddr, buffer, bufferPos);
  return 1;
}

//==============================================================================
// this section is based on https://github.com/adafruit/Adafruit_SSD1306
/** Initialization commands for a 128x32 or 128x64 SSD1306 oled display. */
const uint8_t FLASH SSD1306AsciiWire::Adafruit128xXXinit[] = {
    // Init sequence for Adafruit 128x32/64 OLED module
    0x00,                              // Set to command mode
    SSD1306_DISPLAYOFF,
    SSD1306_SETDISPLAYCLOCKDIV, 0x80,  // the suggested ratio 0x80 
    SSD1306_SETDISPLAYOFFSET, 0x0,     // no offset
    SSD1306_SETSTARTLINE | 0x0,        // line #0
    SSD1306_CHARGEPUMP, 0x14,          // internal vcc
    SSD1306_MEMORYMODE, 0x02,          // page mode
    SSD1306_SEGREMAP | 0x1,            // column 127 mapped to SEG0
    SSD1306_COMSCANDEC,                // column scan direction reversed
    SSD1306_SETCONTRAST, 0x7F,         // contrast level 127
    SSD1306_SETPRECHARGE, 0xF1,        // pre-charge period (1, 15)
    SSD1306_SETVCOMDETECT, 0x40,       // vcomh regulator level
    SSD1306_DISPLAYALLON_RESUME,
    SSD1306_NORMALDISPLAY,
    SSD1306_DISPLAYON
};
/** Initialize a 128x32 SSD1306 oled display. */
const DevType FLASH SSD1306AsciiWire::Adafruit128x32 = {
  Adafruit128xXXinit,
  sizeof(Adafruit128xXXinit),
  128,
  32,
  0
};
/** Initialize a 128x64 oled display. */
const DevType FLASH SSD1306AsciiWire::Adafruit128x64 = {
  Adafruit128xXXinit,
  sizeof(Adafruit128xXXinit),
  128,
  64,
  0
};
//------------------------------------------------------------------------------
// This section is based on https://github.com/stanleyhuangyc/MultiLCD
/** Initialization commands for a 128x64 SH1106 oled display. */
const uint8_t FLASH SSD1306AsciiWire::SH1106_128x64init[] = {
  0x00,                                  // Set to command mode
  SSD1306_DISPLAYOFF,
  SSD1306_SETSTARTPAGE | 0X0,            // set page address
  SSD1306_SETCONTRAST, 0x80,             // 128
  SSD1306_SEGREMAP | 0X1,                // set segment remap
  SSD1306_NORMALDISPLAY,                 // normal / reverse
  SH1106_SET_PUMP_MODE, SH1106_PUMP_ON,  // set charge pump enable **
  SH1106_SET_PUMP_VOLTAGE | 0X2,         // 8.0 volts              **
  SSD1306_COMSCANDEC,                    // Com scan direction
  SSD1306_SETDISPLAYOFFSET, 0X00,        // set display offset
  SSD1306_SETDISPLAYCLOCKDIV, 0X80,      // set osc division
  SSD1306_SETPRECHARGE, 0X1F,            // set pre-charge period  **
  SSD1306_SETVCOMDETECT,  0x40,          // set vcomh
  SSD1306_DISPLAYON
};
/** Initialize a 128x64 oled SH1106 display. */
const DevType FLASH SSD1306AsciiWire::SH1106_128x64 =  {
  SH1106_128x64init,
  sizeof(SH1106_128x64init),
  128,
  64,
  2    // SH1106 is a 132x64 controller.  Use middle 128 columns.
};
