#include <Arduboy2.h>
#include <Sprites.h>
#include "bitmap.h"
#include "src/powerdown.h"

Arduboy2 arduboy;

int16_t androtchi_x = (WIDTH - androtchi_width) / 2;
int16_t androtchi_y = (HEIGHT - androtchi_width) /2;
uint8_t androtchi_frame;         
               
void setup() 
{
  autoPowerDownInit();
  arduboy.begin();
  arduboy.initRandomSeed();
  arduboy.setFrameRate(4); //framerate < 4 not supported
}

void loop() 
{
  if (powerDownTimeout == 0) setAutoPowerDownTimeout(10); // power down after 10 secs if no buttons are pressed
  if (powerUpTimeout == 0) setAutoPowerUpTimeout(10);     //force power up after 10 seconds in power down mode. add // at the beginning of this line to disable this behaviour.

  if (arduboy.buttonsState()) setAutoPowerDownTimeout(10); //reset power down time on any button press
  if (arduboy.pressed(B_BUTTON)) powerDown();              // instand power down when B button is pressed

  if (!arduboy.nextFrame()) return; else delay(375); //additional delay to lower framerate to ~2 fps

  //display  days and time after powering on
  arduboy.clear();
  arduboy.print(powertime.days);
  arduboy.setCursor(WIDTH - 48,0);
  if (powertime.hours < 10) arduboy.print(0);
  arduboy.print(powertime.hours);
  arduboy.print(":");
  if (powertime.minutes < 10) arduboy.print(0);
  arduboy.print(powertime.minutes);
  arduboy.print(":");
  if (powertime.seconds < 10) arduboy.print(0);
  arduboy.print(powertime.seconds);

  //draw androtchi tamagotchi robot
  androtchi_frame = arduboy.frameCount & 1;
  Sprites::draw(androtchi_x, androtchi_y, androtchi_sprite_plus_mask, androtchi_frame, NULL, 0, SPRITE_PLUS_MASK);
  if (random() & 1)
    {
      androtchi_x += 8;
      if (androtchi_x > (WIDTH - androtchi_width)) androtchi_x = WIDTH - androtchi_width;
    }
    else
    {
      androtchi_x-= 8;
      if (androtchi_x <0) androtchi_x= 0;
    }
    
  arduboy.display();
}
