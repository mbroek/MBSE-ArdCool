#ifndef BUTTONS_H
#define BUTTONS_H


byte btn_Press (byte Button_press, int msTime) {
  if (digitalRead(Button_press) == 0) {
    delay (msTime);
    if (digitalRead(Button_press) == 0) {
      while (digitalRead(Button_press) == 0) {
      }
      return 1;
    }
  }
  return 0;
}


byte ReadButton(byte& Direction, unsigned long& Timer ) {
  boolean f_btnUp, f_btnDn;

  if (digitalRead (ButtonUpPin) == 0) {   // Up button is pressed
    if (Direction != DirectionUp)
      Timer = millis();         // Register time of new direction
    f_btnUp = true;
    Direction = DirectionUp;    // Register new direction
    delay(35);
  } else
    f_btnUp = false;

  if (digitalRead (ButtonDownPin) == 0) {   // Down button is pressed
    if (Direction != DirectionDown)
      Timer = millis();         // Register time of new direction
    f_btnDn = true;
    Direction = DirectionDown;  // Register new direction
    delay(35);
  } else
    f_btnDn = false;

  if (f_btnUp == false && f_btnDn == false)
    Direction = DirectionNone;  // No Up or Down direction.
}


float Set(float& Set, float Up, float Low, float Step, long Timer, byte Direction) {
  float step_size;
  int ButtonPressed;

  if (Set > Up)
    Set = Up;
  if (Set < Low)
    Set = Low;

  delay(50);
  if (Direction == DirectionUp) {
    ButtonPressed = digitalRead(ButtonUpPin);
  }
  if (Direction == DirectionDown) {
    ButtonPressed = digitalRead(ButtonDownPin);
  }

  if (ButtonPressed == 0 && Direction != DirectionNone) {
    if (((millis() - Timer) / 1000) >= 4)
      step_size = (Step * 20.0);
    else {
      if (((millis() - Timer) / 1000) >= 2)
        step_size = (Step * 5.0);
      else
        step_size = Step;
    }
    if (Direction == DirectionUp) {
      if (Set + step_size > Up)
        Set = Up;
      else
        Set += step_size;
    } else if (Direction == DirectionDown) {
      if (Set - step_size < Low)
        Set = Low;
      else
        Set -= step_size;
    }
  }
}

#endif
