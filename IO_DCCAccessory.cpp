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

#include "DCC.h"
#include "IODevice.h"
#include "DIAG.h"

void DCCAccessoryDecoder::create(VPIN firstID, int nPins, int DCCAddress, int DCCSubaddress) {
  addDevice(new DCCAccessoryDecoder(firstID, nPins, DCCAddress, DCCSubaddress));
}
void DCCAccessoryDecoder::create(VPIN firstID, int nPins, int DCCLinearAddress) {
  int DCCAddress = (DCCLinearAddress-1)/4;
  int DCCSubaddress = DCCLinearAddress - DCCAddress*4;
  addDevice(new DCCAccessoryDecoder(firstID, nPins, DCCAddress, DCCSubaddress));
}

// Constructor
DCCAccessoryDecoder::DCCAccessoryDecoder(VPIN firstID, int nPins, int DCCAddress, int DCCSubaddress) {
  _firstID = firstID;
  _nPins = nPins;
  _DCCAddress = DCCAddress;
  _DCCSubaddress= DCCSubaddress;
}

// Device-specific write function.
void DCCAccessoryDecoder::_write(VPIN id, int state) {
  int subaddress = id-_firstID+_DCCSubaddress;
  #ifdef DIAG_IO
  DIAG(F("DCC Write Addr:%d Subddr:%d State:%d"), _DCCAddress, subaddress, state);
  #endif
  DCC::setAccessory(_DCCAddress, subaddress, state);
}

void DCCAccessoryDecoder::_display() {
  #ifdef DIAG_IO
  DIAG(F("DCC Addr:%d VPins:%d-%d"), (int)_DCCAddress, (int)_firstID, (int)_firstID+_nPins-1);
  #endif
}

