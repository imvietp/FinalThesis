#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>


#define RXp2 15
#define TXp2 14


#define pinMaySuoi 5
#define pinQuatDHT11 6
#define pinPhunSuong 7


#define pinBuzzerFlameSensor 3
#define pinBuzzer_MQ2  4


#define pinMQ2Sensor  A1
#define pinFlameSensor 2


#define pinFanMQ2  12
#define pinPhunSuongDapLua 11


#define pinLedTrangTrai 8
//#define pinChoAn 9
#define pinVeSinh A0


#define buttonDenSuoi 31
#define buttonQuatDHT11 33
#define buttonPhunSuong 35
#define buttonDenTrangTrai 37
#define buttonChoHeoAn 39
#define buttonVeSinh 41


Servo myservo;
bool tempDenSuoi = LOW;
bool tempQuatDHT11 = LOW;
bool tempPhunSuong = LOW;
bool tempQuatHut = LOW;
bool tempQuatHutDoAM = LOW;
bool tempDenTrangTrai = LOW;
bool tempChoHeoAn = LOW;
bool tempVeSinh = LOW;
bool phoneNumberMode = false; // Flag to indicate whether the system is in phone number reception mode
bool flagFlame = 0;
bool flagMQ135 = 0;



unsigned long fireDetectedTime = 0; 
unsigned long pumpStartTime = 0; 
unsigned long lastFlameSensor_CheckTime = 0;


unsigned long previousMillis = 0;
unsigned long smokeDetectedTime = 0;
unsigned long fanStartTime = 0;
unsigned long lastMQ2Sensor_CheckTime = 0;
const int thresholdMQ135 = 500; // Threshold value for fire detection



String phoneNumber = "";
String receivedNumber = "";


// EEPROM address to store the phone number
const int EEPROM_ADDRESS = 0;
const int PHONE_NUMBER_MAX_LENGTH = 20; 





SoftwareSerial sim800L(17, 16);
LiquidCrystal_I2C lcd(0x27, 20, 4); 

const int numDevices = 10;
String deviceStatuses[numDevices] = {
  "DenSuoi: OFF",
  "Quat: OFF",
  "P_Suong: OFF",
  "FarmLED: OFF",
  "Feed: OFF",
  "Clean: OFF"
};


void setup() {
  Serial.begin(115200);
  Serial3.begin(115200); // Hardware Serial of Arduino mega
  sim800L.begin(9600);
  myservo.attach(9);
  pinMode(buttonDenSuoi, INPUT_PULLUP);
  pinMode(buttonQuatDHT11, INPUT_PULLUP);
  pinMode(buttonPhunSuong, INPUT_PULLUP);
  pinMode(buttonDenTrangTrai, INPUT_PULLUP);
  pinMode(buttonChoHeoAn, INPUT_PULLUP);
  pinMode(buttonVeSinh, INPUT_PULLUP);
  pinMode(pinFlameSensor, INPUT); //initialize Flame sensor output pin connected pin as input.
  pinMode(pinMaySuoi, OUTPUT);
  pinMode(pinQuatDHT11, OUTPUT);
  pinMode(pinPhunSuong, OUTPUT);
  pinMode(pinBuzzerFlameSensor, OUTPUT);
  pinMode(pinPhunSuongDapLua, OUTPUT);
  pinMode(pinLedTrangTrai, OUTPUT);
  //pinMode(pinChoAn, OUTPUT);
  pinMode(pinMQ2Sensor, INPUT);
  pinMode(pinBuzzer_MQ2, OUTPUT);
  pinMode(pinFanMQ2, OUTPUT);
  pinMode(pinVeSinh, OUTPUT);

  Serial.println("Initializing...");

  lcd.init();
  lcd.backlight();

  //Once the handshake test is successful, it will back to OK
  sim800L.println("AT");
  delay(1000);
  sim800L.println("AT+CMGF=1");
  delay(1000);

  // Retrieve phone number from EEPROM
  retrievePhoneNumber();

  lcd.setCursor(10, 2);
  lcd.print("Pump: Off"); // Initial pump status

  lcd.setCursor(10, 3);
  lcd.print("TG: Off");
  updateLCD();
}
 



void loop() {
  unsigned long currentTime = millis();
  
  if (Serial3.available() > 0) 
  {
      char command = Serial3.peek(); 
      if (command == '+') 
      {
          receivePhoneNumber();
      } else {
          onoffLED(); 
      }
  }

  if (currentTime - lastFlameSensor_CheckTime >= 200) 
  {
    flamesensorFunction();
    mq2sensorFunction();
    lastFlameSensor_CheckTime = currentTime;
  }

  manualOnOff();
}



////////////////////////// ON OFF LED /////////////////////////////////
void onoffLED() {
  if (Serial3.available() > 0) {
    char message = Serial3.read();
    switch (message) {
      case 'A':
        digitalWrite(pinMaySuoi, HIGH);
        deviceStatuses[0] = "DS: ON";
        tempDenSuoi = true;
        break;
      case 'B':
        digitalWrite(pinQuatDHT11, HIGH);
        deviceStatuses[1] = "Quat: ON";
        tempQuatDHT11 = true;
        break;
      case 'a':
        digitalWrite(pinMaySuoi, LOW);
        digitalWrite(pinQuatDHT11, LOW); 
        deviceStatuses[0] = "DS: OFF";
        deviceStatuses[1] = "Quat: OFF";
        tempDenSuoi = false;
        tempQuatDHT11 = false;
        break;
      case 'C':
        digitalWrite(pinPhunSuong, HIGH); 
        deviceStatuses[2] = "PS: ON";
        tempPhunSuong = true;
        break;
      case 'G':
        tempQuatHut = true;
        digitalWrite(pinFanMQ2, HIGH);
        break;
      case 'c':
        digitalWrite(pinPhunSuong, LOW); 
        digitalWrite(pinFanMQ2, LOW); 
        deviceStatuses[2] = "PS: OFF";
        tempPhunSuong = false;
        tempQuatHut = false;
        break;
      case 'D':
        digitalWrite(pinLedTrangTrai, HIGH); 
        deviceStatuses[3] = "Den: ON";
        tempDenTrangTrai = true;
        break;
      case 'd':
        digitalWrite(pinLedTrangTrai, LOW); 
        deviceStatuses[3] = "Den: OFF";
        tempDenTrangTrai = false;
        break;
      case 'E':
        myservo.write(90); 
        lcd.setCursor(14, 0);
        lcd.print("EAT: ON");
        tempChoHeoAn = true;
        break;
      case 'e':
        myservo.write(0);
        lcd.setCursor(14, 0);
        lcd.print("EAT: OFF");
        tempChoHeoAn = false;
        break;
      case 'F':
        digitalWrite(pinVeSinh, HIGH); 
        lcd.setCursor(14, 1);
        lcd.print("VS: ON");
        tempVeSinh = true;
        break;
      case 'f':
        digitalWrite(pinVeSinh, LOW);
        lcd.setCursor(14, 1);
        lcd.print("VS: OFF");
        tempVeSinh = false;
        break;
      default:
        Serial.println("Received unrecognized command: " + String(message)); 
        return;
    }
    updateLCD(); 
  }
}



void manualOnOff() {
  if (digitalRead(buttonDenSuoi) == LOW) {
    delay(10);
    if (digitalRead(buttonDenSuoi) == LOW) {
      tempDenSuoi = !tempDenSuoi;
      digitalWrite(pinMaySuoi, tempDenSuoi);
      updateLCD();
    }
    while (digitalRead(buttonDenSuoi) == LOW);
  } 
  else if (digitalRead(buttonQuatDHT11) == LOW) {
    delay(10);
    if (digitalRead(buttonQuatDHT11) == LOW) {
      tempQuatDHT11 = !tempQuatDHT11;
      digitalWrite(pinQuatDHT11, tempQuatDHT11);
      updateLCD();
    }
    while (digitalRead(buttonQuatDHT11) == LOW);
  } 
  else if (digitalRead(buttonPhunSuong) == LOW) {
    delay(10);
    if (digitalRead(buttonPhunSuong) == LOW) {
      tempPhunSuong = !tempPhunSuong;
      digitalWrite(pinPhunSuong, tempPhunSuong);
      updateLCD();
    }
    while (digitalRead(buttonPhunSuong) == LOW);
  } 
  else if (digitalRead(buttonDenTrangTrai) == LOW) {
    delay(10);
    if (digitalRead(buttonDenTrangTrai) == LOW) {
      tempDenTrangTrai = !tempDenTrangTrai;
      digitalWrite(pinLedTrangTrai, tempDenTrangTrai);
      updateLCD();
    }
    while (digitalRead(buttonDenTrangTrai) == LOW);
  } 
  else if (digitalRead(buttonChoHeoAn) == LOW) {
    delay(10);
    if (digitalRead(buttonChoHeoAn) == LOW) {
      tempChoHeoAn = !tempChoHeoAn;
      myservo.write(tempChoHeoAn ? 90 : 0); 
      updateLCD();
    }
    while (digitalRead(buttonChoHeoAn) == LOW); 
  } 
  else if (digitalRead(buttonVeSinh) == LOW) {
    delay(10);
    if (digitalRead(buttonVeSinh) == LOW) {
      tempVeSinh = !tempVeSinh;
      digitalWrite(pinVeSinh, tempVeSinh);
      updateLCD();
    }
    while (digitalRead(buttonVeSinh) == LOW);
  }
}



void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DS: ");
  lcd.print(tempDenSuoi ? "On" : "Off");

  lcd.setCursor(0, 1);
  lcd.print("Quat: ");
  lcd.print(tempQuatDHT11 ? "On" : "Off");

  lcd.setCursor(0, 2);
  lcd.print("PS: ");
  lcd.print(tempPhunSuong ? "On" : "Off");

  lcd.setCursor(0, 3);
  lcd.print("Den: ");
  lcd.print(tempDenTrangTrai ? "On" : "Off");

  lcd.setCursor(10, 0); 
  lcd.print("An: ");
  lcd.print(tempChoHeoAn ? "On" : "Off");

  lcd.setCursor(10, 1); 
  lcd.print("VS: ");
  lcd.print(tempVeSinh ? "On" : "Off");

  lcd.setCursor(10, 2);
  lcd.print("DL: "); 
  lcd.print(tempVeSinh ? "On" : "Off");
  
  lcd.setCursor(10, 3);
  lcd.print("TG: "); 
  lcd.print(tempQuatHut ? "On" : "Off");

}


//////////////////////// RECEIVE PHONE NUMBER /////////////////////////////////
void receivePhoneNumber() {
    phoneNumber = Serial3.readStringUntil('\n'); 
    phoneNumber.trim(); 
    // Check if the received data is a valid phone number
    if (isValidPhoneNumber(phoneNumber)) {
        storePhoneNumber(phoneNumber);
        Serial.println("Phone number received and stored: " + phoneNumber); 
    } else {
        Serial.println("Received unrecognized data: " + phoneNumber); 
    }
}




bool isValidPhoneNumber(const String& number) {
    // A valid phone number starts with + and has digits
    if (number.startsWith("+") && number.length() > 1) {
        for (unsigned int i = 1; i < number.length(); i++) {
            if (!isdigit(number.charAt(i))) {
                return false;
            }
        }
        return true;
    }
    return false;
}




bool isNumeric(String str) 
{
  for (unsigned int i = 0; i < str.length(); i++) 
  {
    if (!isdigit(str.charAt(i))) 
    {
      return false;
    }
  }
  return true;
}




void storePhoneNumber(String phoneNumber) {
  // Check if the phone number is a control character
  if (phoneNumber == "A" || phoneNumber == "B" || phoneNumber == "a" || phoneNumber == "C" || phoneNumber == "c" || phoneNumber == "D" || phoneNumber == "d" || phoneNumber == "E" || phoneNumber == "e") {
    Serial.println("Received a control character. Ignoring."); // Debugging print
    return; // Ignore control characters
  }
  
  int len = phoneNumber.length();
  Serial.println("Storing phone number: " + phoneNumber + " Length: " + String(len)); // Debugging print
  EEPROM.write(EEPROM_ADDRESS, len); // Write the length of the phone number
  for (int i = 0; i < len; i++) {
    EEPROM.write(EEPROM_ADDRESS + 1 + i, phoneNumber[i]); // Write each character of the phone number
    Serial.println("Writing char: " + String(phoneNumber[i]) + " at address: " + String(EEPROM_ADDRESS + 1 + i)); // Debugging print
  }
}


 

void retrievePhoneNumber() {
  int len = EEPROM.read(EEPROM_ADDRESS); // Read the length of the phone number
  Serial.println("Length retrieved from EEPROM: " + String(len)); // Debugging print

  if (len > 0 && len <= PHONE_NUMBER_MAX_LENGTH) {
    char phoneNumberArr[PHONE_NUMBER_MAX_LENGTH + 1]; // Temporary array to hold the phone number
    for (int i = 0; i < len; i++) {
      phoneNumberArr[i] = EEPROM.read(EEPROM_ADDRESS + 1 + i); // Read each character of the phone number
      Serial.println("Reading char: " + String(phoneNumberArr[i]) + " from address: " + String(EEPROM_ADDRESS + 1 + i)); // Debugging print
    }
    phoneNumberArr[len] = '\0'; // Null-terminate the string
    phoneNumber = String(phoneNumberArr); // Convert the character array to a String
    Serial.println("Phone number retrieved from EEPROM: " + phoneNumber); // Debugging print
  } else {
    Serial.println("No valid phone number stored in EEPROM"); // Debugging print
  }
}




/////////////////////////// FLAME SENSOR FUNCTION ////////////////////////////////////////
void flamesensorFunction() {
  unsigned long currentMillis = millis(); 

  // Read from the SIM800L module
  while (sim800L.available())
  {
    Serial.println(sim800L.readString());
  }

  int flameValue = digitalRead(pinFlameSensor);

  if (flameValue == 0) 
  { 
    digitalWrite(pinBuzzerFlameSensor, HIGH); 
    if (fireDetectedTime == 0 && flagFlame == 0) 
    {
      send_fire_sms();
      fireDetectedTime = currentMillis; 
      flagFlame = 1;
    }
  } 
  else 
  { 
    digitalWrite(pinBuzzerFlameSensor, LOW);
    fireDetectedTime = 0; 
    flagFlame = 0;
    if (pumpStartTime > 0) 
    {
      digitalWrite(pinPhunSuongDapLua, LOW); 
      lcd.setCursor(10, 2);
      lcd.print("Pump: Off");
      send_fire_off_sms(); 
      pumpStartTime = 0; 
    }
  }

  // If fire was detected and it's been 3 seconds, turn on the pump
  if (fireDetectedTime > 0 && currentMillis - fireDetectedTime >= 3000 && flagFlame == 1) 
  {
    digitalWrite(pinPhunSuongDapLua, HIGH); 
    lcd.setCursor(10, 2);
    lcd.print("Pump: On "); 
    send_fire_destruction_sms(); 
    pumpStartTime = currentMillis; 
    flagFlame = 2; 
  }

  // If the pump is on and fire is no longer detected, turn off the pump
  if (pumpStartTime > 0 && flameValue != 0) 
  {
    digitalWrite(pinPhunSuongDapLua, LOW); 
    lcd.setCursor(10, 2);
    lcd.print("Pump: Off");   
    send_fire_off_sms();
    pumpStartTime = 0; 
    flagFlame = 0; 
  }
}




//////////////////////////////////////////// MQ2 SENSOR ///////////////////////////////////////
void mq2sensorFunction() {
  unsigned long currentMillis = millis();

  int sensorValue = readSensor();

  if (sensorValue >= thresholdMQ135) 
  { // Value exceeds threshold
    digitalWrite(pinBuzzer_MQ2, HIGH);
    if (smokeDetectedTime == 0 && flagMQ135 == 0) 
    {
      send_mq2_sms();
      smokeDetectedTime = currentMillis; 
      flagMQ135 = 1;
    }
  } 
  else 
  { 
    digitalWrite(pinBuzzer_MQ2, LOW); 
    smokeDetectedTime = 0; 
    flagMQ135 = 0;
    if (fanStartTime > 0) 
    {
      digitalWrite(pinFanMQ2, LOW); 
      lcd.setCursor(10, 3);
      lcd.print("TG: Off");
      send_fan_off_sms(); 
      fanStartTime = 0; 
    }
  }

  // If value was above threshold and it's been 3 seconds, turn on the fan
  if (smokeDetectedTime > 0 && currentMillis - smokeDetectedTime >= 2500 && flagMQ135 == 1) 
  {
    digitalWrite(pinFanMQ2, HIGH); 
    tempQuatHut = true;
    send_fan_on_sms(); 
    fanStartTime = currentMillis; 
    flagMQ135 = 2; 
  }

  // If the fan is on and the value is below threshold, turn off the fan
  if (fanStartTime > 0 && sensorValue < thresholdMQ135) 
  {
    digitalWrite(pinFanMQ2, LOW); 
    tempQuatHut = false;
    send_fan_off_sms(); 
    fanStartTime = 0; 
    flagMQ135 = 0;
  }
}




int readSensor() 
{
  unsigned long currentMillis = millis(); 

  // Check if 1000 milliseconds have passed since the last reading
  if (currentMillis - previousMillis >= 1000) 
  {
    previousMillis = currentMillis; 

    unsigned int sensorValue = analogRead(pinMQ2Sensor); 
    unsigned long currentTime = millis(); 

    //Serial.print("Time: ");
    //Serial.print(currentTime);
    //Serial.print(" ms, Analog output: ");
    //Serial.println(sensorValue);

    return sensorValue; 
  }
  return -1;
}




void send_mq2_sms() 
{
  if (phoneNumber != "") 
  {
    send_sms("Smoke is Detected", phoneNumber);
  }
}




void send_fan_on_sms() 
{
  if (phoneNumber != "") 
  {
    send_sms("Smoke is being destroyed", phoneNumber);
  }
}




void send_fan_off_sms() 
{
  if (phoneNumber != "") 
  {
    send_sms("Fan is off", phoneNumber);
  }
}

////////////////////////////////////// FLAME SENSOR ///////////////////////////////////////////////
void send_fire_sms() 
{
  if (phoneNumber != "") 
  {
    send_sms("Fire is Detected", phoneNumber);
  }
}



void send_fire_destruction_sms() 
{
  if (phoneNumber != "") 
  {
    send_sms("Fire is being destroyed", phoneNumber);
  }
}



void send_fire_off_sms()
{
  if (phoneNumber != "") 
  {
    send_sms("Pump is off", phoneNumber);
  }
}


/////////////////////////////// SEND SMS FUNCTION /////////////////////////////////////
void send_sms(String text, String phone) 
{
  Serial.println("sending sms....");
  delay(50);
  sim800L.print("AT+CMGF=1\r");
  delay(1000);
  sim800L.print("AT+CMGS=\"" + phone + "\"\r");
  delay(1000);
  sim800L.print(text);
  delay(100);
  sim800L.write(0x1A); 
  delay(5000);
}



void make_call(String phone) 
{
  Serial.println("calling....");
  sim800L.println("ATD" + phone + ";");
  delay(20000); 
  sim800L.println("ATH");
  delay(1000); 
}
