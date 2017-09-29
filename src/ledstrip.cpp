#include "ledstrip.h"
#include "config.h"

#define STATE_ON 1
#define STATE_OFF 2
#define STATE_ON_AUTOFIRE 3
#define STATE_OFF_AUTOFIRE 4
#define STATE_FADE_OUT 5
#define STATE_FADE_IN 6
#define STATE_WAKEUP_LIGHT_FADE_IN 7
//we are currently switched on, waiting for an autofire event and then switching off
#define STATE_WAKEUP_LIGHT_WAIT_FOR_MOTION 8

//defaults
uint16_t fadeInTime = 10000;
uint16_t fadeOutTime = 10000;
uint16_t white = 1000;
uint16_t red = 0;
uint16_t green = 0;
uint16_t blue = 0;
uint32_t autofireOnTime = 30000;

//state
uint8_t currentState = STATE_OFF_AUTOFIRE;

uint8_t  isAutofire = 0;
uint32_t autofireTriggered = 0;
uint32_t switchOffTime = 0;

uint32_t fadeInStartTime = 0;
uint32_t fadeOutStartTime = 0;
uint16_t currentWhite = 1000;
uint16_t currentRed = 0;
uint16_t currentGreen = 0;
uint16_t currentBlue = 0;

const uint16_t MAX_VALUE = 1023;
const uint16_t PWM_STEPS = 256;

uint16_t getFadedValue(uint32_t startTime, uint32_t now, uint16_t duration, uint16_t value, uint8_t fadeIn);
void setColors(uint16_t white, uint16_t red, uint16_t green, uint16_t blue, uint8_t publish);

void ledSetup()
{
  //LED pins
  pinMode(LED_PIN_W, OUTPUT);
  pinMode(LED_PIN_R, OUTPUT);
  pinMode(LED_PIN_G, OUTPUT);
  pinMode(LED_PIN_B, OUTPUT);
}

void cycleRgbw()
{
  if (currentGreen == 0 && currentBlue == 0 && currentRed < 1020)
  {
    currentRed++;
    currentGreen = 0;
    currentBlue = 0;
  }
  else if (currentBlue == 0 && currentGreen < 1020)
  {
    currentGreen++;
    currentBlue = 0;
    currentRed = 0;
  }
  else if (currentBlue < 1020)
  {
    currentBlue++;
    currentGreen = 0;
    currentRed = 0;
  }
  else
  {
    currentRed = 0;
    currentGreen = 0;
    currentBlue = 0;
  }
  setCurrentColors(0);
  delay(10);
}

void ledLoop()
{
  uint32_t currentMillis = millis();
  if(isAutofire && autofireTriggered)
  {
    setLedWhite(1000);
    setCurrentColors(1);
    switchOffTime = currentMillis + autofireOnTime;
    autofireTriggered = 0;
  }
  if(switchOffTime && currentMillis > switchOffTime)
  {
    //switchOff();
    fadeOut();
    switchOffTime = 0;
  }

  switch(currentState)
  {
    uint16_t fadedWhite;
    case STATE_ON:
      break;
    case STATE_OFF:
      break;
    case STATE_FADE_IN:
      fadedWhite = getFadedValue(fadeInStartTime, currentMillis, fadeInTime, currentWhite, 1);
      setColors(fadedWhite, currentRed, currentGreen, currentBlue, 0);
      if(fadedWhite == MAX_VALUE)
      {
        setColors(fadedWhite, currentRed, currentGreen, currentBlue, 1);
        currentState = STATE_ON;
      }
      break;
    case STATE_FADE_OUT:
      fadedWhite = getFadedValue(fadeOutStartTime, currentMillis, fadeOutTime, currentWhite, 0);
      setColors(fadedWhite, currentRed, currentGreen, currentBlue, 0);
      if(fadedWhite == 0) 
      {
        switchOff();
      }
      break;
    //case STATE_AUTOFIRE_ON:
  }
  //cycleRgbw();
  return;

  //FIXME: state machine
/*
 * Light autofire wait next         Name
 * On?   fired?
 * 1     0      0    LIGHT_ON     //LIGHT_ON
 * 0     0      0    LIGHT_OFF    //LIGHT_OFF
 * 0     1      10   ON_AUTOFIRE  //OFF_AUTOFIRE
 */

  /*
  switch(currentState)
  {
    case STATE_ON:
      break;
    case STATE_OFF:
      break;
  }
  uint32_t currentMillis = millis();
  //FIXME: over/underflow?
  if(switchOffTime && currentMillis > switchOffTime)
  {
    fadeOut();
    switchOffTime = 0;
  }
  if(isAutofire && autofireTriggered)
  {
    switchOn();
    switchOffTime = currentMillis + autofireOnTime;
  }

  if(currentFadeOutCounter)
  {
    //FIXME
    currentWhite = currentWhite -1 ;
    currentRed--;
    currentGreen--;
    currentBlue--;
    //fade out
    if(currentFadeOutCounter == 1)
    {
      switchOff();
      currentFadeOutCounter = 0;
    }
  }
  if(currentFadeInCounter)
  {
    if(currentFadeInCounter == 1)
    {
      switchOn();
      currentFadeInCounter = 0;
    }
    //fade out
  }
  */
} 

void setCurrentColors(uint8_t publish)
{
  setColors(currentWhite, currentRed, currentGreen, currentBlue, publish);
}

/*
 * sets the given colors on the output without changing any variables
 */
void setColors(uint16_t white, uint16_t red, uint16_t green, uint16_t blue, uint8_t publish)
{
  analogWrite(LED_PIN_W, white);
  /*
  analogWrite(LED_PIN_R, red);
  analogWrite(LED_PIN_G, green);
  analogWrite(LED_PIN_B, blue);
  */

  if(publish)
  {
    snprintf(buf, sizeof(buf), "%i", white);
    mqttClient.publish(OUT_TOPIC_LED_WHITE, buf);

    snprintf(buf, sizeof(buf), "%i,%i,%i", red, green, blue);
    mqttClient.publish(OUT_TOPIC_LED_RGB, buf);

    //publish status: if anything is switched on
    if(white > 0 || red > 0 || blue > 0 || green > 0) mqttClient.publish(OUT_TOPIC_LED_SWITCH, "1");
    else mqttClient.publish(OUT_TOPIC_LED_SWITCH, "0");
  }
}

void setLedWhite(uint16_t w)
{
  currentWhite = w <= MAX_VALUE ? w : MAX_VALUE;
}

void setLedRgb(uint16_t r, uint16_t g, uint16_t b)
{
  currentRed = r <= MAX_VALUE ? r : MAX_VALUE;
  currentGreen = g <= MAX_VALUE ? g : MAX_VALUE;
  currentBlue = b <= MAX_VALUE ? b : MAX_VALUE;
}

void switchOn()
{
  setLedWhite(white);
  setLedRgb(red, green, blue);
  setCurrentColors(1);
  currentState = STATE_ON;
}
void switchOff()
{
  setLedWhite(0);
  setLedRgb(0, 0, 0);
  setCurrentColors(1);
  currentState = STATE_OFF;
}
void fadeIn()
{
  fadeInStartTime = millis();
  currentState = STATE_FADE_IN;
} 
void fadeOut()
{
  fadeOutStartTime = millis();
  currentState = STATE_FADE_OUT;
} 
void setFadeInTime(uint16_t ms)
{
  fadeInTime = ms;
} 
void setFadeOutTime(uint16_t ms)
{
  fadeOutTime = ms;
} 
//0 is off, 1 is on
void setAutofire(uint8_t on)
{
  isAutofire = on;
}
void triggerAutofire()
{
  autofireTriggered = millis();
}

/*
 * returns the faded value for the given arguments
 * startTime: the time fading started in ms
 * now: current time in ms
 * duration: how long to fade in/out in ms
 * fadeIn: 1 if fade in, 0 if fade out
 */
uint16_t getFadedValue(uint32_t startTime, uint32_t now, uint16_t duration, uint16_t value, uint8_t fadeIn)
{
    uint32_t stdu = startTime + duration - now;
    if(stdu > duration && fadeIn) 
    {
      currentState = STATE_ON;
      return value;
    }
    else if(stdu > duration)
    {
      return 0;
    }
    float percentage = stdu/(float)duration;
    if(fadeIn) percentage = 1 - percentage;
    uint16_t val = pow(2, log2(value) * (PWM_STEPS *percentage + 1) / PWM_STEPS);
    val = val <= value ? val : value;
    return val;
}

// vim:ai:cin:sts=2 sw=2 ft=cpp
