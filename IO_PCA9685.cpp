/*
 *  © 2021, Neil McKechnie. All rights reserved.
 *  
 *  This file is part of DCC++EX API
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

#include "IODevice.h"
#include "I2CManager.h"
#include "DIAG.h"

void PCA9685::create(VPIN firstID, int nPins, uint8_t I2CAddress) {
  addDevice(new PCA9685(firstID, nPins, I2CAddress));
}

// Constructor
PCA9685::PCA9685(VPIN firstID, int nPins, uint8_t I2CAddress) {
  _firstID = firstID;
  _nPins = max(nPins, 16);
  _I2CAddress = I2CAddress;
}

// Device-specific initialisation
void PCA9685::_begin() {
  I2CManager.begin();
  I2CManager.setClock(1000000); // Nominally able to run up to 1MHz on I2C
          // In reality, other devices including the Arduino will limit 
          // the clock speed to a lower rate.

  // Initialise I/O module here.
  writeRegister(PCA9685_MODE1, MODE1_SLEEP | MODE1_AI);    
  writeRegister(PCA9685_PRESCALE, PRESCALE_50HZ);   // 50Hz clock, 20ms pulse period.
  writeRegister(PCA9685_MODE1, MODE1_AI);
  writeRegister(PCA9685_MODE1, MODE1_RESTART | MODE1_AI);    
}

// Device-specific write function.  This device is PWM, and the value written
// can be anything in the range of 0-4095 to get between 0% and 100% mark to period
// ratio.
void PCA9685::_write(VPIN vpin, int value) {
  int pin = vpin-_firstID;
  #ifdef DIAG_IO
  DIAG(F("PCA9685 VPin:%d Write I2C:x%x/%d Value:%d"), (int)vpin, (int)_I2CAddress, pin, value);
  #endif
  uint8_t buffer[] = {(uint8_t)(PCA9685_FIRST_SERVO + 4 * pin), 
      0, 0, (uint8_t)(value & 0xff), (uint8_t)(value >> 8)};
  if (value == 4095) buffer[2] = 0x10;   // Full on
  uint8_t error = I2CManager.write(_I2CAddress, buffer, sizeof(buffer));
  //if (error) DIAG(F("Error I2C:%x, errCode=%d"), _I2CAddress, error);
  (void)error;
}

// Device-specific read function (not supported).
int PCA9685::_read(VPIN vpin) {
  int result = 0;
  #ifdef DIAG_IO
  int pin = vpin - _firstID;
  DIAG(F("PCA9685 VPin:%d Read I2C:x%x/%d Value:%d"), (int)vpin, (int)_I2CAddress, pin, result);
  #else
  (void)vpin;  // suppress compiler warning
  #endif
  return result;
}

void PCA9685::_display() {
  DIAG(F("PCA9685 VPins:%d-%d I2C:x%x"), _I2CAddress, _firstID, _firstID+_nPins-1);
}

// Internal helper function for this device
void PCA9685::writeRegister(byte reg, byte value) {
  I2CManager.write(_I2CAddress, 2, reg, value);
}
