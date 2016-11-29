#include <i2c_t3.h>
#include <TinyGPS++.h>
#include <teenkSeries.h>
#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>

File myFile;
File count_file;
TinyGPSPlus gps;
kSeries K_30;

const int chipSelect = BUILTIN_SDCARD;

int led = 13;
time_t last_updated = 0;
String current_run = "nil";

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void setup()
{
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);

  // Open serial communications to computer.
  Serial.begin(9600);

  // Set up hardware serial for GPS.
  Serial2.begin(9600);

  // set the Time library to use Teensy's RTC to keep time
  setSyncProvider(getTeensy3Time);
  Teensy3Clock.set(0);
  setTime(0);
  Serial.println("time init");

  if (!SD.begin(chipSelect)) {
    Serial.println("sd fail");
    critical_error(50);
    return;
  }

  // Extract current run for use with SD card saving.
  count_file = SD.open("runcount");
  String line = "";
  if (count_file) {
    while (count_file.available()) {
      line = count_file.readStringUntil('\n');
    }
  }
  else {
    Serial.println("count file fail");
    critical_error(50);
  }
  count_file.close();
  current_run = String(line.toInt() + 1);
  bool err = SD.remove("runcount");
  if (err != true) {
    Serial.println("count file fail");
    critical_error(50);
  }
  count_file = SD.open("runcount", FILE_WRITE);
  count_file.print(current_run);
  count_file.close();
  Serial.println("sd init");
  Serial.print("run "); Serial.println(current_run);
}

void loop()
{
  // Feed the GPS parser.
  while (Serial2.available() > 0)
    gps.encode(Serial2.read());

  // Wait for GPS time to update.
  time_t rtc = now();
  if (rtc <= last_updated) {
    return;
  }

  last_updated = rtc;

  myFile = SD.open(current_run.c_str(), FILE_WRITE);
  if (!myFile) {
    Serial.print("error opening "); Serial.println(1);
  }

  // Time since start of experiment.
  Serial.print("RTC="); Serial.println(rtc);
  myFile.print("RTC="); myFile.println(rtc);

  // GPS time.
  int gpstime = gps.time.value();
  Serial.print("GPSTIME="); Serial.println(gpstime);
  myFile.print("GPSTIME="); myFile.println(gpstime);

  // Other GPS data.
  int gpssat = gps.satellites.value();
  Serial.print("GPSSAT="); Serial.println(gpssat);
  myFile.print("GPSSAT="); myFile.println(gpssat);
  float gpslat = gps.location.lat();
  Serial.print("GPSLAT="); Serial.println(gpslat, 8);
  myFile.print("GPSLAT="); myFile.println(gpslat, 8);
  float gpslng = gps.location.lng();
  Serial.print("GPSLNG="); Serial.println(gpslng, 8);
  myFile.print("GPSLNG="); myFile.println(gpslng, 6);
  float gpsalt = gps.altitude.meters();
  Serial.print("GPSALT="); Serial.println(gpsalt);
  myFile.print("GPSALT="); myFile.println(gpsalt);

  // If sec % 2 = 0, Collect CO2 data.
  if ((rtc % 2) == 0) {
    int co2 = K_30.getCO2('p');
    Serial.print("K30CO2="); Serial.println(co2);
    myFile.print("K30CO2="); myFile.println(co2);
  }

  // Collect accelerometer data.
  // TODO HERE

  Serial.println("END");
  myFile.println("END");

  // Write data out to SD card.
  myFile.close();

  // Blink LED to show data has been updated.
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
}

void critical_error(int ms) {
  while (true) {
    digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(ms);               // wait for a second
    digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
    delay(ms);
  }
}

