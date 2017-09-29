/*
 * pinout: see config.h
 *
 */
#include "config.h"
#include <Arduino.h>
#include "networking.h"
#include "temps.h"
#include "ledstrip.h"
#include <ESP8266httpUpdate.h>


volatile uint8_t motionChanged  = 0;
volatile uint8_t motion2Changed  = 0;
volatile uint8_t motion = 0;
volatile uint8_t motion2 = 0;

uint16_t currentLight = 0;

static uint32_t lastStatsTime = 0;
uint16_t loopSpeedCounter = 0;

static uint16_t bytesToRgbResult[3];

void publishStats();
uint16_t readLight();
void motionIsr();
#ifdef MOTION_PIN2
void motion2Isr();
#endif
void mqttCallback(char* topic, byte* payload, unsigned int length);
void bytesToRgb(byte* inputBytes, unsigned int length);

uint16_t bytesToInt(byte* inputBytes, unsigned int length);
uint16_t ledBrightness = 500;

void setup() {
  delay( 3000 ); // power-up safety delay
  //SPI.begin();

  networkingSetup(mqttCallback);

  lastStatsTime = millis();

  tempsSetup();

  ledSetup();

  //switch off LED, just testing
  digitalWrite(D4, HIGH);
  //LDR pin
  pinMode(LDR_PIN, INPUT);

  //motion pins
  pinMode(MOTION_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTION_PIN), motionIsr, CHANGE);
#ifdef MOTION_PIN2
  pinMode(MOTION_PIN2, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTION_PIN2), motion2Isr, CHANGE);
#endif
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, IN_TOPIC_LED_WHITE) == 0) {
    uint16_t b = bytesToInt(payload, length);
    setLedWhite(b);
  }
  if (strcmp(topic, IN_TOPIC_LED_RGB) == 0) {
    bytesToRgb(payload, length);
    setLedRgb(bytesToRgbResult[0], bytesToRgbResult[1], bytesToRgbResult[2]);
  }
  else if (strcmp(topic, IN_TOPIC_LED_FADEOUT_TIME) == 0) {
    uint16_t b = bytesToInt(payload, length);
    setFadeOutTime(b);
  }
  else if (strcmp(topic, IN_TOPIC_LED_FADEIN_TIME) == 0) {
    uint16_t b = bytesToInt(payload, length);
    setFadeInTime(b);
  }
  else if (strcmp(topic, IN_TOPIC_LED_FADEIN) == 0) {
    fadeIn();
  }
  else if (strcmp(topic, IN_TOPIC_LED_FADEOUT) == 0) {
    fadeOut();
  }
  else if (strcmp(topic, IN_TOPIC_LED_SWITCH) == 0) {
    uint16_t b = bytesToInt(payload, length);

    if(b) switchOn();
    else switchOff();
  }
  else if (strcmp(topic, IN_TOPIC_LED_AUTOFIRE) == 0) {
    uint16_t a = bytesToInt(payload, length);

    setAutofire(a);
  }
  else if (strcmp(topic, IN_TOPIC_OTA) == 0) {

    char buffer[100];
    if(length < 100) 
    {
      snprintf(buffer, length + 1, "%s", payload );
#ifdef MY_SRC_REV
      ESPhttpUpdate.update(buffer, MY_SRC_REV);
#else
      ESPhttpUpdate.update(buffer, "1");
#endif
    }
  }
}

void loop()
{
  loopSpeedCounter++;

  networkingLoop();

  ledLoop();
  yield();

  tempsLoop();
  yield();

  uint32_t curmillis = millis();

  if((curmillis - lastStatsTime) > STATS_INTERVAL)
  {
    lastStatsTime = curmillis;
    publishStats();
    yield();

    currentLight = readLight();
  }

  if(motionChanged)
  {
    motionChanged = 0;
    //only trigger if motion == 1: motion has been detected, do not trigger if motion sensor switches off
    if(motion && currentLight > 500) triggerAutofire();
    snprintf(buf, sizeof(buf), "%i", motion );
    mqttClient.publish(OUT_TOPIC_MOTION, buf );
  }
#ifdef MOTION_PIN2
  if(motion2Changed)
  {
    motion2Changed = 0;
    //only trigger if motion == 1: motion has been detected, do not trigger if motion sensor switches off
    if(motion2) triggerAutofire();
    snprintf(buf, sizeof(buf), "%i", motion2 );
    mqttClient.publish(OUT_TOPIC_MOTION "2", buf );
  }
#endif
}

uint16_t readLight()
{
  //bright light: 0
  //darkness: 560
  uint16_t light = analogRead(LDR_PIN);
  snprintf(buf, sizeof(buf), "%i", light );
  mqttClient.publish(OUT_TOPIC_LIGHT, buf );
  return light;
}

void publishStats()
{
  mqttClient.publish(OUT_TOPIC_STATS_IP, WiFi.localIP().toString().c_str() );

  snprintf(buf, sizeof(buf), "%i", loopSpeedCounter);
  mqttClient.publish(OUT_TOPIC_STATS_LOOPSPEED, buf );
  loopSpeedCounter = 0;

#ifdef MY_SRC_REV
  mqttClient.publish(OUT_TOPIC_STATS_GITREV, MY_SRC_REV );
#endif
}

void motionIsr()
{
  motion = digitalRead(MOTION_PIN);
  motionChanged = 1;
}

#ifdef MOTION_PIN2
void motion2Isr()
{
  motion2 = digitalRead(MOTION_PIN2);
  motion2Changed = 1;
}
#endif

uint16_t bytesToInt(byte* inputBytes, unsigned int length) 
{
  char input[length + 1];
  for (int i = 0; i < length; i++) {
    input[i] = (char) inputBytes[i];
  }
  input[length] = 0;
  return atoi(input);
}

void bytesToRgb(byte* inputBytes, unsigned int length)
{
  char input[length + 1];
  for (int i = 0; i < length; i++) {
    input[i] = (char) inputBytes[i];
  }
  // Add the final 0 to end the C string
  input[length] = 0;

  char* command = strtok(input, ", ");
  for (int i = 0; command != NULL; i++)
  {
    switch(i) {
      case 0: bytesToRgbResult[0] = atoi(command);
      case 1: bytesToRgbResult[1] = atoi(command);
      case 2: bytesToRgbResult[2] = atoi(command);
    }
    // Find the next token in input string
    command = strtok(NULL, ", ");
  }
}

// vim:ai:cin:sts=2 sw=2 ft=cpp
