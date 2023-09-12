//============================================
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define REPORTING_PERIOD_MS     1000
PulseOximeter pox;
uint32_t tsLastReport = 0;
//===========================================
//===================DHT HEADER---------
#include "DHT.h"
#define DHTPIN A0    //DHT 1 output connected to Pin 9 
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);//create an instance of DHT
#define DEBUG true

float h;
float t;
float f;
//=========================================

//==============
char str[70];
String gpsString = "";
char *test = "$GPGGA";
String latitude = "No Range      ";
String longitude = "No Range     ";
int temp = 0, ii;
int countstate = 0;

int switchstatus = 1;
boolean gps_status = 0;
String stringOne = "this", stringTwo;
//=============

const int mqPin = A2;     // the number of the pushbutton pin
int mqState = 0;

#include <SoftwareSerial.h>
SoftwareSerial mySerial(11, A3); // RX, TX

//=========================================
#include <LiquidCrystal.h>
//#define DEBUG true

const int button1Pin = 12;     // the number of the pushbutton pin
int button1State = 0;
const int button2Pin = 13;     // the number of the pushbutton pin
int button2State = 0;

const int buzzerPin = 10;     // the number of the pushbutton pin
int buzzerState = 0;

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

int sp = 0;
int count = 0;
int smscount1 = 0;
int smscount2 = 0;

//==================================================================

#define DEBUG true
#define IP "184.106.153.149"// thingspeak.com ip
String Api_key = "GET /update?key=TCK3305L0H8EBMVZ"; //change it with your api key like "GET /update?key=Your Api Key"

int error;

//=======================
void setup()
{

  pinMode(9, INPUT); // Setup for leads off detection LO +
  pinMode(8, INPUT); // Setup for leads off detection LO -

  //=====================
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  Serial.begin(9600);
  mySerial.begin(9600);
  dht.begin();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print(" IoT PATIENT  ");
  lcd.setCursor(0, 1);
  lcd.print("MONITORING S/M");
  buzzerState = LOW;
  digitalWrite(buzzerPin, buzzerState);
  delay(3000);
  //====================================
  send_command("AT+RST\r\n", 2000, DEBUG); //reset module
  send_command("AT+CWMODE=1\r\n", 1000, DEBUG); //set station mode
  send_command("AT+CWJAP=\"ABC\",\"11111111\"\r\n", 2000, DEBUG);
  //====================================
  //==initializing Serial========================
  delay(1000);
  mySerial.println("AT");    //Sets the Serial Module in Text Mode
  delay(1000);
  mySerial.println("AT+CMGF=1");    //Sets the Serial Module in Text Mode
  delay(1000);
  mySerial.println("AT+CMGD=1");    //Sets the Serial Module in Text Mode
  delay(1000);
  mySerial.println("AT+CMGD=2");    //Sets the Serial Module in Text Mode
  delay(1000);
  mySerial.println("AT+CSMS=1");    //Sets the Serial Module in Text Mode
  delay(1000);
  mySerial.print("AT+CSCS=");    //Sets the Serial Module in Text Mode
  mySerial.print((char)34);    //Sets the Serial Module in Text Mode
  mySerial.print("Serial");    //Sets the Serial Module in Text Mode
  mySerial.println((char)34);    //Sets the Serial Module in Text Mode
  delay(2000);
  mySerial.println("AT+CNMI=2,2,0,0,0");    //Sets the Serial Module in Text Mode
  delay(1000);
  mySerial.println("AT+CSMP=17,167,0,0");    //Sets the Serial Module in Text Mode
  delay(1000);

  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  if (!pox.begin()) {

    // Serial.println("FAILED");
    for (;;);
  } else {
    // Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);


}

void loop()
{

label1:

  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
    sp = pox.getSpO2();
    tsLastReport = millis();
    lcd.clear();
    lcd.print("SPO2:");
    lcd.print(sp);
    lcd.print("%");

    button2State = digitalRead(button2Pin);
    if (button2State == LOW)
    {
      // thingSpeakWrite(sp, cel);
      // updatedata();
      lcd.clear();
      lcd.print("Checking room..");
      lcd.setCursor(0, 1);
      lcd.print("Parameters....");
      delay(4000);
      goto checkparameter;
    }


    button1State = digitalRead(button1Pin);
    if (button1State == LOW)
    {
      lcd.clear();
      lcd.print("Checking ECG.....");
      delay(3000);
      goto checkecg;
    }

  }


  goto label1;

  //===============================



checkparameter:
  //===================
  h = dht.readHumidity();
  t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  lcd.clear();
  lcd.print("H:");
  lcd.print(h);
  lcd.print("%");
  lcd.print("|T:");
  lcd.print(t);
  lcd.print(char(223));
  lcd.print("C");

  count++;
  lcd.setCursor(0, 1);
  lcd.print(count);
  delay(100);

  if (count == 100)
  {
    count = 0;
    lcd.clear();
    lcd.print("Thingspeak.....");
    updatedata();
    delay(5000);
    goto checkparameter;
  }

  button2State = digitalRead(button2Pin);
  if (button2State == LOW)
  {
    lcd.clear();
    lcd.print("Checking SPO2...");
    delay(4000);
    goto label1;
  }


  //========mq sensor =============
  mqState = digitalRead(mqPin);
  if (mqState == LOW)
  {
    lcd.clear();
    lcd.print("Toxic Gas");
    lcd.setCursor(0, 1);
    lcd.print("Detected!!!");
    buzzerState = HIGH;
    digitalWrite(buzzerPin, buzzerState);
    //  delay(2000);
  }

  if (mqState == HIGH)
  {
    buzzerState = LOW;
    digitalWrite(buzzerPin, buzzerState);
  }
  /*
  //=====================================
  if (t > 35)
  {
    if (smscount1 == 0)
    {
      //===============
      get_gps();
      lcd.clear();
      lcd.print(latitude);
      lcd.setCursor(0, 1);
      lcd.print(longitude);
      delay(1000);
      mySerial.println("AT+CMGF=1");    //Sets the Serial Module in Text Mode
      delay(1000);  // Delay of 1000 milli seconds or 1 second
      mySerial.println("AT+CMGS=\"+919380103868\"\r"); // Replace x with mobile number
      delay(1000);
      mySerial.print("Temeparture:");// The SMS text you want to send
      mySerial.print(t);
      mySerial.print("Location:");
      mySerial.print(latitude);
      mySerial.print("N");

      mySerial.print(longitude);
      mySerial.print("E");
      delay(1000);
      mySerial.println((char)26);// ASCII code of CTRL+Z
      delay(1000);
      lcd.clear();
      lcd.println("SMS Sent !!!");
      smscount1 = 1;
    }
    //============
  }
  else
  {
    smscount1 = 0;
  }
  //=======================================
*/

  
 mqState = digitalRead(mqPin); 
  
  //=====================================
  if (mqState == LOW)
  {
    if (smscount2 == 0)
    {
      //===============
      get_gps();
      lcd.clear();
      lcd.print(latitude);
      lcd.setCursor(0, 1);
      lcd.print(longitude);
      delay(1000);
      mySerial.println("AT+CMGF=1");    //Sets the Serial Module in Text Mode
      delay(1000);  // Delay of 1000 milli seconds or 1 second
      mySerial.println("AT+CMGS=\"+919380103868\"\r"); // Replace x with mobile number
      delay(1000);
      mySerial.print("|Toxic Gas Detected.");
      mySerial.print("Location:");
      mySerial.print(latitude);
      mySerial.print("N");

      mySerial.print(longitude);
      mySerial.print("E");
      delay(1000);
      mySerial.println((char)26);// ASCII code of CTRL+Z
      delay(1000);
      lcd.clear();
      lcd.println("SMS Sent !!!");
      smscount2 = 1;
    }
    //============
  }
  else
  {
    smscount2 = 0;
  }
  //=======================================



  goto checkparameter;


  //=====checking ECG=================
checkecg:

  if ((digitalRead(9) == 1) || (digitalRead(8) == 1))
  {
    Serial.println('!');
  }
  else
  {
    Serial.println(analogRead(A1));
  }
  delay(1);
  goto checkecg;
  //====================
}

//=======================
void updatedata() {
  String command = "AT+CIPSTART=\"TCP\",\"";
  command += IP;
  command += "\",80";
  //Serial.begin(9600);
  delay(400);
  // Serial.begin(9600);
  delay(400);
  // Serial.println(command);
  delay(400);
  Serial.println(command);
  delay(2000);
  if (Serial.find("Error")) {
    return;
  }
  command = Api_key;
  command += "&field1=";
  command += String(sp);
  command += "&field2=";
  command += String(t);
  command += "&field3=";
  command += String(h);
 // command += "&field4=";
 // command += String(mqState);
  //  command += "&field5=";
  // command += String(inthrdata);
  command += "\r\n\r\n\r\n";
  Serial.print("AT+CIPSEND=");
  //  Serial.print("AT+CIPSEND=");
  Serial.println(command.length());
  // Serial.println(command.length());


  if (Serial.find(">")) {
    delay(400);
    Serial.print(command);
    delay(400);
    //   Serial.print(command);
    delay(400);
    delay(5000);
    Serial.println("AT+RST");
    delay(200);
    Serial.println("AT");
    delay(200);
    Serial.println("AT");
    delay(200);
    // Serial.println("AT");
    delay(200);
  }

  /*
    Serial.print("AT+CIPSEND=");
    delay(20);
    Serial.print("AT+CIPSEND=");
    delay(20);
    Serial.println(command.length());
    delay(200);
    Serial.println(command.length());
    Serial.begin(9600);
    delay(200);
    if (Serial.find(">")) {
     delay(100);
     Serial.println(command);
     delay(20);
     Serial.println(command);
     delay(3000);
     Serial.println("AT");
     delay(200);
    }
  */
  else {

    Serial.println("AT+CIPCLOSE");
    // Serial.println("AT+CIPCLOSE");
    //Resend...
    error = 1;
  }
}








String send_command(String command, const int timeout, boolean debug)
{
  Serial.begin(9600);
  String response = "";
  Serial.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (Serial.available())
    {
      char c = Serial.read();
      response += c;
    }
  }
  if (debug)
  {
    delay(10);
    //  Serial.println(response);
  }
  return response;
}

//=================================================
//==========GPS FUNCTION===================================
void gpsEvent()
{
  gpsString = "";
  while (1)
  {
    while (mySerial.available() > 0)          //checking serial data from GPS
    {
      char inChar = (char)mySerial.read();
      gpsString += inChar;                   //store data from GPS into gpsString
      ii++;
      if (ii < 7)
      {
        if (gpsString[ii - 1] != test[ii - 1])    //checking for $GPGGA sentence
        {
          ii = 0;
          gpsString = "";
        }
      }
      if (inChar == '\r')
      {
        if (ii > 65)
        {
          gps_status = 1;
          break;
        }
        else
        {
          ii = 0;
        }
      }
    }
    if (gps_status)
      break;
  }
}
//========================GPS FUNCTION TILL HERE==================

//==================GET GPS FUNCTION===========
void get_gps()
{
  gps_status = 0;
  int x = 0;
  while (gps_status == 0)
  {
    gpsEvent();
    int str_lenth = ii;
    latitude = "";
    longitude = "";
    int comma = 0;
    while (x < str_lenth)
    {
      if (gpsString[x] == ',')
        comma++;
      if (comma == 2)     //extract latitude from string
        latitude += gpsString[x + 1];
      else if (comma == 4)     //extract longitude from string
        longitude += gpsString[x + 1];
      x++;
    }
    int l1 = latitude.length();
    latitude[l1 - 1] = ' ';
    l1 = longitude.length();
    longitude[l1 - 1] = ' ';
    lcd.clear();
    lcd.print("Lat:");
    lcd.print(latitude);
    lcd.setCursor(0, 1);
    lcd.print("Long:");
    lcd.print(longitude);
    ii = 0; x = 0;
    str_lenth = 0;
    delay(2000);
  }
}
//=================GET GPS FUNCTION TILL HERE==========
