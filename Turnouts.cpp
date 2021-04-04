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
  IODevice::write(300+data.id, state);
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
      int inactiveAngle = (data.inactiveAngle >> 4) & 0xfff;
      int activeAngle = ((data.inactiveAngle & 0xf) << 8) | data.moveAngle;
      tt=create(data.id,data.tStatus & STATUS_PWMPIN, activeAngle, inactiveAngle);
    } else 
      tt=create(data.id,data.address,data.subAddress);
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
Turnout *Turnout::create(int id, VPIN vpin){
  Turnout *tt=create(id);
  tt->data.address = vpin;
  tt->data.tStatus=0;
  tt->data.address = 0;
  tt->data.subAddress = 0;
  return(tt);
}

// Legacy method for creating a DCC-controlled turnout.
Turnout *Turnout::create(int id, int add, int subAdd){
  Turnout *tt=create(id);
  tt->data.address=add;
  tt->data.subAddress=subAdd;
  tt->data.tStatus=0;
  DCCAccessoryDecoder::create(300+id, add, subAdd);
  return(tt);
}

// Legacy method for creating a PCA9685 PWM turnout.
// Pin here is numbered from 0 within PCA9685, i.e. vpins 100-115 in first PCA9685.
Turnout *Turnout::create(int id, byte pin, int activeAngle, int inactiveAngle){
  Turnout *tt=create(id);
  tt->data.tStatus= STATUS_PWM | (pin &  STATUS_PWMPIN);
  // Pack active/inactive angles into available space.
  tt->data.inactiveAngle = (inactiveAngle << 4) | (activeAngle >> 8); 
                                  // inactiveAngle | high 4 bits of activeAngle.
  tt->data.moveAngle = activeAngle & 0xff;  // low 8 bits of activeAngle.
  // Create PWM object
  PWM::create(300+id, 100+pin, activeAngle, inactiveAngle, PWM::SP_Medium);
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
