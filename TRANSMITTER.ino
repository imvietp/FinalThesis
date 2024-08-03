#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>
#include "DHT.h"


// Firebase setup
#define FIREBASE_HOST "pigmanager-b0507-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "QRuGipNsMYyZUX5X1EoQN5Fd656UkkEmseZXPZW0"


// WiFi credentials
const char* ssid = "vietpham";
const char* password = "11111111";
const long utcOffsetInSeconds = 7 * 3600;  


FirebaseData fbdo;


// Pin definitions
#define RXp2 16
#define TXp2 17
#define DHTPIN 4
#define led 18
#define led_an 19
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


// Variables
int tempUnderThreshold = 25;
int tempUpperThreshold = 35;
int humidityUnderThreshold = 20;
int humidityUpperThreshold = 80;
int valueTemp;
int valueHumidity;
String phoneNumber;
String lastPhoneNumber;

unsigned long lastCheckThresholdCheckTime = 0;
unsigned long lastThresholdCheckTime = 0;
unsigned long lastLightCheckTime = 0;
unsigned long lastFeedingCheckTime = 0;
unsigned long lastCleaningCheckTime = 0;
unsigned long lastPhoneCheckTime = 0;


const unsigned long thresholdCheckInterval = 10000;
const unsigned long lightCheckInterval = 1000; 
const unsigned long checkthresholdCheckInterval = 1000;
const unsigned long feedingCheckInterval = 1000; 
const unsigned long cleaningCheckInterval = 1000; 
const unsigned long phoneCheckInterval = 60000; 



void setup() 
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXp2, TXp2); // UART communication
  WiFi.begin(ssid, password);
  pinMode(led, OUTPUT);
  pinMode(led_an, OUTPUT);
  digitalWrite(led, LOW);
  digitalWrite(led_an, LOW);
  delay(500);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(fbdo, 1000 * 60);
  Firebase.setwriteSizeLimit(fbdo, "tiny");

  dht.begin();
}



void loop() 
{
  unsigned long currentTime = millis();
  if (currentTime - lastThresholdCheckTime >= thresholdCheckInterval) 
  {
    updateFirebase();
    lastThresholdCheckTime = currentTime;
  }


  if (currentTime - lastCheckThresholdCheckTime >= checkthresholdCheckInterval) 
  {
    checkThresholdsAndControlDevices();
    turnonoffLight();
    turnonoffFeeding();
    turnonoffCleaning();
    lastCheckThresholdCheckTime = currentTime;
  }
  

  if (currentTime - lastPhoneCheckTime >= phoneCheckInterval) 
  {
    sendPhoneNumberToArduino();
    lastPhoneCheckTime = currentTime;
  }
}



void updateFirebase() 
{
  valueHumidity = dht.readHumidity();
  valueTemp = dht.readTemperature();

  Firebase.setInt(fbdo, "/CAGE1/temperature/temp", valueTemp);
  Firebase.setInt(fbdo, "/CAGE1/humidity/humid", valueHumidity);
}



void checkThresholdsAndControlDevices() 
{
  static bool aSent = false; 
  static bool bSent = false; 
  static bool cSent = false; 
  static bool dSent = false; 
  bool tempFlag1 = false;
  bool tempFlag2 = false;
  bool humidityFlag1 = false;
  bool humidityFlag2 = false;
  updateFirebase();

  if (Firebase.getInt(fbdo, "/CAGE1/temperature/thresholds/tempUnderThreshold")) 
  {
    tempUnderThreshold = fbdo.intData();
  }

  if (Firebase.getInt(fbdo, "/CAGE1/temperature/thresholds/tempUpperThreshold")) 
  {
    tempUpperThreshold = fbdo.intData();
  }

  if (Firebase.getInt(fbdo, "/CAGE1/humidity/thresholds/humidUnderThreshold")) 
  {
    humidityUnderThreshold = fbdo.intData();
  }

  if (Firebase.getInt(fbdo, "/CAGE1/humidity/thresholds/humidUpperThreshold")) 
  {
    humidityUpperThreshold = fbdo.intData();
  }

  valueTemp = dht.readTemperature();
  valueHumidity = dht.readHumidity();

  if (valueTemp < tempUnderThreshold) 
  {
    tempFlag1 = true;
    tempFlag2 = false;
  } 
  else if (valueTemp > tempUpperThreshold) 
  {
    tempFlag2 = true;
    tempFlag1 = false;
  } 
  else 
  {
    tempFlag1 = false;
    tempFlag2 = false;
  }

  if (valueHumidity < humidityUnderThreshold) 
  {
    humidityFlag1 = true;
    humidityFlag2 = false;
  } 
  else if (valueHumidity > humidityUpperThreshold)
  {
    humidityFlag1 = false;
    humidityFlag2 = true;
  } 
  else 
  {
    humidityFlag1 = false;
    humidityFlag2 = false;
  }

  if (!bSent && !aSent) 
  {
    if (tempFlag1) 
    {
      aSent = true; 
      Serial2.write('A'); 
    } 
    else if (tempFlag2) 
    {
      bSent = true; 
      Serial2.write('B'); 
    }
  } 
  else 
  {
    if (!tempFlag1 && !tempFlag2) 
    {
      aSent = false;
      bSent = false;
      Serial2.write('a'); 
    }
  }

  if (!cSent && !dSent) 
  {
    if (humidityFlag1) 
    {
      cSent = true; 
      Serial2.write('C'); 
    }
    else if (humidityFlag2) 
    {
      dSent = true;
      Serial2.write('G'); 
    }
  } 
  else 
  {
    if (!humidityFlag1 && !humidityFlag2) 
    {
      cSent = false; 
      dSent = false;
      Serial2.write('c'); 
    }
  }
}



void turnonoffLight() 
{
  static bool ledInProgress = false;
  if (Firebase.getString(fbdo, "/CAGE1/Light/Status")) 
  {
    String currentStatusLed = fbdo.stringData();
    if (currentStatusLed == "ON" && !ledInProgress) 
    {
      ledInProgress = true;
      Serial2.write('D'); 
    } 
    else if (currentStatusLed == "OFF" && ledInProgress) 
    {
      ledInProgress = false;
      Serial2.write('d'); 
    }
  }
}



void turnonoffFeeding() 
{
  static bool feedingInProgress = false;
  static unsigned long feedingStartTime = 0;

  if (Firebase.getString(fbdo, "/CAGE1/Feeding/Status")) 
  {
    String feedingStatus = fbdo.stringData();
    if (feedingStatus == "ON" && !feedingInProgress) 
    {
      feedingInProgress = true;
      feedingStartTime = millis(); 
      Serial2.write('E'); 
    } 
    else if (feedingStatus == "ON" && feedingInProgress && millis() - feedingStartTime >= 3000) 
    {
      feedingInProgress = false;
      Serial2.write('e'); 
      Firebase.setString(fbdo, "/CAGE1/Feeding/Status", "OFF");
    } 
    else if (feedingStatus == "OFF" && feedingInProgress) 
    {
      feedingInProgress = false;
      Serial2.write('e');
    }
  }
}



void turnonoffCleaning() 
{
  static bool cleaningInProgress = false;
  if (Firebase.getString(fbdo, "/CAGE1/Cleaning/Strength")) 
  {
    String currentStatusCleaning = fbdo.stringData();
    if (currentStatusCleaning == "ON" && !cleaningInProgress) 
    {
      cleaningInProgress = true;
      Serial2.write('F'); 
    } 
    else if (currentStatusCleaning == "OFF" && cleaningInProgress) 
    {
      cleaningInProgress = false;
      Serial2.write('f'); 
    }
  }
}



void sendPhoneNumberToArduino() 
{
  if (Firebase.getString(fbdo, "/CAGE1/Contact/PhoneNumber")) 
  {
    phoneNumber = fbdo.stringData();
    if (phoneNumber != lastPhoneNumber) 
    { 
      Serial2.print(phoneNumber); 
      lastPhoneNumber = phoneNumber; 
    }
  }
}
