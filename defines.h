#ifndef DEFINES_H
#define DEFINES_H

#define  VERSION        "0.0.2"

/*
   EEPROM MAP
*/

#define  EM_ErrorBase        900
#define  EM_ErrorNo(i)       ((EM_ErrorBase)+(i))

#define  EM_Box1_Run         921
#define  EM_Box1_Temp        922
#define  EM_Box2_Run         924
#define  EM_Box2_Temp        925


#define DirectionNone         0
#define DirectionUp           1
#define DirectionDown         2


byte degC[8]         = {B01000, B10100, B01000, B00111, B01000, B01000, B01000, B00111};  // [0] degree c sybmol
byte SP_Symbol[8]    = {B11100, B10000, B11100, B00111, B11101, B00111, B00100, B00100};  // [2] SP Symbol
byte CoolONOFF[8]    = {B00000, B01110, B01010, B01000, B01000, B01010, B01110, B00000};  // [5] COOL symbol
byte RevCoolONOFF[8] = {B11111, B10001, B10101, B10111, B10111, B10101, B10001, B11111};  // [6] reverse COOL symbol
#if langNL == true
byte Language[8]     = {B00001, B00001, B11111, B00000, B11111, B00100, B01000, B11111};  // [7] NL symbol
#else
byte Language[8]     = {B11111, B00100, B01000, B11111, B00000, B10001, B10101, B11111};  // [7] EN symbol
#endif

#endif

