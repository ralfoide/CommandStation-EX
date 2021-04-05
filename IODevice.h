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

#ifndef iodevice_h
#define iodevice_h

//#define DIAG_IO Y

#include "DIAG.h"
#include "FSH.h"
#include "I2CManager.h"

typedef uint16_t VPIN;
#define VPIN_MAX 65534
#define VPIN_NONE 65535

/*
 * IODevice class
 * 
 * This class is the basis of the Hardware Abstraction Layer (HAL) for
 * the DCC++EX Command Station.  All device classes derive from this.
 * 
 */
class IODevice;  // Pre-declare to allow references within definition.

class IODevice {
public:
  // Static functions to find the device and invoke its member functions
  static void begin();
  static void write(VPIN id, int value);
  static bool read(VPIN id);
  static void loop();
  static void DumpAll();
  static bool exists(VPIN vpin);
  static void remove(VPIN vpin);

  // When a turnout needs to allocate a vpin as its output, it allocates one using ID+turnoutVpinOffset.
  static const VPIN turnoutVpinOffset = 300; 
  // VPIN of first PCA9685 servo controller pin.  
  static const VPIN firstServoVPin = 100;
  
protected:
  // Method to perform initialisation of the device (optionally implemented within device class)
  virtual void _begin() {}
  // Method to write new state (optionally implemented within device class)
  virtual void _write(VPIN id, int value) {
    (void)id; (void)value;
  };
  // Method to read pin state (optionally implemented within device class)
  virtual int _read(VPIN id) { 
    (void)id; 
    return 0;
  };
  // Method to perform updates on an ongoing basis (optionally implemented within device class)
  virtual void _loop() {};
  // Method for displaying info on DIAG output (optionally implemented within device class)
  virtual void _display();

  // Destructor
  virtual ~IODevice() {};
  
  // Common object fields.
  VPIN _firstID;
  int _nPins;

  // Static support function for subclass creation
  static void addDevice(IODevice *newDevice);

private:
  // Method to check whether the id corresponds to this device
  bool owns(VPIN id);
  IODevice *_nextDevice = 0;
  static IODevice *_firstDevice;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Example of an IODevice subclass for PCA9685 16-channel PWM module.
 */
 
class PCA9685 : public IODevice {
public:
  static void create(VPIN firstID, int nPins, uint8_t I2CAddress);

private:
  // Constructor
  PCA9685();
  // Device-specific initialisation
  void _begin();
  // Device-specific write function.
  void _write(VPIN id, int value);
  void _display();
  void writeRegister(byte reg, byte value);

  uint8_t _I2CAddress;
  uint8_t _currentPortState;

};

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Example of an IODevice subclass for PCF8574 8-bit I/O expander.
 */
 
class PCF8574 : public IODevice {
public:
  static void create(VPIN firstID, int nPins, uint8_t I2CAddress);

private:
  // Constructor
  PCF8574();  
  // Device-specific initialisation
  void _begin();
  // Device-specific write function.
  void _write(VPIN id, int value);
  // Device-specific read function.
  int _read(VPIN id);
  void _display();

  uint8_t _I2CAddress;
  uint8_t _currentPortState = 0x00; 
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Example of an IODevice subclass for MCP23017 16-bit I/O expander.
 */
 
class MCP23017 : public IODevice {
public:
  static void create(VPIN firstID, int nPins, uint8_t I2CAddress);
  
private:
  // Constructor
  MCP23017();
  // Device-specific initialisation
  void _begin();
  // Device-specific write function.
  void _write(VPIN id, int value);
  // Device-specific read function.
  int _read(VPIN id);
  // Helper functions
  void writeRegister(uint8_t reg, uint8_t value) ;
  uint8_t readRegister(uint8_t reg);

  uint8_t _I2CAddress;
  uint8_t _currentPortStateA = 0;
  uint8_t _currentPortStateB = 0;
  uint8_t _portModeA = 0xff; // Read mode
  uint8_t _portModeB = 0xff; // Read mode

  enum {
    IODIRA = 0x00,
    IODIRB = 0x01,
    GPIOA = 0x12,
    GPIOB = 0x13
  };
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Example of an IODevice subclass for DCC accessory decoder.
 */
 
class DCCAccessoryDecoder: public IODevice {
public:
  static void create(VPIN firstID, int DCCAddress, int DCCSubaddress);
  static void create(VPIN firstID, int DCCLinearAddress);

private:
  // Constructor
  DCCAccessoryDecoder();
  // Device-specific write function.
  void _write(VPIN id, int value);
  void _display();
  int _DCCAddress;
  int _DCCSubaddress;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////
/* 
 *  Example of an IODevice subclass for arduino input/output pins.
 */

// TODO: Implement pullup configuration (currently permanently enabled by default).
 
class ArduinoPins: public IODevice {
public:
  static void create(VPIN firstID, int nPins) {
    addDevice(new ArduinoPins(firstID, nPins));
  }
  
  // Constructor
  ArduinoPins(VPIN firstID, int nPins);

private:
  // Device-specific write function.
  void _write(VPIN id, int value);
  // Device-specific read function.
  int _read(VPIN id);
  void _display();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////

#include "IO_AnalogueDevice.h"

#endif // iodevice_h