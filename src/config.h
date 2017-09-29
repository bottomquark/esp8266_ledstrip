#ifndef __CONFIG_H_INCLUDED
#define __CONFIG_H_INCLUDED

#include "credentials.h"

//D0 has no interrupt or PWM
#define MOTION_PIN   D6
#define MOTION_PIN2  D8
#define LDR_PIN      A0

//the pin for a WS2801 LED strip. Either this or an RGB strip may be connected.
#define LED_PIN   D2

#define LED_PIN_W   D1
#define LED_PIN_R   D2
#define LED_PIN_B   D3
//D4 is LED_BUILTIN on d1_mini: disconnect LED if in use 
#define LED_PIN_G   D4

//if using DHT22, define it here. 
#define DHT22_PIN D5
//if using one or more DS18B20, define the pin here.
//up to 5 may be connected in series and will be recognized automatically
#define DS18B20_PIN D7

//see ~/.platformio/packages/framework-arduinoespressif8266/variants/d1_mini/pins_arduino.h for pin mapping
//TODO: implement BMP180
#define BMP180_SDA RX
#define BMP180_SCL TX

//intervals for reporting stats and temperatures
#define TEMP_READ_INTERVAL 50000
#define STATS_INTERVAL     60000

#define MQTT_CLIENT_ID "kitchen"
#define IN_TOPIC_OTA "fn/kitchen/in/ota"

//definitions of MQTT topics
//in topics
#define IN_SUBSCRIBE_TOPIC "fn/" MQTT_CLIENT_ID "/in"
#define IN_TOPIC_LED_WHITE IN_SUBSCRIBE_TOPIC "/leds/white"
#define IN_TOPIC_LED_RGB IN_SUBSCRIBE_TOPIC "/leds/rgb"
#define IN_TOPIC_LED_FADEOUT_TIME IN_SUBSCRIBE_TOPIC "/leds/fadeouttime"
#define IN_TOPIC_LED_FADEIN_TIME IN_SUBSCRIBE_TOPIC "/leds/fadeintime"
#define IN_TOPIC_LED_FADEOUT IN_SUBSCRIBE_TOPIC "/leds/fadeout"
#define IN_TOPIC_LED_FADEIN IN_SUBSCRIBE_TOPIC "/leds/fadein"
#define IN_TOPIC_LED_AUTOFIRE IN_SUBSCRIBE_TOPIC "/leds/autofire"
//switch on with 1, off with 0 to this topic
#define IN_TOPIC_LED_SWITCH IN_SUBSCRIBE_TOPIC "/leds/switch"

//out topics
#define OUT_TOPIC_MOTION "fn/" MQTT_CLIENT_ID "/out/motion"
#define OUT_TOPIC_LIGHT "fn/" MQTT_CLIENT_ID "/out/light"
#define OUT_TOPIC_TEMP "fn/" MQTT_CLIENT_ID "/out/temp"
#define OUT_TOPIC_HUM "fn/" MQTT_CLIENT_ID "/out/hum"
#define OUT_TOPIC_PRESSURE "fn/" MQTT_CLIENT_ID "/out/pressure"

//LED status topics
#define OUT_TOPIC_LED_SWITCH "fn/" MQTT_CLIENT_ID "/status/switch"
#define OUT_TOPIC_LED_WHITE "fn/" MQTT_CLIENT_ID "/status/white"
#define OUT_TOPIC_LED_RGB "fn/" MQTT_CLIENT_ID "/status/rgb"

//stats topics:
#define OUT_TOPIC_STATS "fn/" MQTT_CLIENT_ID "/stats"
#define OUT_TOPIC_STATS_IP OUT_TOPIC_STATS "/ip"
#define OUT_TOPIC_STATS_LOOPSPEED OUT_TOPIC_STATS "/loopspeed"
#define OUT_TOPIC_STATS_GITREV OUT_TOPIC_STATS "/gitRev"

//global, multi purpose buffer for conversions etc. 16 chars for DS18B20 address + 34 for MQTT topic = 50 chars 
static char buf[50];
//second buffer, used in temps.cpp currently
static char buf20[20];

#endif //__CONFIG_H_INCLUDED

// vim:ai:cin:sts=2 sw=2 ft=cpp
