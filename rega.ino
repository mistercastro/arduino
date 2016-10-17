/*
#####################################################################################
#  	File:               Arduino_Watering_sustem.ino                                             
#       Processor:          Arduino UNO, MEGA ou Teensy++ 2.0      
#  	Language:	    Wiring / C /Processing /Fritzing / Arduino IDE          
#						
#	Objectives:         Watering System - Irrigation
#										  
#	Behavior:	    When the soil is dry,
#                           
#
#			
#   Author:                 Hugo Castro 
#   Date:                   27/02/15	
#   place:                  Portugal	
#					
#####################################################################################
 
  This project contains public domain code.
  The modification is allowed without notice.
  
 */

// libraries definition
#include <Wire.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"


// Struct
struct Irrigation {
    int ontime;
    int offtime;
    int pinNr;
    int frequency;
  };

// Variables
// pins
const int buttonPin = 8;     // the number of the pushbutton pin
const char *daysOfWeekStrings[7] = {"Dom, ",
                                    "Seg, ",
                                    "Ter, ",
                                    "Qua, ",
                                    "Qui, ",
                                    "Sex, ",
                                    "Sab, "};
int _day;
int _month;
int _year;
int _dayOfWeek;
int _hour;
int _minute;
int _second;
int _timeNow; // 10:00 => 1000

// uma pumpN pode ser um array com n horarios
Irrigation pump1 = {2100,2200,7,B0101010}; // sab;sex;qui:qua;ter;seg;dom
Irrigation pump2 = {2100,2200,3,B0101010};
Irrigation pump3 = {2100,2200,3,B0101010};
Irrigation pump4 = {2100,2200,3,B0101010};
Irrigation pumps[]={pump1,pump2,pump3,pump4};
int buttonState = 0;         // variable for reading the pushbutton status
int totalPumps = 4; // para remover e usar o sizeof 
// system messages
const char *string_table[] =
{   
  "Welcome! =)",
  "The water pump is on",
  "Watering System",
  "Please wait!"
};

// objects definition
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
RTC_DS1307 RTC;

void setup(){
  // serial initialization
  Serial.begin(9600);
  
  // LCD initialization
  lcd.begin(20,2);
  lcd.backlight();     // with Backlight
  
  // Wire initialization
  Wire.begin();
  
  // RTC initialization
  RTC.begin();
  
  if (!RTC.isrunning()){
    // date and time adjust as the PC computer date and time
    Serial.println("date and time adjust as the PC computer date and time");
    RTC.adjust(DateTime(__DATE__, __TIME__)); 
  }
  
  // Arduino pins initalization
  Serial.println("size =" +  sizeof(pumps));
  for (int i = 0; i < sizeof(pumps); i++){
    pinMode(pumps[i].pinNr,OUTPUT);    
  }
  
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  
  // LCD initial messages
  lcd.setCursor(0,0);
  lcd.print(string_table[2]); 
  Serial.println(string_table[2]);
  lcd.setCursor(0,1);
  lcd.print(string_table[3]);
  Serial.println(string_table[3]);  
  // initialization delay
  delay(5000);
  lcd.clear();
}

void loop(){
  ActualizeDateTime();
  WriteHour();  
  WritePumpState();
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (buttonState == HIGH) {  
    if (digitalRead(pump1.pinNr) == HIGH) {
      digitalWrite(pump1.pinNr,LOW);   
    }else{
      digitalWrite(pump1.pinNr,HIGH);
    }
  }
  pumpOnOff();
}

void WritePumpState()
{
//  String result1 = " P1|P2|P3|P4 ";
  String result = "";
  for (int i = 0; i < sizeof(pumps); i++){
    if (digitalRead(pumps[i].pinNr) == HIGH){
      result += i+1 + "-1;";
    }else{
      result += i+1 + "-0";  
    }
  }
  lcd.setCursor(12, 1);
  lcd.print(result);
}

// Real Time Clock Function
void ActualizeDateTime()
{
  // RTC parameters definition
  DateTime Now = RTC.now();
  _day = Now.day();
  _month = Now.month();
  _year = Now.year();
  _dayOfWeek = Now.dayOfWeek();
  _hour = Now.hour();
  _minute = Now.minute();
  _second = Now.second();
  _timeNow = _hour*100+_minute;
}

void WriteHour()
{
  String clock_date = "";
  String clock_hour = "";

  clock_date += daysOfWeekStrings[_dayOfWeek];
  clock_date += fixZero(_day);
  clock_date += "/";
  clock_date += fixZero(_month);
  clock_date += "/";
  clock_date += _year;
  
  clock_hour += fixZero(_hour);
  clock_hour += ":";
  clock_hour += fixZero(_minute);
  clock_hour += ":";
  clock_hour += fixZero(_second);
  
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(clock_date);
  Serial.println(clock_date);
  lcd.setCursor(0, 1);
  lcd.print(clock_hour);
  Serial.println(clock_hour);
  delay(250);
}


String fixZero(int i)
{
  String ret;
  if (i < 10) ret += "0";
  ret += i;
  return ret;
}

void pumpOnOff ()
{
  for (int thisPump = 0; thisPump < totalPumps; thisPump++) {
    Irrigation pump = pumps[thisPump];
    //((pump.dayOfWeek[_dayOfWeek] == "1") && (pump.HHstart == _hour) && (pump.MMstart == _minute) && (_second >= 0) && (_second <= 2))
    // if ((pump.dayOfWeek[_dayOfWeek] == "1") && (pump.ontime == _time.now)) {
    if (((int) pow(2, _dayOfWeek) & pump.frequency) && (pump.ontime == _timeNow)) {
      if (digitalRead(pump.pinNr) == LOW) {
	digitalWrite(pump.pinNr,HIGH);   
      }
    }else{
      // if ((pump.dayOfWeek[_dayOfWeek] == "1") && (pump.off == _time.now)) {
      if (((int) pow(2, _dayOfWeek) & pump.frequency) && (pump.offtime == _timeNow)) {
	if (digitalRead(pump.pinNr) == HIGH) {
	  digitalWrite(pump.pinNr,LOW);
	}
      }
    }
  }
}

/* Local Variables: */
/* mode: c++ */
/* End: */
