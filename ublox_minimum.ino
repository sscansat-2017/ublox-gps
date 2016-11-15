#include <SoftwareSerial.h>
#include "TinyGPS++.h"

TinyGPSPlus gps;
SoftwareSerial ss(4, 3);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  ss.begin(9600);
}

void loop() {
  /// Feed the GPS parser.
  while (ss.available() > 0)
    gps.encode(ss.read());
  
  if (gps.satellites.isUpdated()) {
    Serial.print("SAT="); Serial.println(gps.satellites.value());
  }
    
  if (gps.location.isUpdated())
  {
    Serial.print("LAT="); Serial.print(gps.location.lat(), 6);
    Serial.print("LNG="); Serial.println(gps.location.lng(), 6);
  }
}
