#ifndef AUTOPOWERDOWN_H
#define AUTOPOWERDOWN_H

/*******************************************************************************
Usage:

Put the following line in your setup():

autoPowerDownReset();

Put one of the following lines in your (main) loop():

autoPowerDown();

For powering down after ~30.2 seconds of no activity.

autoPowerDown(uint16_t timeout);

For when you want a different power down time.
timeout = power down time in seconds / 256.

*******************************************************************************/

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Arduboy2.h>

void autoPowerDownReset();

uint16_t get_millis_div256();

void autoPowerDown(uint16_t timeout = 118); //~30,2 sec

extern uint16_t APD_time;

#ifdef AB_DEVKIT
  #error "Arduboy DevKit is not supported (No wake up button)"
#endif

/*******************************************************************************
 * INT6_enable
 * Enable INT6 interrupt on low level. (attachInterrupt)
 ******************************************************************************/

#define INT6_enable() \
  EICRB &= ~(ISC61 || ISC60); \
  EIMSK |= _BV(INT6);

/*******************************************************************************
 * INT6_disable
 * Disables INT6 interrupt (detachInterrupt)
 ******************************************************************************/

#define INT6_disable() \
  EIMSK &= ~_BV(INT6);

/*******************************************************************************
* activatePowerDown()
* Macro to activate power down with BOD disabled
*******************************************************************************/

#define activatePowerDown()            \
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); \
  cli();                               \
  sleep_enable();                      \
  sei();                               \
  sleep_cpu();                         \
  sleep_disable();                     \
  sei();

/*******************************************************************************
* all_LEDs_off()
* Macro to turn all LEDs off including PWM ones
*******************************************************************************/

#ifndef ARDUINO_AVR_PROMICRO
  #define all_LEDs_off()                                  \
    TCCR1A = 0; /* disable Arduboy RGB LED PWMs */        \
    Arduboy2::digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_OFF); \
    TXLED0; RXLED0; /* Rx Tx LEDs off */
#else
  #define all_LEDs_off()                                  \
    TCCR1A = 0; /* disable Pro Micro BLUE RED LED PWMs */ \
    TCCR0A = 0; /* disable Pro Micro GREEN LED PWM */     \
    Arduboy2::digitalWriteRGB(RGB_OFF, RGB_OFF, RGB_OFF); \
    TXLED0; RXLED0; /* Rx Tx LEDs off */
#endif

#endif //AUTOPOWERDOWN_H
