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

// Method to check whether the id corresponds to this device
bool IODevice::owns(VPIN id) {
  return (id >= _firstID && id < _firstID + _nPins);
}

// Static functions
void IODevice::begin() {
  // Initialise the IO subsystem
  ArduinoPins::create(2, 48);  // Reserve pins numbered 2-49 for direct access
  // On devices with 32k flash, don't predefine PCA9685.  It can be added in mysetup,h
  #if !defined(ARDUINO_AVR_NANO) && !defined(ARDUINO_AVR_UNO)
  PCA9685::create(IODevice::firstServoVPin, 16, 0x40); // Predefine one PCA9685 module
  #endif
}

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

bool IODevice::exists(VPIN vpin) {
  for (IODevice *dev = _firstDevice; dev != 0; dev = dev->_nextDevice) {
    if (dev->owns(vpin)) return true;
  }
  return false;
}

void IODevice::remove(VPIN vpin) {
  // Only works if the object is exclusive, i.e. only one VPIN.
  IODevice *previousDev = 0;
  for (IODevice *dev = _firstDevice; dev != 0; dev = dev->_nextDevice) {
    if (dev->owns(vpin)) {
      // Found object
      if (dev->_nPins == 1) {
        // Now unlink
        if (!previousDev)
          _firstDevice = dev->_nextDevice;
        else
          previousDev->_nextDevice = dev->_nextDevice;
        delete dev;
      }
    }
    previousDev = dev;
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
  digitalWrite(pin, value);
  pinMode(pin, OUTPUT);
}

// Device-specific read function.
int ArduinoPins::_read(VPIN id) {
  int pin = id;
  pinMode(pin, INPUT_PULLUP);
  int value = digitalRead(pin);
  #ifdef DIAG_IO
  //DIAG(F("Arduino Read Pin:%d Value:%d"), pin, value);
  #endif
  return value;
}

void ArduinoPins::_display() {
  DIAG(F("Arduino VPins:%d-%d"), (int)_firstID, (int)_firstID+_nPins-1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

