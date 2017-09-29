#include "temps.h"

#ifdef DHT22_PIN
DHT dht(DHT22_PIN, DHT22);
#endif
#ifdef DS18B20_PIN
OneWire oneWire(DS18B20_PIN);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DeviceAddress ds18Addresses[5];
uint8_t ds18Count;
#endif

static unsigned long dhtReadingRequested = 0;
static unsigned long ds18ReadingRequested = 0;

uint32_t lastTempsTime;
float median(float a, float b, float c);
float temps[3];
float hums[3];
uint8_t measurementCounter = 0;

void readTemperature();
void readDs18();

void tempsSetup()
{
#ifdef DHT22_PIN
  dht.begin();
#endif
#ifdef DS18B20_PIN
  sensors.begin();
  ds18Count = sensors.getDeviceCount();
  ds18Count = ds18Count < 5 ? ds18Count : 5;
  for(uint8_t i = 0; i < ds18Count; i++)
  {
    sensors.getAddress(ds18Addresses[i], i);
    sensors.setResolution(ds18Addresses[i], 12);
  }
  //request async reading
  sensors.setWaitForConversion(false);
#endif
}

void tempsLoop()
{
  uint32_t curmillis = millis();
  //temperature reading. If ds18b20 is used, it is read asynchronly
  if((curmillis - lastTempsTime) > TEMP_READ_INTERVAL)
  {
    lastTempsTime = curmillis;
    readTemperature();
    readDs18();
  }
  if(dhtReadingRequested) readTemperature();
  if(ds18ReadingRequested) readDs18();
}

void readTemperature()
{
#ifdef DHT22_PIN
  //DHT22 can be read every 2 seconds. Make it 3 seconds here to be on the safe side
  if(dhtReadingRequested == 0 || ( measurementCounter < 3 && millis() - dhtReadingRequested > 3000 ))
  {
    float hum = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float temp = dht.readTemperature();
    if(!isnan(hum) && !isnan(temp))
    {
      hums[measurementCounter] = hum;
      temps[measurementCounter] = temp;
      measurementCounter++;
    }

    dhtReadingRequested = millis();

    if(measurementCounter == 3)
    {
      float temp = median(temps[0], temps[1], temps[2]);
      dtostrf(temp, 1, 1, buf);
      mqttClient.publish(OUT_TOPIC_TEMP, buf );

      float hum = median(hums[0], hums[1], hums[2]);
      dtostrf(hum, 1, 1, buf);
      mqttClient.publish(OUT_TOPIC_HUM, buf );

      measurementCounter = 0;
      dhtReadingRequested = 0;
    }
  }
#endif
}

void readDs18()
{
#ifdef DS18B20_PIN
  if (ds18ReadingRequested == 0)
  {
    ds18ReadingRequested = millis();
    //request conversion
    sensors.requestTemperatures();
  }
  else if(millis() - ds18ReadingRequested > 1000)
  {
    for(uint8_t i = 0; i < ds18Count; i++)
    {
      //fetch result requested 1000 ms earlier
      float temp = sensors.getTempC(ds18Addresses[i]);
      //snprintf with float does not work on Arduino (has been removed)
      //snprintf(buf, sizeof(buf), "%f", temp );
      dtostrf(temp, 1, 2, buf20);
      //get device address as hex
      snprintf(buf, sizeof(buf), OUT_TOPIC_TEMP "%02X%02X%02X%02X%02X%02X%02X%02X", ds18Addresses[i][0], ds18Addresses[i][1], ds18Addresses[i][2], ds18Addresses[i][3], ds18Addresses[i][4], ds18Addresses[i][5], ds18Addresses[i][6], ds18Addresses[i][7]);
      mqttClient.publish(buf, buf20 );

      ds18ReadingRequested = 0;
    }
  }
#endif
}

float median(float a, float b, float c)
{
  return fmax(fmin(a,b), fmin(fmax(a,b),c));
}

// vim:ai:cin:sts=2 sw=2 ft=cpp
