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

/* 
 *  Class PWM for servos and variable brightness LEDs.  Well, anything
 *  that can be driven by a Pulse Width Modulated signal, really...
 *  In fact, all the class does is converts the values of '1' and 
 *  '0' on the input pin into the values corresponding to 'activePosition' 
 *  and 'inactivePosition' respectively on the output pin, with a 
 *  configurable progression between states.  The device 
 *  attached to the output pin decides whether it's PWM or whatever!
 */

#ifndef pwmdevice_h
#define pwmdevice_h

#include "IODevice.h"
#include "FSH.h"

class PWM : public IODevice {
public:

  enum ServoProfile {
    SP_Instant = 0,  // Moves immediately between positions
    SP_Fast = 1,     // Takes around 500ms end-to-end
    SP_Medium = 2,   // 1 second end-to-end
    SP_Slow = 3,     // 2 seconds end-to-end
    SP_Bounce = 4    // For semaphores/turnouts with a bit of bounce!!
  };

  static void create(VPIN vpin, int devicePin, int activePosition, int inactivePosition, enum ServoProfile profile) {
    addDevice(new PWM(vpin, devicePin, activePosition, inactivePosition, profile));
  }
  
  PWM(VPIN vpin, int devicePin, int activePosition, int inactivePosition, enum ServoProfile profile) {
    _firstID = vpin;
    _nPins = 1;
    _devicePin = devicePin;
    _activePosition = activePosition;
    _inactivePosition = inactivePosition;
    _profile = profile;
    _currentPosition = _targetPosition = _inactivePosition;
    _lastRefreshTime = millis();
  }

  void _loop();
  void _write(VPIN vpin, int value);
  void _display();
  
private:
  // Reposition servo
  void updateServoPosition();
        
  int _devicePin = 0;
  int _activePosition;
  int _inactivePosition;
  int _currentPosition;
  int _state = 0;
  int _stepNumber = 0;
  static const int numSteps = 30;
  static const byte PROGMEM profile[][numSteps];
  int _targetPosition;
  int _increment;
  enum ServoProfile _profile;
  const unsigned int refreshInterval = 50; // refresh every 50ms
  unsigned int _lastRefreshTime; // low 16-bits of millis() count.

};

#endif // pwmdevice_h