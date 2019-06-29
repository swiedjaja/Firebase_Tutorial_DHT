/*
git remote add origin https://github.com/swiedjaja/Firebase_Tutorial_DHT.git
git push -u origin master

username: swiedjaja@gmail.com; Steff123Xyz
*/
#include <Arduino.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "https://environmentmonitoring-792cc.firebaseio.com/"
#define FIREBASE_AUTH "DrkKOHYHRhnhlDVFm1sKI9pLpNXlFlhsT0F4jo34"
#define WIFI_SSID "SmartNet"
#define WIFI_PASSWORD "steffxyz"
#include "DHT.h"

#define DHTPIN D3     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);
//Define FirebaseESP8266 data object
FirebaseData firebaseData;
#define FIREBASE_TEMP "Environment/Data/Temperature"
#define FIREBASE_HUMIDITY "Environment/Data/Humidity"
#define FIREBASE_LEDS "Environment/Control/Led"

#define LED_MAX 4
const byte arLeds[LED_MAX] = {D5, D6, D7, D8};

int nLastLedValue = 0;
bool FirebaseGetInt(const char* path, int& value)
{
  bool result = Firebase.getInt(firebaseData, FIREBASE_LEDS);
  value = firebaseData.intData();
  return result;
}

void setup() {
  Serial.begin(115200);
  for (byte i=0; i<LED_MAX; i++)
  {
     pinMode(arLeds[i], OUTPUT);
     digitalWrite(arLeds[i], HIGH);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  dht.begin();
}

void loop() {
  delay(3000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));

  if (!Firebase.setFloat(firebaseData, FIREBASE_TEMP, t))
    Serial.println("FAILED, REASON: " + firebaseData.errorReason());

  if (!Firebase.setFloat(firebaseData, FIREBASE_HUMIDITY, h))
    Serial.println("FAILED, REASON: " + firebaseData.errorReason());

  if (Firebase.getInt(firebaseData, FIREBASE_LEDS))
  {
     int nData = firebaseData.intData();
     if (nLastLedValue!=nData)
     {
         nLastLedValue = nData;
         Serial.printf("ReceiveData: %d\n", nData);
         for (byte i=0; i<LED_MAX; i++)
           digitalWrite(arLeds[i], HIGH);

         if (nData>0 && nData<=LED_MAX)
           digitalWrite(arLeds[nData-1], LOW);
      }
  }
}
