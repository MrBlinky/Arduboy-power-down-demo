#ifndef AUTOPOWERDOWN_H
#define AUTOPOWERDOWN_H

/*******************************************************************************
Usage:

Put autoPowerDownReset() in your setup()

Put autoPowerDown() in your (main) loop()

If you want to run some specific code before a power down, put the following 
code imediately before autoPowerDown()

if (autoShutdownPending())
{
  //Your code here
};

The default APD timer interval is 8 second and the default timeout value is
set to 4, resulting in a a power down when there is no button pressed for
32 seconds (4 * 8)

You can change the default interval and value by putting one of the following
defines before the include:

#define APD_INTERVAL WDTO_1S  //1 second interval
#define APD_INTERVAL WDTO_4S  //4 second interval
#define APD_INTERVAL WDTO_8S  //8 second interval (default)

To change the default value add the following define before the include also:

#define APD_VALUE n << 1 // where n = 1 to 127

Example:
if you want a timeout of 1 minute you'd add the following defines:

#define APD_INTERVAL WDTO_4S
#define APD_VALUE 15 << 1

if you want a timeout of 20 seconds you'd add the following defines:

#define APD_INTERVAL WDTO_4S
#define APD_VALUE 5 << 1

if you want a timeout of 15 seconds you'd add the following defines:

#define APD_INTERVAL WDTO_1S
#define APD_VALUE 15 << 1

Notes:

After you've compiled a sketch with the above defines added, changes to the
defines will not take effect until you close and reopen the sketch.

The Maximum power down time posible is 16 minutes and 56 seconds
After powering up the LEDs remain off.
*******************************************************************************/

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Arduboy2.h>

void autoPowerDownReset();

void autoPowerDown();

extern volatile uint8_t APD_timer; //bits 7-1: timer value, bit 0: power down flag

#ifndef APD_VALUE
  #define APD_VALUE 4 << 1
#endif
#ifndef APD_INTERVAL
  #define APD_INTERVAL WDTO_8S
#endif

#ifdef AB_DEVKIT
  #error "Arduboy DevKit is not supported (No wake up button)"
#endif

/*******************************************************************************
* WDT_interrupt_disable
* A shorter version of wdt_disable()
*******************************************************************************/

#define wdt_interrupt_disable()                                        \
  asm volatile (                                                       \
    "sts  %[WDTREG],  %[WDCE_WDE]  \n\t"                               \
    "sts  %[WDTREG],  __zero_reg__ \n\t"                               \
    :                                                                  \
    : [WDTREG]   "M" (_SFR_MEM_ADDR(WDTCSR)),                          \
      [WDCE_WDE] "r" ((uint8_t)(_BV(_WD_CHANGE_BIT) | _BV(WDE)))       \
  )

/*******************************************************************************
* WDT_interrupt_enable
* enables WDT interrupt and disables WDT reset.
*******************************************************************************/

#define wdt_interrupt_enable(interval)                                 \
  asm volatile (                                                       \
    "in   __tmp_reg__,  __SREG__  \n\t"                                \
    "cli                          \n\t"                                \
    "wdr                          \n\t"                                \
    "sts  %[WDTREG],  %[WDCE_WDE] \n\t"                                \
    "sts   %[WDTREG],  %[WDVALUE]  \n\t"                               \
    "out  __SREG__, __tmp_reg__   \n\t"                                \
    :                                                                  \
    : [WDTREG]   "M" (_SFR_MEM_ADDR(WDTCSR)),                          \
      [WDCE_WDE] "r" ((uint8_t)(_BV(_WD_CHANGE_BIT) | _BV(WDE))),      \
      [WDVALUE]  "r" ((uint8_t)((interval & 0x08 ? (1<<WDP3) : 0x00) | \
                     (1<<WDIF) | (1<<WDIE) | (interval & 0x07)))       \
  )

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

/*******************************************************************************
* autoPowerDownPending()
* Macro to test if a power down is pending
*******************************************************************************/

#define autoPowerDownPending() \
  (APD_Timeout & 1)

#endif //AUTOPOWERDOWN_H
