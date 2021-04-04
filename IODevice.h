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

#define USE_BIGVPINS
#ifdef USE_BIGVPINS
  typedef uint16_t VPIN;
  #define VPIN_MAX 65534
  #define VPIN_NONE 65535
#else
  typedef uint8_t VPIN;
  #define VPIN_MAX 254
  #define VPIN_NONE 255
#endif


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
  // Device-specific read function.
  int _read(VPIN id);
  void _display();
  void writeRegister(byte reg, byte value);

  uint8_t _I2CAddress;
  uint8_t _currentPortState;

    // REGISTER ADDRESSES
  const byte PCA9685_MODE1=0x00;      // Mode Register 
  const byte PCA9685_FIRST_SERVO=0x06;  /** low byte first servo register ON*/
  const byte PCA9685_PRESCALE=0xFE;     /** Prescale register for PWM output frequency */
  // MODE1 bits
  const byte MODE1_SLEEP=0x10;   /**< Low power mode. Oscillator off */
  const byte MODE1_AI=0x20;      /**< Auto-Increment enabled */
  const byte MODE1_RESTART=0x80; /**< Restart enabled */
  
  const float FREQUENCY_OSCILLATOR=25000000.0; /** Accurate enough for our purposes  */
  const uint8_t PRESCALE_50HZ = (uint8_t)(((FREQUENCY_OSCILLATOR / (50.0 * 4096.0)) + 0.5) - 1);
  const uint32_t MAX_I2C_SPEED = 1000000L; // PCA9685 rated up to 1MHz I2C clock speed
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
  uint8_t _currentPortState;
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

  uint8_t _I2CAddress;
  uint8_t _currentPortState;
  enum {
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

// TBD: Implement optional pullups (currently permanently enabled by default).
 
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

#include "IO_PWMDevice.h"

#endif // iodevice_h