#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <RTClib.h>

#define LM35_PIN A0
#define BUZZER_PIN 8
#define DS1307_SDA A4
#define DS1307_SCL A5

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
RTC_DS1307 rtc;
SoftwareSerial esp(0, 1); // RX, TX
DateTime dt, alarm;

float sensorValue;
float voltageOut;
float temperature;
char choice, command;
bool active = LOW;

int dd = -1, MM = -1 , yyyy = -1, HH = -1, mm = -1;
String ddatime, ttime, dnt;
String desc, heat, humid;

void setup() {
   
  Serial.begin(9600); 
  esp.begin(9600);
  esp.setTimeout(10);
  lcd.begin(16,2);
  pinMode(LM35_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  lcd.clear();
  
  lcd.print("Power on");
  delay(5000); //waiting for loading local web
  choice = '1';
}

void getAlarmData() {

  // Format received: yyyy-MM-dd+HH:mm
  // Extract date and time from inputString
 
  if (esp.available() >= 16) {
    dnt = esp.readStringUntil('\n');
    yyyy = dnt.substring(0,4).toInt();
    MM = dnt.substring(5,7).toInt();
    dd = dnt.substring(8,10).toInt();
    HH = dnt.substring(11,13).toInt();
    mm = dnt.substring(14).toInt();
    alarm = DateTime(yyyy, MM, dd, HH, mm, 0);
  }
  showDateTime();
}

void checkAlarm() {
  dt = rtc.now();
  if(dd != -1) {
    if(dt == alarm) {
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("alarm");
      digitalWrite(BUZZER_PIN, HIGH);
      choice = 0;
    }     
  }  
}



void showDateTime() {
  dt = rtc.now();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Date: ");
  lcd.print(dt.day(), DEC);
  lcd.print("/");
  lcd.print(dt.month(), DEC);
  lcd.print("/");
  lcd.print(dt.year(), DEC);

  lcd.setCursor(0,1);
  lcd.print("Time: ");
  lcd.print(dt.hour(), DEC);
  lcd.print(':');
  lcd.print(dt.minute(), DEC);
  lcd.print(':');
  lcd.print(dt.second(), DEC);
  delay(1000);
}

void showTemperature() {
  lcd.clear(); 
  sensorValue = analogRead(LM35_PIN);
  voltageOut = (sensorValue * 5000) / 1024;
  temperature = voltageOut / 10;
  lcd.setCursor(0,0);
  lcd.print("Heat: ");
  lcd.print(temperature);
  lcd.print("^C"); 
  delay(1500);
}

void readWeather() {
 
  if(esp.available() > 0) {
    desc = esp.readStringUntil('\n');
    int len = desc.length();
    desc = desc.substring(0, len - 1);
  }
  if(esp.available() > 0) {
    heat = esp.readStringUntil('\n');
    int len = heat.length();
    heat = heat.substring(0, len - 1);
  }
  if(esp.available() > 0) {
   humid = esp.readStringUntil('\n');
   int len = humid.length();
    humid = humid.substring(0, len -1);
  } 
}

void showWeather() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Ho Chi Minh city");
  lcd.setCursor(0,1);
  lcd.print(desc);
  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
   
  lcd.print("Heat: " + heat + "^C");
  
  lcd.setCursor(0,1);
  lcd.print("Humid: " + humid + "%");
  delay(1000);
}
   

void loop() {
  checkAlarm();
   if (esp.available() > 0) {
    command = esp.read();
    if (command == '1' || command == '2' || command == '3' || command == '4' || command == '5') {
      choice = command;
    }
  }
    if (choice == '1') {     
      showDateTime();
    } else if (choice == '2') {
      readWeather();    
      showWeather();
    } else if (choice == '3') {
      showTemperature();
    } else if (choice == '4') {
      getAlarmData();
    } else if (choice == '5') {
      dd = -1;
      digitalWrite(BUZZER_PIN,LOW);
      choice = '1';
    }

}
