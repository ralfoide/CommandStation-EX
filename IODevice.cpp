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

#include <Arduino.h>
#include "IODevice.h"
#include "DIAG.h" 
#include "FSH.h"

#define FASTIO  // Use direct port manipulation instead of digitalRead/Write.


// Method to check whether the id corresponds to this device
bool IODevice::owns(VPIN id) {
  return (id >= _firstID && id < _firstID + _nPins);
}

// Static functions
void IODevice::loop() {
  // Call every device's loop function in turn.
  for (IODevice *dev = _firstDevice; dev != 0; dev = dev->_nextDevice) {
    dev->_loop();
  }
}

void IODevice::DumpAll() {
  for (IODevice *dev = _firstDevice; dev != 0; dev = dev->_nextDevice) {
    dev->_display();
  }
}

void IODevice::_display() {
  DIAG(F("Unknown device VPins:%d-%d"), (int)_firstID, (int)_firstID+_nPins-1);
}

// Write value to virtual pin(s).  If multiple devices are allocated the same pin
//  then they will be written in turn.
void IODevice::write(VPIN id, int value) {
  bool found = false;
  for (IODevice *dev = _firstDevice; dev != 0; dev = dev->_nextDevice) {
    if (dev->owns(id)) {
      dev->_write(id, value);
      found = true;
    }
  }
#ifdef DIAG_IO
  if (!found)
    DIAG(F("IODevice::write(): VPin ID %d not found!"), (int)id);
#else 
  (void)found; // Suppress compiler warning
#endif
}

// Read value from virtual pin.
bool IODevice::read(VPIN id) {
  for (IODevice *dev = _firstDevice; dev != 0; dev = dev->_nextDevice) {
    if (dev->owns(id)) 
      return dev->_read(id);
  }
#ifdef DIAG_IO
  DIAG(F("IODevice::read(): VPin %d not found!"), (int)id);
#endif
  return false;
}

void IODevice::addDevice(IODevice *newDevice) {
  // Link new object to the end of chain.  Thereby,
  // the objects that are defined first will be found 
  // more quickly.
  IODevice *dev = _firstDevice;
  if (!dev) 
    _firstDevice = newDevice;
  else {
    while (dev->_nextDevice != 0)
      dev = dev->_nextDevice;
    dev->_nextDevice = newDevice;
  }
  newDevice->_nextDevice = 0;

  // Initialise device
  newDevice->_begin();
}

// Start of chain of devices.
IODevice *IODevice::_firstDevice = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////

// Constructor
ArduinoPins::ArduinoPins(VPIN firstID, int nPins) {
  _firstID = firstID;
  _nPins = nPins;
}

// Device-specific write function.
void ArduinoPins::_write(VPIN id, int value) {
  int pin = id;
  #ifdef DIAG_IO
  DIAG(F("Arduino Write Pin:%d Val:%d"), pin, value);
  #endif
#if !defined(FASTIO)
  digitalWrite(pin, value);
  pinMode(pin, OUTPUT);
#else
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *portRegister = portInputRegister(port);
  volatile uint8_t *portModeRegister = portModeRegister(port);
  uint8_t mask = digitalPinToBitMask(pin);
  if (value) 
    *portRegister |= mask;
  else  
    *portRegister &= ~mask;
  *portModeRegister |= mask;  // Set to write mode
#endif
}

// Device-specific read function.
int ArduinoPins::_read(VPIN id) {
  int pin = id;
#if !defined(FASTIO)
  pinMode(pin, INPUT_PULLUP);
  int value = digitalRead(pin);
#else
  uint8_t port = digitalPinToPort(pin);
  volatile uint8_t *portRegister = portInputRegister(port);
  volatile uint8_t *portModeRegister = portModeRegister(port);
  uint8_t mask = digitalPinToBitMask(pin);
  *portModeRegister &= ~mask; // Set to read mode
  *portRegister |= mask;   // Enable pull-up
  int value = (*portRegister & mask) ? 1 : 0; // Read value
#endif
  #ifdef DIAG_IO
  //DIAG(F("Arduino Read Pin:%d Value:%d"), pin, value);
  #endif
  return value;
}

void ArduinoPins::_display() {
  DIAG(F("Arduino VPins:%d-%d"), (int)_firstID, (int)_firstID+_nPins-1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

