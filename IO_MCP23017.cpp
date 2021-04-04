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

void MCP23017::create(VPIN firstID, int nPins, uint8_t I2CAddress) {
  MCP23017 *dev = new MCP23017();
  dev->_firstID = firstID;
  dev->_nPins = max(nPins, 16);
  dev->_I2CAddress = I2CAddress;

  addDevice(dev);
}
  
// Constructor
MCP23017::MCP23017() {}

// Device-specific initialisation
void MCP23017::_begin() { 
  I2CManager.begin();
  I2CManager.setClock(1000000);
}
  
// Device-specific write function.
// TBD: Register setup for output mode
void MCP23017::_write(VPIN id, int value) {
  VPIN pin = id-_firstID;
  uint8_t port = (pin < 8) ? GPIOA : GPIOB;
  #ifdef DIAG_IO
  //DIAG(F("MCP23017 Write Addr:x%x Pin:%d Value:%d"), (int)_I2CAddress, (int)pin, value);
  #endif
  uint8_t mask = 1 << (pin % 8);
  if (value) 
    _currentPortState |= mask;
  else
    _currentPortState &= ~mask;

  I2CManager.write(_I2CAddress, 2, port, _currentPortState);
}

// Device-specific read function.
// TBD: Register setup for input mode
int MCP23017::_read(VPIN id) {
  int result;
  uint8_t buffer;
  VPIN pin = id-_firstID;
  uint8_t port = (pin < 8) ? GPIOA : GPIOB;
  I2CManager.read(_I2CAddress, &buffer, 1, 1, port);
  uint8_t mask = 1 << (pin % 8);
  if (buffer & mask) 
    result = 1;
  else
    result = 0;
  #ifdef DIAG_IO
  //DIAG(F("MCP23017 Read Addr:x%x Pin:%d Value:%d"), (int)_I2CAddress, (int)pin, result);
  #endif
  return result;
}
