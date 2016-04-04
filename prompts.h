#ifndef  PROMPTS_H
#define  PROMPTS_H



#define  P0_clear        100
#define  P0_banner       101
#define  P0_setup        102

#define  P1_clear        200
#define  P1_status       201

#define  P2_clear        300
#define  P2_status       301

#define  P3_clear        400
#define  P3_xxxS         401
#define  P3_xGQO         402
#define  P3_SxQO         403
#define  P3_xxTO         404
#define  P3_QQxO         405
#define  P3_xxxO         406
#if DebugErrors == true
#define  P3_erase        420
#endif


void Buzzer(byte NumBeep, int Period) {
#if Silent != true
  for (byte i = 0; i < NumBeep; i++) {
    digitalWrite (BuzzerPin, HIGH);
    delay (Period);
    digitalWrite(BuzzerPin, LOW);
    delay(75);
  }
#endif
}


void LCDSpace (byte Num) {
  for (byte i = 0; i < Num; i++)
    lcd.write(32);
}


void FormatNumber(float Numero, int Offset) {
  byte Space = 0;
  if (Numero <= -100.0)
    Space = 0;
  if (Numero <=  -10.0 && Numero > -100.0)
    Space = 1;
  if (Numero <     0.0 && Numero >  -10.0)
    Space = 2;
  if (Numero <    10.0 && Numero >=   0.0)
    Space = 3;
  if (Numero <   100.0 && Numero >=  10.0)
    Space = 2;
  if (Numero <  1000.0 && Numero >= 100.0)
    Space = 1;
  if (Numero >= 1000.0)
    Space = 0;

  LCDSpace(Space + Offset);
}


/*
 * LCD messages
 */
void Prompt(int Pmpt) {
  float Temp;

  if (Pmpt == 0)
    return;

  if ((Pmpt >= 100) && (Pmpt < 200)) {
    lcd.setCursor(0, 0);
  } else if ((Pmpt >= 200) && (Pmpt < 300)) {
    lcd.setCursor(1, 1);
  } else if ((Pmpt >= 300) && (Pmpt < 400)) {
    lcd.setCursor(1, 2);
  } else if ((Pmpt >= 400) && (Pmpt < 500)) {
    lcd.setCursor(0, 3);
  }

  switch (Pmpt) {
    case P0_clear:
      LCDSpace(20);
      return;
    case P0_banner:
      lcd.print(F("MBSE ArdCool "VERSION" \x07"));
      return;
    case P0_setup:
#if langNL == true
      lcd.print(F("    INSTELLINGEN    "));
#else
      lcd.print(F("     SETUP MODE     "));
#endif
      return;


    case P1_clear:
      LCDSpace(18);
      return;
    case P1_status:
      Temp = Temp_1;
      FormatNumber(Temp, -1);
      lcd.print(Temp, 2);
      lcd.write((byte)0);
      lcd.write(32);
      lcd.setCursor(11, 1);
      Temp = SetPoint_1;
      FormatNumber(Temp, -1);
      lcd.print(Temp, 2);
      lcd.write(2);
      return;

    case P2_clear:
      LCDSpace(18);
      return;
    case P2_status:
      Temp = Temp_2;
      FormatNumber(Temp, -1);
      lcd.print(Temp, 2);
      lcd.write((byte)0);
      lcd.write(32);
      lcd.setCursor(11, 2);
      Temp = SetPoint_2;
      FormatNumber(Temp, -1);
      lcd.print(Temp, 2);
      lcd.write(2);
      lcd.write(32);
      return;

    case P3_clear:
      LCDSpace(20);
      return;
    case P3_xxxS:
#if langNL == true
      lcd.print(F(" --   --   --  SETUP"));
#else
      lcd.print(F(" --   --   --  SETUP"));
#endif
      return;
    case P3_xGQO:
#if langNL == true
      lcd.print(F(" --  Neer  Terug  Ok"));
#else
      lcd.print(F(" --  Down  Quit   Ok"));
#endif
      return;
    case P3_SxQO:
#if langNL == true
      lcd.print(F(" Op   --   Terug  Ok"));
#else
      lcd.print(F(" Up   --   Quit   Ok"));
#endif
      return;
    case P3_xxTO:
#if langNL == true
      lcd.print(F("            Uit  Ok "));
#else
      lcd.print(F("            Aan  Ok "));
#endif
      return;
    case P3_QQxO:
#if langNL == true
      lcd.print(F(" Op  Neer        Ok "));
#else
      lcd.print(F(" Up  Down        Ok "));
#endif
      return;
    case P3_xxxO:
#if langNL == true
      lcd.print(F("                 Ok "));
#else
      lcd.print(F("                 Ok "));
#endif
      return;
#if DebugErrors == true
    case P3_erase:
      lcd.print(F("Clear all:  Yes  No "));
      return;
#endif
 
  }
}



#endif
