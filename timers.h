#ifndef	TIMERS_H
#define	TIMERS_H


unsigned long TimeSpent;
unsigned long _TimeStart;


/*
 * Should be called as much as possible
 */
void TimerRun() {

  gCurrentTimeInMS = millis();
  if ((gCurrentTimeInMS - _TimeStart) > 977) {  // 1 millis() is 1,024 mSec.
    _TimeStart += (gCurrentTimeInMS - _TimeStart);
    TimeSpent++;
    NewSecond = true;
  }
}


#endif
