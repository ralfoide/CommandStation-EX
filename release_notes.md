The DCC-EX Team is pleased to release CommandStation-EX-v3.0.0 as a Production Release.  This release is a major re-write of earlier versions.  We've re-architected the code-base so that it can better handle new features going forward.  

**Known Bugs:**
 - **Consisting through JMRI** - currently does not work in this release.  You may use the <M> command to do this manually.
 - **Wi-Fi** - works, but can be challenging to use if you want to switch between AP mode and STA station mode.
 - **Pololu Motor Shield** - is supported with this release, but the user may have to play around with some timings to enable programming mode due to limitation in its current sensing circuitry 

**Summary of the key new features added to CommandStation-EX V3.0.7**
  - **Support for 28 Speed steps** - Option to turn on 28 step speed decoders in addition to 128. If set, all locos will use 28 steps.
  - **Improved overload messages with raw values (relative to offset)**

**Summary of the key new features added to CommandStation-EX V3.0.6**

 - **Prevent compiler warning about deprecated B constants**
 - **Fix Bug that did not let us transmit 5 byte sized packets** - 5 Byte commands like PoM (programming on main) were not being sent correctly
 - **Huge function numbers (DCC BinaryStateControl)** - Support Functions beyond F28
 - **<!> ESTOP all** - New command to emergency stop all locos on the main track
 - **<- [cab]> estop and forget cab/all cabs** - Stop and remove loco from the CS. Stops the repeating throttle messages
 - **``<D RESET>`` command to reboot arduino**
 - **Automatic sensor offset detect** -
 - **Improved startup msgs from Motor Drivers (accuracy and auto sense factors)** -
 - **Drop post-write verify** - No need to double check CV writes. Writes are now even faster.
 - **Allow current sene pin set to UNUSED_PIN** - No need to ground an unused analog current pin. Produce startup warning and callback -2 for prog track cmds.

**Summary of the key new features added to CommandStation-EX V3.0.5**
 - **Fix Fn Key startup with loco ID and fix state change for F16-28**
 - removed ethernet mac config and made it automatic
 - show wifi ip and port on lcd
 - auto load config.example.h with warningh
 - dropped example .ino files
 - corrected .ino comments 
 - pololu fault pin handling
 - waveform speed/simplicity improvements
 - improved pin speed in waveform 
 - portability to nanoEvery and UnoWifiRev2 CPUs
 - analog read speed improvements
 - drop need for DIO2 library
 - improved current check code
 - linear <a> command
 - removed need for ArduinoTimers files
 - removed <D DCC SLOW> 
 - Removed option to choose different timer
 - Added EX-RAIL hooks for later
 - fixed Turnout list
 - allow command keywords in mixed case
 - dropped unused memstream
 - PWM pin accuracy if requirements met.

**Summary of the key new features added to CommandStation-EX V3.0.4**
 - **"Drive-Away" Feature added so that throttles like Engine Driver can allow a loco to be programmed on a usable, electrically isolated programming track and then drive off onto the main track.
 - **WiFi Startup Fixes**
 
**Summary of the key new features added to CommandStation-EX V3.0.3**
 - **<W addr> command to write loco address and clear consist** 
 - **<R> command will allow for consist address**
 - **Startup commands implemented**
 
**Summary of the key new features added to CommandStation-EX V3.0.2:**
- **Create new output for current in mA for ``<c>`` command** - New current response outputs current in mA, overlimit current, and maximum board capable current
- **Simultaneously update JMRI to handle new current meter**

**Summary of the key new features added to CommandStation-EX V3.0.1:**
 - **Add back fix for jitter**
 - **Add Turnouts, Outputs and Sensors to ```<s>``` command output**

 **Summary of the key new features added to CommandStation-EX V3.0.0:**

 - **New USB Browser Based Throttle** - WebThrottle-EX is a full front-end to controller to control the CS to run trains.
 - **WiFi Support** - AP and station modes supported. Auto-detection of an ESP8266 WiFi module with AT firmware on a Mega's serial port. Connection to JMRI and WiThrottle clients.
 - **Withrottle Integrations** - Act as a host for up to four WiThrottle clients concurrently. 
 - **Add LCD/OLED support** - OLED supported on Mega only
 - **Improved CV programming routines** - checks for length of CV pulse, and breaks out of the wait state once it has received an ACK, now reading one CV per second.
 - **Improved current sensing** - rewrote current sensing routines for safer operation. Current thresholds based on milliamps, not magic numbers
 - **Individual track power control** - Ability to toggle power on either or both tracks, and to "JOIN" the tracks and make them output the same waveform for multiple power districts.
 - **Single or Dual-Pin PWM output** - Allows control of H-bridges with PH/EN or dual PWM inputs
 - **New, simpler function command** - ```<F>``` command allows setting functions based on their number, not based on a code as in ```<f>```
 - **Function reminders** - Function reminders are sent in addition to speed reminders
 - **Functions to F28** - All NMRA functions are now supported
 - **Filters and user functions** - Ability to filter commands in the parser and execute custom code based on them. (ex: Redirect Turnout commands via NRF24)
 - **Diagnostic ```<D>``` commands** - See documentation for a full list of new diagnostic commands
 - **Rewrote DCC++ Parser** - more efficient operation, accepts multi-char input and uses less RAM
 - **Rewritten waveform generator** - capable of using any pin for DCC waveform out, eliminating the need for jumpers
 - **Rewritten packet generator** - Simplify and make smaller, remove idea of "registers" from original code
 - **Add free RAM messages** - Free RAM messages are now printed whenever there is a decerase in available RAM
 - **Fix EEPROM bugs**
 - **Number of locos discovery command** - ```<#>``` command 
 - **Support for more locomotives** - 20 locomotives on an UNO and 50 an a Mega.
 - **Automatic slot managment** - slot variable in throttle/function commands are ignored and slot management is taken care of automatically. ```<!>``` command added to release locos from memory.


**Key Contributors**

**Project Lead**
- Fred Decker - Holly Springs, North Carolina, USA (FlightRisk)

**CommandStation-EX Developers**
- Chris Harlow - Bournemouth, UK (UKBloke)
- Harald Barth - Stockholm, Sweden (Haba)
- Neil McKechnie - Worcestershire, UK (NeilMck)
- Fred Decker - Holly Springs, North Carolina, USA (FlightRisk)
- Dave Cutting - Logan, Utah, USA (Dave Cutting/ David Cutting)
- M Steve Todd - - Engine Driver and JMRI Interface
- Scott Catalanno - Pennsylvania
- Gregor Baues - Île-de-France, France (grbba)

**exInstaller Software**
- Anthony W - Dayton, Ohio, USA (Dex, Dex++)

**Website and Documentation**
- Mani Kumar - Bangalor, India (Mani / Mani Kumar)
- Fred Decker - Holly Springs, North Carolina, USA (FlightRisk)
- Dave Cutting - Logan, Utah, USA (Dave Cutting/ David Cutting)
- Roger Beschizza - Dorset, UK (Roger Beschizza)
- Keith Ledbetter - Chicago, Illinois, USA (Keith Ledbetter)
- Kevin Smith - (KCSmith)

**WebThrotle-EX**
- Fred Decker - Holly Springs, NC (FlightRisk/FrightRisk)
- Mani Kumar - Bangalor, India (Mani /Mani Kumar)
- Matt H - Somewhere in Europe



**Beta Testing / Release Management / Support**
- Larry Dribin	- Release Management
- Keith Ledbetter	
- BradVan der Elst	
- Andrew Pye	
- Mike Bowers	
- Randy McKenzie
- Roberto Bravin
- Sim Brigden
- Alan Lautenslager
- Martin Bafver	
- Mário André Silva	
- Anthony Kochevar	
- Gajanatha Kobbekaduwe	
- Sumner Patterson 
- Paul - Virginia, USA
