#include "powerdown.h"

volatile uint8_t  powerState;
volatile uint32_t powerTime;
volatile uint16_t powerDownTimeout;
volatile uint16_t powerUpTimeout;
volatile powertime_t powertime;

/*******************************************************************************
WDT interrupt service routine
Used for a clock, auto power down and auto power Up feature.

Note:
Arduino USB CDC class will disable and reset WDT. Put a copy of CDC.cpp in your
sketch folder, edit it, find CDC_Setup and put // in front of wdt_disable() and
in //wdt_reset()
*******************************************************************************/

ISR (WDT_vect)
{
  WDTCSR |= (1 << WDIE); //WDIE was cleared by hardware, re-enable for next WDT interrupt.
  //* test */ Arduboy2::digitalWriteRGB(RGB_OFF, RGB_ON, RGB_ON); delay(100); Arduboy2::digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_OFF); //heartbeat
  powertime.seconds ++; if (powertime.seconds >= 60)
  { 
    powertime.seconds = 0; powertime.minutes ++; if (powertime.minutes >= 60)
    {
      powertime.minutes = 0; powertime.hours ++; if (powertime.hours >= 24)
      {
        powertime.hours = 0;  powertime.days ++;
      }
    }
  }
  if (powerState == ps_up && powerDownTimeout > 0)
  {
    powerDownTimeout --;
    if (powerDownTimeout == 0) powerDown();
  }
  if (powerState == ps_down && powerUpTimeout > 0)
  {
    powerUpTimeout --;
    if (powerUpTimeout == 0) powerState = ps_up;
  }
}

/*******************************************************************************
INT6 external interrupt service routine
Generated by pressing the A BUTTON. 
*******************************************************************************/

ISR (INT6_vect)
{
  powerState = ps_up;
}

/*******************************************************************************
autoPowerDownInit

Enables WDT interrupt for clock, auto power down and auto power on features.
Default timeout is 1 second.
*******************************************************************************/

void autoPowerDownInit(uint8_t interval) 
{
  wdt_interrupt_enable(interval);
}

/*******************************************************************************
setAutoPowerUpTimeout

Sets auto power down time in WDS interval units.
*******************************************************************************/

void setAutoPowerDownTimeout(uint16_t timeout)
{
  powerDownTimeout = timeout;
}

/*******************************************************************************
setAutoPowerUpTimeout

Sets auto power up time in WDS interval units.
*******************************************************************************/

void setAutoPowerUpTimeout(uint16_t timeout)
{
  powerUpTimeout = timeout;
}

/*******************************************************************************
powerDown

Puts Arduboy in low power mode by turning OLED display  off and MCU in low power
mode. Only the A button and auto power up feature can wake arduboy up again.
*******************************************************************************/

void powerDown() 
{
  powerDownTimeout = 0;
#ifndef ARDUBOY_NO_USB
  UDCON |= 1 << DETACH; //detach USB
#endif
  Arduboy2::digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_OFF); TXLED0; RXLED0; //Turn all LEDs off
  Arduboy2Core::displayOff();
  //powerDownIO();
  INT6_enable();
  powerState = ps_down;
  do 
  {
    activatePowerDown();
  }
  while (powerState == ps_down);
  INT6_disable();
#ifndef ARDUBOY_NO_USB
  init(); // restore USB support
  USBDevice.attach(); 
#endif
  //Arduboy2Core::boot();// initialize IO, SPI and OLED display
  Arduboy2Core::displayOn();  
}