// References
// https://github.com/lesterlo/Notched-Shaft-Encoder
// https://forum.arduino.cc/index.php?topic=681420.0
// https://www.skaarhoj.com/FreeStuff/GraphicDisplayImageConverter.php
// https://github.com/jasonacox/TM1637TinyDisplay
// ==============================INCLUDES=========================================

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Arduino.h>
#include <Wire.h>
#include "RTClib.h"
#include <time.h>

#include <MsTimer2.h>
#include <NSEncoder.h>

#include <TM1637TinyDisplay.h>
// ======================================================================
// Display
// Instantiation and pins configurations
// Pin 12 - > DIO
// Pin 13 - > CLK
#define CLK 13
#define DIO 12

// ======================================================================
// Rotary Encoder Inputs
#define ENCODER_S1_PIN 3
#define ENCODER_S2_PIN 4
#define ENCODER_KEY_PIN 2

// *******************************ENUMS**********************************

typedef enum
{
  Running,
  Set_Hours,
  Set_Minutes
} ClockStates;

// *******************************FIELDS*********************************
// Clock
RTC_DS1307 rtc;
DateTime LastTime;
ClockStates ClockState;
ClockStates LastClockState;

// Initialize TM1637TinyDisplay
TM1637TinyDisplay display(CLK, DIO);

// **********************************************************************
// Rotary encoder
//                    A,               B,             Btn, StepsPerNotch, EnableDoublePress
NSEncoder_btn enc(ENCODER_S1_PIN, ENCODER_S2_PIN, ENCODER_KEY_PIN, 4, false);
int enc_position = 0;
int Last_enc_position = 0;
String display_str = "Open";
char TimeString[4];
char LastTimeString[4];

//Timer isr
void isr_mstimer2()
{
  enc.btn_task();
}

void setup() {
  while (!Serial); // for Leonardo/Micro/Zero
  {
    Serial.begin(57600);
  }

  // **********************************************************************
  //Clock
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");

    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // **********************************************************************
  // Display
  display.setBrightness(0x02);
  delay(1000);

  LastTimeString[0] = ' ';
  LastTimeString[1] = ' ';
  LastTimeString[2] = ' ';
  LastTimeString[3] = ' ';

  // **********************************************************************
  // Rotary encoder
  // Set the button monitoring parameter

  enc.setBtnDebounceInterval(30);
  enc.setBtnPressInterval(500);
  enc.setBtnHoldInterval(1000);

  //You must setup this timer to enable button monitoring
  MsTimer2::set(enc.getBtnDebounceInterval(), isr_mstimer2); //Set 30 ms clock
  MsTimer2::start();

  ClockState = Running;
  Serial.println("Clock: Running");
}

void loop() {
  DateTime now = rtc.now();

  String h = "";
  String m = "";

  // **********************************************************************
  NSEncoder_btn::BTN_STATE enc_btn;

  if (enc.get_Position(enc_position)) //If value is updated
  {
    Serial.print("Enc. Pos: ");
    Serial.println(enc_position);

    if (enc_position > Last_enc_position)
    {
      Serial.println("INCREASE");

      uint32_t UnixTime = now.unixtime();
      switch (ClockState)
      {
        case Set_Hours:
          rtc.adjust(UnixTime + 3600);
          break;
        case Set_Minutes:
          rtc.adjust(UnixTime + 60);
          ResetSeconds(rtc.now());
          break;
      }
    }
    else
    {
      Serial.println("DECREASE");

      uint32_t UnixTime = now.unixtime();
      switch (ClockState)
      {
        case Set_Hours:
          rtc.adjust(UnixTime - 3600);
          break;
        case Set_Minutes:
          rtc.adjust(UnixTime - 60);
          ResetSeconds(rtc.now());
          break;
      }
    }

    now = rtc.now();
    Last_enc_position = enc_position;
  }

  enc_btn = enc.get_Button();
  switch (enc_btn)
  {
    case NSEncoder_btn::OPEN:
      display_str = "Open";
      break;

    case NSEncoder_btn::PRESSED:
      display_str = "Pressed";

      switch (ClockState)
      {
        case Set_Hours: // Hours -> Minutes
          ClockState = Set_Minutes;
          break;
        case Set_Minutes: // Minutes -> Running
          ClockState = Running;
          break;
      }
      break;

    case NSEncoder_btn::RELEASED:
      display_str = "Released";
      break;

    case NSEncoder_btn::HELD:
      display_str = "Held";
      switch (ClockState)
      {
        case Running:
          ClockState = Set_Hours;
          break;
        default:
          ClockState = Running;
          break;
      }
      break;

    case NSEncoder_btn::DOUBLE_PRESSED:
      display_str = "Double Pressed";
      break;
  }

  if (enc_btn != NSEncoder_btn::OPEN)
  {
    Serial.print("Btn status: ");
    Serial.println(display_str);
  }

  if (LastTime != now)
  {
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.println(now.second(), DEC);

    // HOUR
    if (now.hour() < 10)
    {
      TimeString[0] = ' ';

      h = String(now.hour());
      TimeString[1] = h[0];
    }
    else if (now.hour() > 12)
    {
      h = String(now.hour() - 12);
      TimeString[0] = ' ';
      TimeString[1] = h[0];
    }
    else
    {
      h = String(now.hour());
      TimeString[0] = h[0];
      TimeString[1] = h[1];
    }

    // MINUTE
    if (now.minute() < 10)
    {
      TimeString[2] = '0';

      m = String(now.minute());
      TimeString[3] = m[0];
    }
    else
    {
      m = String(now.minute());
      TimeString[2] = m[0];
      TimeString[3] = m[1];
    }

    LastTime = now;
  }

  if (LastTimeString[0] == ' ' && LastTimeString[1] == ' ' && LastTimeString[2] == ' ' && LastTimeString[3] == ' ' ||
      LastTimeString[0] != TimeString[0] || LastTimeString[1] != TimeString[1] || LastTimeString[2] != TimeString[2] || LastTimeString[3] != TimeString[3])
  {
    display.showNumberDec(now.minute(), 0, true, 2, 2);
    
    if (now.hour() > 12)
    {
      display.showNumberDec(now.hour() - 12, 0x40, false, 2, 0);
    }
    else  
    {
      display.showNumberDec(now.hour(), 0x40, true, 2, 0);
    }
    
    LastTimeString[0] = TimeString[0];
    LastTimeString[1] = TimeString[1];
    LastTimeString[2] = TimeString[2];
    LastTimeString[3] = TimeString[3];

    Serial.println("Time to display: " + (String)LastTimeString[0] + (String)LastTimeString[1] + ":" + (String)LastTimeString[2] + (String)LastTimeString[3]);
  }

  if (LastClockState != ClockState)
  {
    switch (ClockState)
    {
      case Running:
        Serial.println("Clock: Running");
        break;
      case Set_Hours:
        Serial.println("Clock: Set_Hours");
        break;
      case Set_Minutes:
        Serial.println("Clock: Set_Minutes");
        break;
    }
    LastClockState = ClockState;
    delay(500);
  }
  //Serial.println("=======================================================");
}

void ResetSeconds(DateTime now)
{
  rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute(), 0));
}
