// ==============================================
// ATTENTION!!!!!
// YOU MUST SET ONLY THIS SECTION
// ==============================================

#include <EEPROM.h>
#include <OneWire.h>


// should be false
#define FakeCooling     false       // For development only.
#define Silent          false       // No beeps (during development).

// Serial debugging
#define DebugProcess    false
#define DebugReadWrite  false
#define DebugErrors     true


// Default language is English, others must be set.
#define langNL          true

// Don not change this next block
#if FakeCooling == true
#define USE_DS18020     false
#else
#define USE_DS18020     true
#endif

// hardware setup, adjust for your own board.
#if USE_DS18020 == true
const byte Sensor1Pin =  7;
const byte Sensor2Pin = 11;
#endif
#define BuzzerPin        8
#define Cooler1Pin       9
#define Cooler2Pin      10

// Keyboard pins
#define ButtonUpPin     A3
#define ButtonDownPin   A2
#define ButtonStartPin  A1
#define ButtonEnterPin  A0


#if USE_DS18020 == true
OneWire ds1(Sensor1Pin);
OneWire ds2(Sensor2Pin);
#endif

// LCD connections
#include <LiquidCrystal.h>
LiquidCrystal lcd(A4, A5, 2, 3, 4, 5);

// ==============================================
// END OF SETTING SECTION
// ==============================================

/*
 MBSE-ArdCool is a simple Coolbox controller using the MBSE-ArdRims
 hardware. It can control two cheap outdoor coolers that have no
 regulation of their own.
 

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


// *************************
//*  global variables
// *************************

#include "defines.h"

unsigned long gSystemStartTime;          // in milliseconds.
unsigned long gCurrentTimeInMS;          // in milliseconds.
unsigned long Timer;
#if FakeCooling == true
unsigned long FakeHeatLastInMS;          // in milliseconds.
#endif

#if USE_DS18020 == true
boolean Conv1_start = false;
boolean Conv2_start = false;
#endif
boolean NewSecond = false;
boolean Box1_on;
boolean Box2_on;


byte    mainMenu = 0;
byte    Direction;

#if FakeCooling == true
float   Temp_1 = 18.90;
float   Temp_2 = 18.90;
float   Plate_1 = 18.90;
float   Plate_2 = 18.90;
#else
float   Temp_1 = 0.0;
float   Temp_2 = 0.0;
#endif
float   SetPoint_1 = 12.0;
float   SetPoint_2 = 12.0;


void Cool1_on();
void Cool1_off();
void Cool2_on();
void Cool2_off();
void Buzzer(byte, int);

#include "functions.h"
#include "timers.h"
#include "buttons.h"
#include "prompts.h"


#if (DebugProcess == true)
void DebugTimeSerial() {
  byte Hour, Minute, Second;
  unsigned int Millisecond;

  Hour        = (byte) ((gCurrentTimeInMS / 1000) / 3600);
  Minute      = (byte)(((gCurrentTimeInMS / 1000) % 3600) / 60);
  Second      = (byte) ((gCurrentTimeInMS / 1000) % 60);
  Millisecond = (unsigned int)gCurrentTimeInMS % 1000;
  Serial.print("Time: ");
  if (Hour < 10)
    Serial.print("0");
  Serial.print(Hour);
  Serial.print(":");
  if ( Minute < 10)
    Serial.print("0");
  Serial.print(Minute);
  Serial.print(":");
  if (Second < 10)
    Serial.print("0");
  Serial.print(Second);
  Serial.print(".");
  if (Millisecond <   10)
    Serial.print("0");
  if (Millisecond <  100)
    Serial.print("0");
  Serial.print(Millisecond);
  Serial.print(" ");
}
#endif



#if USE_DS18020 == true
byte OwsInitialize(OneWire ows) {
  if (ows.reset()) {   // return 1 if present, 0 if not.
    ows.skip();
    return 1;
  }
  return 0;
}


void ReadOwSensor(OneWire ows, boolean & Convert_start, float & TempC, boolean Offset) {
  byte data[9];

  // start conversion and return
  if (!(Convert_start)) {
    if (! OwsInitialize(ows))
      return;
    ows.write(0x44, 0);
    Convert_start = true;
    return;
  }
  if (Convert_start) {

    // check for conversion if it isn't complete return if it is then convert to decimal
    if (ows.read_bit() == 0)
      return;

    // Allways a new start after the next steps
    Convert_start = false;
    if (OwsInitialize(ows)) {
      ows.write(0xBE);                           // Read scratchpad
      ows.read_bytes(data, 9);
      if ( OneWire::crc8(data, 8) != data[8]) {
        // if checksum fails start a new conversion.
        ew_byte(EM_ErrorNo(0), er_byte(EM_ErrorNo(0)) + 1);        // error counter 0
        return;
      }
    } else {
      return;
    }

    /*
     * After a sensor is connected, or after power-up, the sensor resolution
     * can be different from what we desire. If so, configure the sensor and
     * start over again.
     */
    if ((data[4] & 0x60) != 0x60) {
      OwsInitialize(ows);
      ows.write(0x4E);           // Write scratchpad
      ows.write(0);              // TL register
      ows.write(0);              // TH register
      ows.write(0x7F);           // Configuration 12 bits, 750 ms
      return;
    }
    int16_t raw = (data[1] << 8) | data[0];

    /*
     * Check sign bits, must be all zero's or all one's.
     */
    if ((raw & 0xf800) != 0) {
      ew_byte(EM_ErrorNo(1), er_byte(EM_ErrorNo(1)) + 1);        // error counter 1
      return;
    }
    if ((raw & 0xf800) == 0xf800) {
      ew_byte(EM_ErrorNo(2), er_byte(EM_ErrorNo(1)) + 2);        // error counter 2
      return;
    }

    TempC = (float)raw / 16.0;
  }
}
#endif



/*
 * Read Temperature sensors
 */
void Temperature() {

#if USE_DS18020 == true
  ReadOwSensor(ds1, Conv1_start, Temp_1, true);
  ReadOwSensor(ds2, Conv2_start, Temp_2, true);
#endif

#if FakeCooling == true
  TimerRun();

  // Try to be as slow as a real sensor
  if ((gCurrentTimeInMS - FakeHeatLastInMS) < 500)
    return;
  /*
   * Make this fake cooler a bit more real by using a simulated coolplate.
   * We cool that plate and then transfer the cold to the air.
   * That way we get a nice slow effect with overshoot.
   */
  if (digitalRead(Cooler1Pin) == HIGH) {
    if (Plate_1 > -10.0)
      Plate_1 -= (gCurrentTimeInMS - FakeHeatLastInMS) * 0.0001;   // Simulate plate downto -10 degrees
  } else {
    if (Plate_1 < Temp_1)
      Plate_1 += (gCurrentTimeInMS - FakeHeatLastInMS) * 0.00002 * (Temp_1 - Plate_1);
  }
  if (digitalRead(Cooler2Pin) == HIGH) {
    if (Plate_2 > -10.0)
      Plate_2 -= (gCurrentTimeInMS - FakeHeatLastInMS) * 0.0001;   // Simulate plate downto -10 degrees
  } else {
    if (Plate_2 < Temp_2)
      Plate_2 += (gCurrentTimeInMS - FakeHeatLastInMS) * 0.00002 * (Temp_2 - Plate_2);
  }
 
  // If plate is colder then the air, cool the air
  if (Plate_1 < Temp_1) {
    Temp_1 -= (gCurrentTimeInMS - FakeHeatLastInMS) * 0.000001 * (Temp_1 - Plate_1);
  }
  if (Plate_2 < Temp_2) {
    Temp_2 -= (gCurrentTimeInMS - FakeHeatLastInMS) * 0.000001 * (Temp_2 - Plate_2);
  }
  
  // Lose temperature to the outside world.
  if (Temp_1 < 21.0) {
    Temp_1 += (gCurrentTimeInMS - FakeHeatLastInMS) * 0.00000010 * (21.0 - Temp_1);
  }
  if (Temp_2 < 21.0) {
    Temp_2 += (gCurrentTimeInMS - FakeHeatLastInMS) * 0.00000010 * (21.0 - Temp_2);
  }

  FakeHeatLastInMS = gCurrentTimeInMS;
#endif
}


/*
 * Run the coolers
 */
void Coolers(void) {
  TimerRun();
  Temperature();
  
  if (Box1_on) {
    if (digitalRead(Cooler1Pin) == HIGH) {
      if ((Temp_1 + 0.1) < SetPoint_1)
        Cool1_off();
    } else {
      if ((Temp_1 - 0.1) >= SetPoint_1)
        Cool1_on();
    }
  }
  if (Box2_on) {
    if (digitalRead(Cooler2Pin) == HIGH) {
      if ((Temp_2 + 0.1) < SetPoint_2)
        Cool2_off();
    } else {
      if ((Temp_2 - 0.1) >= SetPoint_2)
        Cool2_on();
    }
  }
  
#if DebugProcess == true
  if (NewSecond) {
    DebugTimeSerial();
    Serial.print(F(" 1: "));
    Serial.print(Temp_1);
    Serial.print(F("/"));
    Serial.print(SetPoint_1);
#if FakeCooling == true
    Serial.print(F(" Plate="));
    Serial.print(Plate_1);
#endif
    Serial.print(F("   2: "));
    Serial.print(Temp_2);
    Serial.print(F("/"));
    Serial.print(SetPoint_2);
#if FakeCooling == true
    Serial.print(F(" Plate="));
    Serial.print(Plate_2);
#endif
  
    Serial.println();
  }
#endif
  NewSecond = false;
}


void LCDChar(byte X, byte Y, byte C) {
  lcd.setCursor(X, Y);
  lcd.write(C);
}


/*
 * Coolers switching
 */
void Cool1_on() {
  digitalWrite(Cooler1Pin, HIGH);
  LCDChar(0, 1, 6);
}
void Cool1_off() {
  digitalWrite(Cooler1Pin, LOW);
  LCDChar(0, 1, 5);
}

void Cool2_on() {
  digitalWrite(Cooler2Pin, HIGH);
  LCDChar(0, 2, 6);
}
void Cool2_off() {
  digitalWrite(Cooler2Pin, LOW);
  LCDChar(0, 2, 5);
}



void editTemp(int address, float min, float max) {
  float   temperature = word(er_byte(address), er_byte(address + 1)) / 16.0;
  boolean editLoop = true;

  Prompt(P2_clear);
  lcd.setCursor(1, 2);
#if langNL == true
  lcd.print(F("Temperatuur "));
#else
  lcd.print(F("Temperature "));
#endif
  Prompt(P3_QQxO);

  while (editLoop) {

    ReadButton(Direction, Timer);
    lcd.setCursor(13, 2);
    if (temperature < 10.0)
      lcd.write(32);
    lcd.print(temperature, 2);
    lcd.write((byte)0);
    Set(temperature, max, min, 0.25, Timer, Direction);

    if (btn_Press(ButtonEnterPin, 50)) {
      editLoop = false;
    }
  }

  int w_stagetempSet = word(temperature * 16);
  ew_byte(address, highByte(w_stagetempSet));
  ew_byte(address + 1, lowByte(w_stagetempSet));
}


void setupBox1(void) {
  boolean editLoop = true;
  byte    tsave = er_byte(EM_Box1_Run);
  
  Prompt(P2_clear);
  lcd.setCursor(1, 2);
#if langNL == true
  lcd.print(F("Gebruik box "));
#else
  lcd.print(F("Use this box"));
#endif
  Prompt(P3_xxTO);
  
  while (editLoop) {
    lcd.setCursor(13, 2);
#if langNL == true
    (tsave) ? lcd.print(F("    Ja")) : lcd.print(F("   Nee"));
#else
    (tsave) ? lcd.print(F("   Yes")) : lcd.print(F("    No"));
#endif
    if (btn_Press(ButtonStartPin, 50)) {
      if (tsave)
        tsave = 0;
      else
        tsave = 1;
    }
    if (btn_Press(ButtonEnterPin, 50)) {
      editLoop = false;
    }
  }
  ew_byte(EM_Box1_Run, tsave);
  Box1_on = tsave;
  
  if (tsave) {
    editTemp(EM_Box1_Temp, 1.0, 20.0);
    SetPoint_1 = word(er_byte(EM_Box1_Temp), er_byte(EM_Box1_Temp + 1)) / 16.0;
  }
}


void setupBox2(void) {
  boolean editLoop = true;
  byte    tsave = er_byte(EM_Box2_Run);
  
  Prompt(P2_clear);
  lcd.setCursor(1, 2);
#if langNL == true
  lcd.print(F("Gebruik box "));
#else
  lcd.print(F("Use this box"));
#endif
  Prompt(P3_xxTO);
  
  while (editLoop) {
    lcd.setCursor(13, 2);
#if langNL == true
    (tsave) ? lcd.print(F("    Ja")) : lcd.print(F("   Nee"));
#else
    (tsave) ? lcd.print(F("   Yes")) : lcd.print(F("    No"));
#endif
    if (btn_Press(ButtonStartPin, 50)) {
      if (tsave)
        tsave = 0;
      else
        tsave = 1;
    }
    if (btn_Press(ButtonEnterPin, 50)) {
      editLoop = false;
    }
  }
  ew_byte(EM_Box2_Run, tsave);
  Box2_on = tsave;
  
  if (tsave) {
    editTemp(EM_Box2_Temp, 1.0, 20.0);
    SetPoint_2 = word(er_byte(EM_Box2_Temp), er_byte(EM_Box2_Temp + 1)) / 16.0;
  }
}



void setup_mode(void) {
  byte setupMenu = 0;
  
  Cool1_off();
  Cool2_off();
  LCDChar(0, 1, 32);
  LCDChar(0, 2, 32);
  
  while (true) {
    Prompt(P0_setup);
    Prompt(P2_clear);
    lcd.setCursor(1, 1);
    
    switch (setupMenu) {
      case 0:
#if langNL == true
        lcd.print(F("  Koeler 1 setup  "));
#else
        lcd.print(F("  Cooler 1 setup  "));
#endif
        Prompt(P3_xGQO);
        if (btn_Press(ButtonDownPin, 50))
          setupMenu = 1;
        if (btn_Press(ButtonEnterPin, 50))
          setupBox1();
        break;
        
      case 1:
#if langNL == true
        lcd.print(F("  Koeler 2 setup  "));
#else
        lcd.print(F("  Cooler 2 setup  "));
#endif
        Prompt(P3_SxQO);
        if (btn_Press(ButtonUpPin, 50))
          setupMenu = 0;
        if (btn_Press(ButtonEnterPin, 50))
          setupBox2();
        break;
    }
    if (btn_Press(ButtonStartPin, 50)) {
      lcd.clear();
      return;
    }
  }
}




void setup() {
#if (DebugProcess == true || DebugReadWrite == true || DebugErrors == true)
  Serial.begin(115200);
#endif

  pinMode (Cooler1Pin, OUTPUT);
  pinMode (Cooler2Pin, OUTPUT);
  pinMode (BuzzerPin,  OUTPUT);
  pinMode (ButtonUpPin,    INPUT_PULLUP);
  pinMode (ButtonDownPin,  INPUT_PULLUP);
  pinMode (ButtonStartPin, INPUT_PULLUP);
  pinMode (ButtonEnterPin, INPUT_PULLUP);
  digitalWrite (Cooler1Pin, LOW);
  digitalWrite (Cooler2Pin, LOW);
  digitalWrite (BuzzerPin,  LOW);

#if FakeCooling == true
  FakeHeatLastInMS =
#endif
    gSystemStartTime = gCurrentTimeInMS = millis();

  lcd.begin(20, 4);
  Buzzer(1, 50);

  // write custom symbol to LCD
  lcd.createChar(0, degC);         // Celcius
  lcd.createChar(2, SP_Symbol);    // Set Point
  lcd.createChar(5, CoolONOFF);    // Cool On-OFF
  lcd.createChar(6, RevCoolONOFF); // Cool On-OFF
  lcd.createChar(7, Language);     // Language Symbol
  
  TimeSpent = 0;
  _TimeStart = millis();
  
  if ((er_byte(EM_Box1_Run) == 255) && (er_byte(EM_Box2_Run) == 255)) {
    /*
     * Never used before, set defaults
     */
     ew_byte(EM_Box1_Run, 1);
     ew_byte(EM_Box2_Run, 1);
     ew_byte(EM_Box1_Temp, 0);
     ew_byte(EM_Box1_Temp + 1, 160);
     ew_byte(EM_Box2_Temp, 0);
     ew_byte(EM_Box2_Temp + 1, 160);
  }
  Box1_on = er_byte(EM_Box1_Run);
  Box2_on = er_byte(EM_Box2_Run);
  SetPoint_1 = word(er_byte(EM_Box1_Temp), er_byte(EM_Box1_Temp + 1)) / 16.0;
  SetPoint_2 = word(er_byte(EM_Box2_Temp), er_byte(EM_Box2_Temp + 1)) / 16.0;
}



void loop() {

  switch (mainMenu) {

    case 3:
      setup_mode();
      mainMenu = 0;
      break;

    default:
      Prompt(P0_banner);
      if (Box1_on)
        Prompt(P1_status);
      if (Box2_on)
        Prompt(P2_status);
      Prompt(P3_xxxS);
      Coolers();

      if (btn_Press(ButtonEnterPin, 500))
        mainMenu = 3;
#if DebugErrors == true
      // "Secret" counters reset
      if (btn_Press(ButtonUpPin, 1000)) {
        lcd.clear();
        byte x = 1;
        byte y = 0;
        for (byte i = 0; i < 9; i++) {
          lcd.setCursor(x, y);
          lcd.print(er_byte(EM_ErrorNo(i)));
          if (x == 1)
            x = 7;
          else if (x == 7)
            x = 14;
          else if (x == 14) {
            x = 1;
            y++;
          }
        }
        Prompt(P3_erase);
        Buzzer(1, 100);
        while (true) {
          if (btn_Press(ButtonEnterPin, 50))
            break;
          if (btn_Press(ButtonStartPin, 50)) {
            for (byte i = 0; i < 10; i++)
              ew_byte(EM_ErrorNo(i), 0);
            break;
          }
        }
        lcd.clear();
      }
#endif
      break;
  }

}

