#ifndef __LEDCONTROL_H_INCLUDED
#define __LEDCONTROL_H_INCLUDED

#include <Arduino.h>
#include "networking.h"

void ledLoop();
void ledSetup();
void setCurrentColors(uint8_t);
void setLedWhite(uint16_t w);
void setLedRgb(uint16_t r, uint16_t g, uint16_t b);
void fadeIn();
void fadeOut();
void switchOn();
void switchOff();
void setFadeInTime(uint16_t ms);
void setFadeOutTime(uint16_t ms);
//0 is off, 1 is on
void setOutofire(uint8_t on);
void triggerAutofire();
void setAutofire(uint8_t on);

#endif //__LEDCONTROL_H_INCLUDED

// vim:ai:cin:sts=2 sw=2 ft=cpp
