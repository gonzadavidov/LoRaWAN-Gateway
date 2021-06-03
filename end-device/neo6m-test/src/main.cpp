#include <Arduino.h>

// GPS Includes
#include <NMEAGps.h>
#include <NeoSWSerial.h>
#include <GPSport.h>

// Neo6m Constants 
#define NEO6M_BAUDRATE 9600

// GPS Handling variables
NMEAGPS  gps;                   // This parses the GPS characters
gps_fix  fix;                   // This holds on to the latest values

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));

  gpsPort.begin(NEO6M_BAUDRATE);
}

void loop() {
  while (gps.available( gpsPort )) {
    fix = gps.read();

    if (fix.valid.location) {
      Serial.print( F("Location: ") );

      Serial.print( fix.latitude(), 6 );
      Serial.print( ',' );
      Serial.print( fix.longitude(), 6 );
    }

    if (fix.valid.altitude)
    {
      Serial.print( F(", Altitude: ") );
      Serial.print( fix.altitude() );
    }
    Serial.print( F("Status: ") );
    Serial.print( fix.status );

    Serial.println("");
  }
}