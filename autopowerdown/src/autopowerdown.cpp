#include <wiring.c>
#include "autopowerdown.h"

uint16_t APD_time; 

/*******************************************************************************
INT6 external interrupt service routine
Executed when the A-BUTTON is pressed
*******************************************************************************/

EMPTY_INTERRUPT (INT6_vect)
    
/*******************************************************************************
PCINT0 pinchange interrupt service routine
Executed when the B-BUTTON is pressed
*******************************************************************************/

EMPTY_INTERRUPT (PCINT0_vect)
    
/*******************************************************************************
autoPowerDownReset

resets auto power down millis to current millis
*******************************************************************************/

void autoPowerDownReset()
{
  uint16_t time;    
  asm volatile(
    "    in    __tmp_reg__, __SREG__    \n\t" //uint8_t oldSREG = SREG;
    "    cli                            \n\t" //cli();    
    "    lds   %A[time], %[millis1]     \n\t" //time = *(uint16_t *)(&millis + 1);
    "    lds   %B[time], %[millis2]     \n\t" 
    "    out   __SREG__, __tmp_reg__    \n\t" //SREG = oldSREG;    
    "    sts   %[APD_time0], %A[time]   \n\t" //APD_time = time    
    "    sts   %[APD_time1], %B[time]   \n\t" 
    : [time]      "=&r" (time)
    : [millis1]   ""    ((uint8_t*)(&timer0_millis) + 1),
      [millis2]   ""    ((uint8_t*)(&timer0_millis) + 2),
      [APD_time0] ""  ((uint8_t*)(&APD_time) + 0),
      [APD_time1] ""  ((uint8_t*)(&APD_time) + 1)
  );
}

/*******************************************************************************
autoPowerDown
*******************************************************************************/

void autoPowerDown(uint8_t timeout)
{
  uint16_t time;
  asm volatile(
    "    in    __tmp_reg__, __SREG__    \n\t" //uint8_t oldSREG = SREG;
    "    cli                            \n\t" //cli()
    "    lds   %A[time], %[millis1]     \n\t" //time = (uint16_t)(millis >>8)
    "    lds   %B[time], %[millis2]     \n\t" 
    "    out   __SREG__, __tmp_reg__    \n\t" //SREG = oldSREG
    "    sub   %A[time], %A[APD_time]   \n\t" //time -= APD_time
    "    sbc   %B[time], %B[APD_time]   \n\t" 
    "1:                                 \n\t" 
    : [time]     "=&r" (time)
    : [millis1]  ""    ((uint8_t*)(&timer0_millis)+1),
      [millis2]  ""    ((uint8_t*)(&timer0_millis)+2),
      [APD_time] "r"   (APD_time)
  );  
  if (time >= timeout * 16)
 //power down timeout has been reached, so power down
  {
    if (UHWCON & _BV(UVREGE)) // Test if USB is enabled
    {
      UDIEN = 0;            //disable USB interrupts left enabled after upload
      USBCON = _BV(FRZCLK); //disable VBUS transition interrupt, freeze USB clock for power savings
      UDCON  = 1 << DETACH; //disconnect from USB bus
    }
    all_LEDs_off();
    Arduboy2Core::displayOff();
    INT6_enable();               //enable A-button interrupt so Arduboy can wake up by pressing it.
    PCINT0_enable();             //enable B-button interrupt so Arduboy can wake up by pressing it.
    activatePowerDown();
    INT6_disable();              //disable A-button interrupt. No unneccesary interrupts wanted.
    PCINT0_disable();            //disable B-button interrupt. No unneccesary interrupts wanted.
    if (UHWCON & _BV(UVREGE)) // Test if USB is enabled
    {
      init();                      //restore USB support
      USBDevice.attach();
    }
    Arduboy2Core::displayOn();
    autoPowerDownReset();
  }
  if (Arduboy2Core::buttonsState()) autoPowerDownReset(); //reset power down time on any button press
}