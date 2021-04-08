/*
 *  © 2013-2016 Gregg E. Berman
 *  © 2020, Chris Harlow. All rights reserved.
 *  © 2020, Harald Barth.
 *  
 *  This file is part of Asbelos DCC API
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
#include "Turnouts.h"
#include "EEStore.h"
#include "StringFormatter.h"
#ifdef EESTOREDEBUG
#include "DIAG.h"
#endif

// print all turnout states to stream
// TODO: Echo sensible values for servo turnouts.
void Turnout::printAll(Print *stream){
  for (Turnout *tt = Turnout::firstTurnout; tt != NULL; tt = tt->nextTurnout)
    StringFormatter::send(stream, F("<H %d %d>\n"), tt->data.id, (tt->data.tStatus & STATUS_ACTIVE)!=0);
} // Turnout::printAll

bool Turnout::activate(int n,bool state){
#ifdef EESTOREDEBUG
  DIAG(F("Turnout::activate(%d,%d)"),n,state);
#endif
  Turnout * tt=get(n);
  if (tt==NULL) return false;
  tt->activate(state);
  EEStore::store();
  turnoutlistHash++;
  return true;
}

bool Turnout::isActive(int n){
  Turnout * tt=get(n);
  if (tt==NULL) return false;
  return tt->data.tStatus & STATUS_ACTIVE;
}

// activate is virtual here so that it can be overridden by a non-DCC turnout mechanism
void Turnout::activate(bool state) {
#ifdef EESTOREDEBUG
  DIAG(F("Turnout::activate(%d)"),state);
#endif
  if (data.address==LCN_TURNOUT_ADDRESS) {
     // A LCN turnout is transmitted to the LCN master.
     LCN::send('T',data.id,state);
     return;   // The tStatus will be updated by a message from the LCN master, later.    
  }
  if (state)
    data.tStatus|=STATUS_ACTIVE;
  else
    data.tStatus &= ~STATUS_ACTIVE;
  int pin = data.tStatus & STATUS_PWMPIN;
  IODevice::write(IODevice::firstServoVPin + pin, state);
  EEStore::store();
}
///////////////////////////////////////////////////////////////////////////////

Turnout* Turnout::get(int n){
  Turnout *tt;
  for(tt=firstTurnout;tt!=NULL && tt->data.id!=n;tt=tt->nextTurnout);
  return(tt);
}
///////////////////////////////////////////////////////////////////////////////

bool Turnout::remove(int n){
  Turnout *tt,*pp=NULL;

  for(tt=firstTurnout;tt!=NULL && tt->data.id!=n;pp=tt,tt=tt->nextTurnout);

  if(tt==NULL) return false;
  
  if(tt==firstTurnout)
    firstTurnout=tt->nextTurnout;
  else
    pp->nextTurnout=tt->nextTurnout;

  free(tt);
  turnoutlistHash++;
  return true; 
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::load(){
  struct TurnoutData data;
  Turnout *tt;

  for(int i=0;i<EEStore::eeStore->data.nTurnouts;i++){
    EEPROM.get(EEStore::pointer(),data);
    if (data.tStatus & STATUS_PWM) {
      // Unpack PWM values
      int inactivePosition = (data.positionWord >> 4) & 0xfff;
      int activePosition = ((data.positionWord & 0xf) << 8) | data.positionByte;
      tt=create(data.id,data.tStatus & STATUS_PWMPIN, activePosition, inactivePosition);
    } else if (data.address==VPIN_TURNOUT_ADDRESS) 
      tt=create(data.id,data.subAddress);  // VPIN-based turnout
    else
      tt=create(data.id,data.address,data.subAddress); // DCC/LCN-based turnout
    tt->data.tStatus=data.tStatus;
    EEStore::advance(sizeof(tt->data));
#ifdef EESTOREDEBUG
    tt->print(tt);
#endif
  }
}

///////////////////////////////////////////////////////////////////////////////

void Turnout::store(){
  Turnout *tt;

  tt=firstTurnout;
  EEStore::eeStore->data.nTurnouts=0;

  while(tt!=NULL){
#ifdef EESTOREDEBUG
    tt->print(tt);
#endif
    EEPROM.put(EEStore::pointer(),tt->data);
    EEStore::advance(sizeof(tt->data));
    tt=tt->nextTurnout;
    EEStore::eeStore->data.nTurnouts++;
  }

}
///////////////////////////////////////////////////////////////////////////////

// Method for associating a turnout id with a virtual pin in IODevice space.
// The actual creation and configuration of the pin must be done elsewhere,
// e.g. in mySetup.h during startup of the CS.
// TODO: Vpin is currently a byte so limited to 0-255.
Turnout *Turnout::create(int id, VPIN vpin){
  Turnout *tt=create(id);
  tt->data.address = VPIN_TURNOUT_ADDRESS;
  tt->data.tStatus=0;
  tt->data.subAddress = vpin;
  return(tt);
}

// Legacy method for creating a DCC-controlled turnout.
Turnout *Turnout::create(int id, int add, int subAdd){
  Turnout *tt=create(id);
  tt->data.address=add;
  tt->data.subAddress=subAdd;
  tt->data.tStatus=0;
  DCCAccessoryDecoder::create(IODevice::turnoutVpinOffset+id, add, subAdd);
  return(tt);
}

// Legacy method for creating a PCA9685 PWM turnout.
// Pin here is numbered from 0 within PCA9685.
Turnout *Turnout::create(int id, byte pin, int activePosition, int inactivePosition, int profile){
  Turnout *tt=create(id);
  tt->data.tStatus= STATUS_PWM | (pin &  STATUS_PWMPIN);
  // Pack active/inactive positions into available space.
  tt->data.positionWord = (inactivePosition << 4) | (activePosition >> 8); 
                                  // inactivePosition | high 4 bits of activePosition.
  tt->data.positionByte = activePosition & 0xff;  // low 8 bits of activeAngle.
  // Create PWM interface object 
  VPIN vpin = IODevice::firstServoVPin + pin;
  Analogue::create(vpin, vpin, activePosition, inactivePosition, profile);
  return(tt);
}

Turnout *Turnout::create(int id){
  Turnout *tt=get(id);
  if (tt==NULL) { 
     tt=(Turnout *)calloc(1,sizeof(Turnout));
     tt->nextTurnout=firstTurnout;
     firstTurnout=tt;
     tt->data.id=id;
    }
  turnoutlistHash++;
  return tt;
}

///////////////////////////////////////////////////////////////////////////////
//
// print debug info about the state of a turnout
//
#ifdef EESTOREDEBUG
void Turnout::print(Turnout *tt) {
    if (tt->data.tStatus & STATUS_PWM )
      DIAG(F("Turnout %d ZeroAngle %d MoveAngle %d Status %d"),tt->data.id, tt->data.inactiveAngle, tt->data.moveAngle,tt->data.tStatus & STATUS_ACTIVE);
    else
	DIAG(F("Turnout %d Addr %d Subaddr %d Status %d"),tt->data.id, tt->data.address, tt->data.subAddress,tt->data.tStatus & STATUS_ACTIVE);
}
#endif

Turnout *Turnout::firstTurnout=NULL;
int Turnout::turnoutlistHash=0; //bump on every change so clients know when to refresh their lists
