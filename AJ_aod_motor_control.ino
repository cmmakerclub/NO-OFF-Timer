
#if defined(ESP8266)
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif

/* for normal hardware wire use below */
#include <Wire.h> // must be included here so that Arduino library object file references work
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);
/* for normal hardware wire use above */


#define _aDay     86400000
#define _aHour    3600000
#define _aMin     60000
#define _aSec     1000

#define SW_0    9
#define SW_1    8
#define SW_2    7
#define SW_3    6
#define SW_4    10
#define SW_5    11
#define SW_6    12
#define SW_7    13
#define SW_8    14
#define SW_9    15
#define SW_10   16
#define SW_11   17

#define LED_G   5
#define LED_R   3
#define SSR     4

#define LED_G_on()     analogWrite(LED_G, 20)
#define LED_G_off()    analogWrite(LED_G, 0)
#define LED_R_on()     analogWrite(LED_R, 1)
#define LED_R_off()    analogWrite(LED_R, 0)

#define LED_SSR_on()     digitalWrite(SSR, 1)
#define LED_SSR_off()    digitalWrite(SSR, 0)

#define MODE_normal   0
#define MODE_fail     1
int LED_G_status = 1;

uint8_t PIN_SW[] = {SW_0, SW_1, SW_2, SW_3, SW_4, SW_5, SW_6, SW_7, SW_8, SW_9, SW_10, SW_11};

int H_time = 0;
int mode = MODE_normal;
uint16_t SW_status;
void setup ()
{
  delay(10);
  setup_HW();
  LED_G_on();
  SW_status = 0;
  for (int x = 0; x < 12; x++) SW_status = (SW_status << 1) | (1 - digitalRead(PIN_SW[x]));
  Serial.println(SW_status, BIN);

  mode = MODE_normal;
  if (!Rtc.IsDateTimeValid())
  {
    // Common Cuases:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
    mode =  MODE_fail;
  }
}
uint32_t time_now, time_prev1, time_prev2, time_prev3, time_prev4;
void loop ()
{
  time_now = millis();

  if (time_now < time_prev1) {
    time_prev1 = time_now;
    time_prev2 = time_now;
    time_prev3 = time_now;
    time_prev4 = time_now;
  }

  if (time_now - time_prev2 >= _aSec) {
    time_prev2 = time_now;
    SW_status = 0;
    for (int x = 0; x < 12; x++) SW_status = (SW_status << 1) | (1 - digitalRead(PIN_SW[x]));

    int motor = 0;
    if (H_time < 2) {
      if (SW_status & (0x01 << 11))motor = 1;
    } else if (H_time < 4) {
      if (SW_status & (0x01 << 10))motor = 1;
    } else if (H_time < 6) {
      if (SW_status & (0x01 << 9))motor = 1;
    } else if (H_time < 8) {
      if (SW_status & (0x01 << 8))motor = 1;
    } else if (H_time < 10) {
      if (SW_status & (0x01 << 7))motor = 1;
    } else if (H_time < 12) {
      if (SW_status & (0x01 << 6))motor = 1;
    } else if (H_time < 14) {
      if (SW_status & (0x01 << 5))motor = 1;
    } else if (H_time < 16) {
      if (SW_status & (0x01 << 4))motor = 1;
    } else if (H_time < 18) {
      if (SW_status & (0x01 << 3))motor = 1;
    } else if (H_time < 20) {
      if (SW_status & (0x01 << 2))motor = 1;
    } else if (H_time < 22) {
      if (SW_status & (0x01 << 1))motor = 1;
    } else {
      if (SW_status & (0x01 << 0))motor = 1;
    }
    
    if (mode ==  MODE_fail)motor = 0;
    
    if (motor) {
      LED_R_on();
      LED_SSR_on();
    } else {
      LED_R_off();
      LED_SSR_off();
    }

    Serial.print("Mode : ");
    Serial.print(mode);
    Serial.print("\tH : ");
    Serial.print(H_time);
    Serial.print("\tDIP_SW : ");
    Serial.println(SW_status, BIN);

  }

  if (time_now - time_prev3 >= _aMin) {
    time_prev3 = time_now;
    mode = MODE_normal;
    if (!Rtc.IsDateTimeValid())
    {
      // Common Cuases:
      //    1) the battery on the device is low or even missing and the power line was disconnected
      Serial.println("RTC lost confidence in the DateTime!");
      mode =  MODE_fail;
    }

    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    Serial.print("\t");

    RtcTemperature temp = Rtc.GetTemperature();
    Serial.print(temp.AsFloat());
    Serial.println("C");

    H_time = now.Hour();
  }

  if ((time_now - time_prev4) >= (_aSec / 4)) {
    time_prev4 = time_now;

    if (mode == MODE_normal) {
      LED_G_on();
    } else {
      LED_G_status = 1 - LED_G_status;
      if (LED_G_status) {
        LED_G_off();
      } else {
        LED_G_on();
      }
    }
  }

}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
  char datestring[20];

  snprintf_P(datestring,
             countof(datestring),
             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
             dt.Month(),
             dt.Day(),
             dt.Year(),
             dt.Hour(),
             dt.Minute(),
             dt.Second() );
  Serial.print(datestring);
}

void setup_HW(void) {

  Serial.begin(115200);
  for (int x = 0; x < 12; x++) pinMode(PIN_SW[x], INPUT_PULLUP);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(SSR, OUTPUT);
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  //--------RTC SETUP ------------
  Rtc.Begin();

  // if you are using ESP-01 then uncomment the line below to reset the pins to
  // the available pins for SDA, SCL
  // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  printDateTime(compiled);
  Serial.println();

  if (!Rtc.IsDateTimeValid())
  {
    // Common Cuases:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing
    Serial.println("RTC lost confidence in the DateTime!");
    // following line sets the RTC to the date & time this sketch was compiled
    // it will also reset the valid flag internally unless the Rtc device is
    // having an issue
    Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);

  H_time = now.Hour();
}

