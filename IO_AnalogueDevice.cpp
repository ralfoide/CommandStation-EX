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

#include "IO_AnalogueDevice.h"
#include "FSH.h"
#include "DIAG.h"

void Analogue::create(VPIN vpin, VPIN devicePin, int activePosition, int inactivePosition, enum ProfileType profile) {
  IODevice::remove(vpin);  // Delete any existing device that conflicts.
  Analogue *dev = new Analogue();
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


// Periodically update current position if it is changing.
// It's not worth going faster than 20ms as this is the pulse 
// frequency for the PWM Servo driver.  50ms is acceptable.
void Analogue::_loop() {
  unsigned int currentTime = millis(); // low 16 bits of millis()
  if (currentTime - _lastRefreshTime >= refreshInterval) { 
    updatePosition();
    _lastRefreshTime = currentTime;
  }
}

void Analogue::_write(VPIN vpin, int value) {
  #ifdef DIAG_IO
  DIAG(F("Analogue Write VPin:%d Value:%d"), vpin, value);
  #else
  (void)vpin;  // suppress compiler warning
  #endif
  if (value) value = 1;
  if (_state == value) return; // Nothing to do.
  _targetPosition = value ? _activePosition : _inactivePosition;
  _state = value;
  switch (_profile) {
    case Instant: 
      _increment = 0;
      break;
    case Fast:
      _increment = (_targetPosition - _currentPosition) / 10; // 500ms end-to-end
      break;
    case Medium:
      _increment = (_targetPosition - _currentPosition) / 20; // 1s end-to-end
      break;
    case Slow:
      _increment = (_targetPosition - _currentPosition) / 40; // 2s second end-to-end
      break;      
    case Bounce:
      _increment = 0;
      _stepNumber = 0;
      break;
    default:
      _increment = 0;
      break;
  }
  updatePosition();
  _lastRefreshTime = millis();
}

void Analogue::_display() {
  DIAG(F("Analogue VPin:%d->VPin:%d Range:%d-%d"), 
    _firstID, _devicePin, _activePosition, _inactivePosition);
}

// Private function to reposition servo
// TODO: Could calculate step number from elapsed time, to allow for erratic loop timing.
void Analogue::updatePosition() {
  bool changed = false;
  switch (_profile) {
    case Fast:
    case Medium:
    case Slow:
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
    case Bounce:
      if (_stepNumber < numSteps) {
        byte profileValue = GETFLASH(&profile[_stepNumber]);
        if (!_state) profileValue = 100 - profileValue;
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
// of the servo set by _activePosition and _inactivePosition.  The profile is symmetrical here,
// i.e. the bounce is the same on the down action as on the up action.
const byte FLASH Analogue::profile[numSteps] = 
    {2,3,7,13,33,50,83,100,83,75,70,65,60,60,65,74,84,100,83,75,70,70,72,75,80,87,92,97,100,100};
