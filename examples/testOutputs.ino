#include "flprogTca9555.h"

FLProgTca9555 lamp1(0x20, 0);
FLProgTca9555 lamp2(0x21, 0);
FLProgTca9555 lamp3(0x22, 0);
FLProgTca9555 lamp4(0x23, 0);
FLProgTca9555 lamp5(0x24, 0);
FLProgTca9555 lamp6(0x25, 0);
uint32_t timerStartTime;
uint8_t workPin = 0;

void setup()
{
  RT_HW_Base.i2cSetPins(20, 21, 0);
  for (uint8_t i = 0; i < 16; i++)
  {
    lamp1.pinMode(i, OUTPUT);
    lamp2.pinMode(i, OUTPUT);
    lamp3.pinMode(i, OUTPUT);
    lamp4.pinMode(i, OUTPUT);
    lamp5.pinMode(i, OUTPUT);
    lamp6.pinMode(i, OUTPUT);
  }
  timerStartTime = millis();
}

void loop()
{
  lamp1.pool();
  lamp2.pool();
  lamp3.pool();
  lamp4.pool();
  lamp5.pool();
  lamp6.pool();
  if (flprog::isTimer(timerStartTime, 1000))
  {
    timerStartTime = millis();
    lamp1.write(workPin, 0);
    lamp2.write(workPin, 0);
    lamp3.write(workPin, 0);
    lamp4.write(workPin, 0);
    lamp5.write(workPin, 0);
    lamp6.write(workPin, 0);
    workPin++;
    if (workPin > 15)
    {
      workPin = 0;
    }
    lamp1.write(workPin, 1);
    lamp2.write(workPin, 1);
    lamp3.write(workPin, 1);
    lamp4.write(workPin, 1);
    lamp5.write(workPin, 1);
    lamp6.write(workPin, 1);
  }
}