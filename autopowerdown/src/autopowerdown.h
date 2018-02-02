#ifndef AUTOPOWERDOWN_H
#define AUTOPOWERDOWN_H

/*******************************************************************************
Usage:

Put the following line in your (main) loop() for default power down after
~32.8 seconds of no button activity. 

autoPowerDown();

For other power down timeouts use:

autoPowerDown(uint8_t timeout);

Where timeout = power down time in seconds / 4.096

*******************************************************************************/

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Arduboy2.h>

void autoPowerDownReset();

void autoPowerDown(uint8_t timeout = 8); //~32,8 sec

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

#ifndef AB_ALTERNATE_WIRING
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
