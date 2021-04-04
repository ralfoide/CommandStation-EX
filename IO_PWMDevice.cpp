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

#include "IO_PWMDevice.h"
#include "FSH.h"
#include "DIAG.h"

void PWM::create(VPIN vpin, int devicePin, int activePosition, int inactivePosition, enum ServoProfile profile) {
  IODevice::remove(vpin);  // Delete any existing device that conflicts.
  PWM *dev = new PWM();
  dev->_firstID = vpin;
  dev->_nPins = 1;
  dev->_devicePin = devicePin;
  dev->_activePosition = activePosition;
  dev->_inactivePosition = inactivePosition;
  dev->_profile = profile;
  dev->_currentPosition = dev->_targetPosition = dev->_inactivePosition;
  dev->_lastRefreshTime = millis();
  addDevice(dev);
}


// Periodically update current servo position if it is moving.
// It's not worth going faster than 20ms as this is the pulse 
// frequency for the PWM Servo driver.  50ms is acceptable.
void PWM::_loop() {
  unsigned int currentTime = millis(); // low 16 bits of millis()
  if (currentTime - _lastRefreshTime >= refreshInterval) { 
    updateServoPosition();
    _lastRefreshTime = currentTime;
  }
}

void PWM::_write(VPIN vpin, int value) {
  #ifdef DIAG_IO
  DIAG(F("PWM Write VPin:%d Value:%d"), vpin, value);
  #else
  (void)vpin;  // suppress compiler warning
  #endif
  if (value) value = 1;
  if (_state == value) return; // Nothing to do.
  _targetPosition = value ? _activePosition : _inactivePosition;
  _state = value;
  switch (_profile) {
    case SP_Instant: 
      _increment = 0;
      break;
    case SP_Fast:
      _increment = (_targetPosition - _currentPosition) / 10; // 500ms end-to-end
      break;
    case SP_Medium:
      _increment = (_targetPosition - _currentPosition) / 20; // 1s end-to-end
      break;
    case SP_Slow:
      _increment = (_targetPosition - _currentPosition) / 40; // 2s second end-to-end
      break;      
    case SP_Bounce:
      _increment = 0;
      _stepNumber = 0;
      break;
    default:
      _increment = 0;
      break;
  }
  updateServoPosition();
  _lastRefreshTime = millis();
}

void PWM::_display() {
  DIAG(F("PWM VPin:%d->VPin:%d Range:%d-%d"), 
    _firstID, _devicePin, _activePosition, _inactivePosition);
}

// Private function to reposition servo
// TODO: Calculate step number from elapsed time, to allow for erratic loops.
void PWM::updateServoPosition() {
  bool changed = false;
  switch (_profile) {
    case SP_Fast:
    case SP_Medium:
    case SP_Slow:
      if (_currentPosition != _targetPosition) {
        _currentPosition += _increment;
        changed = true;
        if (_increment > 0) {
          if (_currentPosition > _targetPosition)
            _currentPosition = _targetPosition;
        } else {
          if (_currentPosition < _targetPosition)
            _currentPosition = _targetPosition;
        }
      }
      break;
    case SP_Bounce:
      if (_stepNumber < numSteps) {
        byte profileValue = GETFLASH(&profile[_state][_stepNumber]);
        _currentPosition = map(profileValue, 
              0, 100, _inactivePosition, _activePosition);
        _stepNumber++;
        changed = true;
      }
      break;
    default: // Includes SP_Instant
      if (_currentPosition != _targetPosition) {
        _currentPosition = _targetPosition;
        changed = true;
      }
      break;
  }
      
  // Write to PWM module.
  if (changed)
    IODevice::write(_devicePin, _currentPosition);
}

// The profile below is in the range 0-100% and should be combined with the desired limits
// of the servo set by _activePosition and _inactivePosition.
const byte FLASH PWM::profile[][numSteps] = {
  {0,3,7,13,20,33,45,58,67,75,83,92,100,90,82,76,80,87,92,96,100,98,97,96,94,97,98,99,100,100},
  {100,98,97,93,87,67,50,17,0,17,40,52,65, 63,57,40,26,16,0,17,30,33,32,28,25,20,13,8,3,0}};
