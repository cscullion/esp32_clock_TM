#include <Arduino.h>
#include <TM1637Display.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "secrets.h"

// Module connection pins (Digital Pins)
#define CLK 23
#define DIO 22

const int LED_PIN = 2;		// lights up when connecting to wifi

// WiFi network name and password:
const char *ssid = WIFI_SSI;
const char *wifipw = WIFI_PASSWORD;

TM1637Display display(CLK, DIO);

void setTimezone(String timezone){
  Serial.printf("  Setting Timezone to %s\n",timezone.c_str());
  setenv("TZ",timezone.c_str(),1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

void initTime(String timezone){
  struct tm timeinfo;

  Serial.println("Setting up time");
  configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
  if(!getLocalTime(&timeinfo)){
    Serial.println("  Failed to obtain time");
    return;
  }
  Serial.println("  Got the time from NTP");
  // Now we can set the real timezone
  setTimezone(timezone);
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time 1");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");
}

void  startWifi(){
	digitalWrite(LED_PIN, HIGH); // LED on
	WiFi.begin(ssid, wifipw);
	Serial.println("Connecting Wifi");
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		delay(500);
	}
	Serial.print("Wifi RSSI=");
	Serial.println(WiFi.RSSI());
	digitalWrite(LED_PIN, LOW); // LED off
}

void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst){
  struct tm tm;

  tm.tm_year = yr - 1900;   // Set date
  tm.tm_mon = month-1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;      // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}


void setup()
{
	Serial.begin(115200);
	Serial.setTimeout(10000);

  pinMode(LED_PIN, OUTPUT);

  // Connect to the WiFi network
  
	startWifi();

	initTime("EST5EDT,M3.2.0,M11.1.0");   // Set for New_York. see https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
	printLocalTime();
}

void loop()
{
	struct tm timeinfo;
	int lHour = 0;
	int lMin = 0;
	int timeDec = 0;
  uint8_t ch = 0;
  
	display.setBrightness(0x0f);

	if (WiFi.status() != WL_CONNECTED) {
		startWifi();
	}

	printLocalTime();
	getLocalTime(&timeinfo);
	lHour = timeinfo.tm_hour;
	if (lHour > 12) {
		lHour = lHour - 12;
	}
	lMin = timeinfo.tm_min;

	timeDec = (lHour * 100) + lMin;
	display.showNumberDecEx(timeDec, 0b11100000, false, 4, 0);

/*
  if (Serial.available() > 0) {
    ch = Serial.read();
    Serial.printf("read %c\n", ch);
  }
*/

  delay(2000);
}
