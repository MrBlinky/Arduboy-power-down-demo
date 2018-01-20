#ifndef POWERDOWN_H
#define POWERDOWN_H

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <Arduboy2.h>

#ifdef AB_DEVKIT
  #error "Arduboy DevKit is not supported (No wake up button)"
#endif

#define PD_INTERVAL_1SEC WDTO_1S
#define PD_INTERVAL_4SEC WDTO_4S

enum {
  ps_up,
  ps_down
};

struct powertime_t {
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint16_t days;
  }; 

#define wdt_interrupt_enable(interval)                                 \
  asm volatile (                                                       \
    "in   __tmp_reg__,  __SREG__  \n\t"                                \
    "cli                          \n\t"                                \
    "wdr                          \n\t"                                \
    "sts  %[WDTREG],  %[WDCE_WDE] \n\t"                                \
    "sts   %[WDTREG],  %[WDVALUE]  \n\t"                                \
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
  EIMSK &= _BV(INT6);

/*******************************************************************************
powerDownIO()
Macro to set all I/O to inputs except the OLED reset pin which is kept as a low
output. Only a pullup is kept on the A button for a stable powerup button.
*******************************************************************************/
 
#define powerDownIO()        \
  DDRB  = 0;                 \
  PORTB = 0;                 \
  DDRC  = 0;                 \
  PORTC = 0;                 \
  DDRD  = _BV(RST_BIT);      \
  PORTD = 0;                 \
  DDRE  = 0;                 \
  PORTE = _BV(A_BUTTON_BIT); \
  DDRF  = 0;                 \
  PORTF = 0;                 

/*******************************************************************************
activatePowerDown()
Macro to activate power down with BOD disabled
*******************************************************************************/
  
#define activatePowerDown()            \
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); \
  cli();                               \
  sleep_enable();                      \
  sei();                               \
  sleep_cpu();                         \
  sleep_disable();                     \
  sei();                               \
  
/******************************************************************************/

void autoPowerDownInit(uint8_t timeout = PD_INTERVAL_1SEC);
void setAutoPowerDownTimeout(uint16_t timeout);
void setAutoPowerUpTimeout(uint16_t timeout);
void powerDown();

extern volatile uint8_t  powerState;
extern volatile uint16_t powerDownTimeout;
extern volatile uint16_t powerUpTimeout;
extern volatile powertime_t powertime;

#endif //POWERDOWN_H


