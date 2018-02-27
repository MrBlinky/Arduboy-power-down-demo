#include <wiring.c>
#include "autopowerdown.h"

uint16_t APD_time; 

/*******************************************************************************
INT6 external interrupt service routine
Executed when the A-BUTTON is pressed
*******************************************************************************/

EMPTY_INTERRUPT (INT6_vect)
    
/*******************************************************************************
autoPowerDownReset

resets auto power down millis to current millis
*******************************************************************************/

void autoPowerDownReset()
{
#ifndef ARDUBOY_CORE    
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
#endif  
}

/*******************************************************************************
autoPowerDown
*******************************************************************************/

void autoPowerDown(uint8_t timeout)
{
#ifndef ARDUBOY_CORE
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
#else
 if (buttonsIdleTime() >= timeout)
#endif    
 //power down timeout has been reached, so power down
  {
  #ifndef ARDUBOY_NO_USB
    UDIEN = 0;            //disable USB interrupts left enabled after upload
    USBCON = _BV(FRZCLK); //disable VBUS transition interrupt, freeze USB clock for power savings
    UDCON  = 1 << DETACH; //disconnect from USB bus
  #endif
    all_LEDs_off();
    Arduboy2Core::displayOff();
    INT6_enable();               //enable A-button interrupt so Arduboy can wake up by pressing it.
    activatePowerDown();
    INT6_disable();              //disable A-button interrupt. No unneccesary interrupts wanted.
  #ifndef ARDUBOY_NO_USB
    init();                      //restore USB support
    USBDevice.attach(); 
  #endif
    Arduboy2Core::displayOn();
#ifndef ARDUBOY_CORE    
    autoPowerDownReset();
#else
    //auto power down reset is handled by Arduboy core    
#endif    
  }
#ifndef ARDUBOY_CORE    
  if (Arduboy2Core::buttonsState()) autoPowerDownReset(); //reset power down time on any button press
#endif  
}