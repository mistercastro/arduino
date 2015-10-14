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
struct irrigation {
  irrigation (int hStart, int mStart, int hEnd, int mEnd, const char* frequency, int pinNr) :
    HHstart(hStart), MMstart(mStart), HHend(hEnd), MMend(mEnd), _dayOfWeek(frequency), _pinNr(pinNr) 
  {}

  int HHstart;
  int MMstart;
  int HHend;
  int MMend;
  char *_dayOfWeek[7]; //dias da semana em que rega a bomba
  int _pinNr;

private:
  irrigation(const irrigation&);
  irrigation& operator=(const irrigation & other);
};

// Variables
// pins
const int pump1Pin = 7; // the number of the reley 1
const int pump2Pin = 3;// the number of the reley 2
const int pump3Pin = 3;// the number of the reley 3
const int pump4Pin = 3;// the number of the reley 4
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

// uma pumpN pode ser um array com n horarios
irrigation pump1(21,0,22,0,pump1Pin{"1","0","1","0","1","0","1"});
irrigation pump2(21,0,22,0,pump2Pin,{"1","0","1","0","1","0","1"});
irrigation pump3(21,0,22,0,pump3Pin,{"1","0","1","0","1","0","1"});
irrigation pump4(21,0,22,0,pump4Pin,{"1","0","1","0","1","0","1"});
irrigation pumps[]={pump1,pump2,pump3,pump4};
int buttonState = 0;         // variable for reading the pushbutton status
int totalPumps = 4;

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
  pinMode(pump1Pin,OUTPUT);
  pinMode(pump2Pin,OUTPUT);
  pinMode(pump3Pin,OUTPUT);
  pinMode(pump4Pin,OUTPUT);
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
    if (digitalRead(pump1Pin) == HIGH) {
      digitalWrite(pump1Pin,LOW);   
    }else{
      digitalWrite(pump1Pin,HIGH);
    }
  }
  pumpOnOff();
}

void WritePumpState()
{
//  String result1 = " P1|P2|P3|P4 ";
  String result2 = "";
  int valPump1 = digitalRead(pump1Pin);
//  int valPump2 = digitalRead(pump2Pin);
  if(valPump1 == HIGH)
  {
    result2 += "ON ";
  }else{
    result2 += "OFF";  
  }
  lcd.setCursor(12, 1);
  lcd.print(result2);
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
    irrigation pump = pumps[thisPump];

    if ((pump.dayOfWeek[_dayOfWeek] == "1") && (pump.HHstart == _hour) && (pump.MMstart == _minute) && (_second >= 0) && (_second <= 2)) {      
      if (digitalRead(pump.pinNr) == LOW) {
	digitalWrite(pump.pinNr,HIGH);   
      }
    }else{
      if ((pump.dayOfWeek[_dayOfWeek] == "1") && (pump.HHend == _hour) && (pump.MMend == _minute)) {
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
