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
 * along with this software.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef SSD1306Ascii_h
#define SSD1306Ascii_h

#include "Arduino.h"
#include "FSH.h"

// Uncomment to remove lower-case letters to save 108 bytes of flash
//#define NOLOWERCASE

//------------------------------------------------------------------------------
/**
 * @struct DevType
 * @brief Device initialization structure.
 */
struct DevType {
  /**
   * Pointer to initialization command bytes.
   */
  const uint8_t* initcmds;
  /**
   * Number of initialization bytes.
   */
  const uint8_t initSize;
  /**
   * Width of the diaplay in pixels.
   */
  const uint8_t lcdWidth;
  /**
   * Height of the display in pixels.
   */
  const uint8_t lcdHeight;
  /**
   * Column offset RAM to display.  Used to pick start column of SH1106.
   */
  const uint8_t colOffset;
};


class SSD1306AsciiWire : public Print {
 public:
  using Print::write;
  SSD1306AsciiWire() {}
  // Initialize the display controller.
  void begin(const DevType* dev, uint8_t i2cAddr);
  // Clear the display and set the cursor to (0, 0).
  void clear();
  // Clear a region of the display.
  void clear(uint8_t c0, uint8_t c1, uint8_t r0, uint8_t r1);
  // The current column in pixels.
  inline uint8_t col() const { return m_col; }
  // The display hight in pixels.
  inline uint8_t displayHeight() const { return m_displayHeight; }
  // The display height in rows with eight pixels to a row.
  inline uint8_t displayRows() const { return m_displayHeight / 8; }
  // The display width in pixels.
  inline uint8_t displayWidth() const { return m_displayWidth; }
  // Set the cursor position to (0, 0).
  inline void home() { setCursor(0, 0); }
  // Initialize the display controller.
  void init(const DevType* dev);
  // the current row number with eight pixels to a row.
  inline uint8_t row() const { return m_row; }
  /**
   * @brief Set the display contrast.
   *
   * @param[in] value The contrast level in th range 0 to 255.
   */
  void setContrast(uint8_t value);
  /**
   * @brief Set the cursor position.
   *
   * @param[in] col The column number in pixels.
   * @param[in] row the row number in eight pixel rows.
   */
  void setCursor(uint8_t col, uint8_t row);
  /**
   * @brief Set the current font.
   *
   * @param[in] font Pointer to a font table.
   */
  void setFont(const uint8_t* font);
  /**
   * @brief Display a character.
   *
   * @param[in] c The character to display.
   * @return one for success else zero.
   */
  size_t write(uint8_t c);

  // Display characteristics / initialisation
  static const DevType FLASH Adafruit128x32;
  static const DevType FLASH Adafruit128x64;
  static const DevType FLASH SH1106_128x64;

 private:
  uint8_t m_col;                    // Cursor column.
  uint8_t m_row;                    // Cursor RAM row.
  uint8_t m_displayWidth;           // Display width.
  uint8_t m_displayHeight;          // Display height.
  uint8_t m_colOffset = 0;          // Column offset RAM to SEG.
  const uint8_t* const m_font = System5x7;  // Current font.

  // Only fixed size 5x7 fonts in a 6x8 cell are supported.
  const uint8_t fontWidth = 5;
  const uint8_t fontHeight = 7;
  const uint8_t letterSpacing = 1;
  const uint8_t m_fontFirstChar = 0x20;
  const uint8_t m_fontCharCount = 0x61;

  uint8_t m_i2cAddr;

  static const uint8_t blankPixels[];

  static const uint8_t System5x7[];
  static const uint8_t FLASH Adafruit128xXXinit[];
  static const uint8_t FLASH SH1106_128x64init[];
};

#endif  // SSD1306Ascii_h
