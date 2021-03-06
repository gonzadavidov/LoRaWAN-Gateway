#include <Arduino.h>

/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the (early prototype version of) The Things Network.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1,
 *  0.1% in g2).
 *
 * Change DEVADDR to a unique address!
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *******************************************************************************/

// LoRaTx Includes
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// Application LoRa definitions
#include <lora_def.h>

// GPS Includes
#include <NMEAGps.h>
#include <NeoSWSerial.h>
#include <GPSport.h>


// GPS Constants and variables

// Neo6m Constants 
#define NEO6M_BAUDRATE 9600
// GPS Handling variables
NMEAGPS  gps;                   // This parses the GPS characters
gps_fix  fix;                   // This holds on to the latest values
float latitude, longitude;
bool validData = false;

// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the prototype TTN
// network initially.

static const PROGMEM u1_t NWKSKEY[16] = { 0xE9, 0x84, 0x85, 0xC7, 0x19, 0xD9, 0x84, 0x9C, 0x21, 0x0E, 0x3B, 0x2F, 0x04, 0x29, 0x63, 0x4C };

// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the prototype TTN
// network initially.
static const u1_t PROGMEM APPSKEY[16] = { 0x8D, 0xBB, 0x4A, 0x52, 0x05, 0x7A, 0x8D, 0xF1, 0x00, 0x4C, 0x2A, 0xCF, 0xC1, 0xA3, 0xD9, 0x0B };

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
static const u4_t DEVADDR = 0x26013253; // <-- Change this address for every node!

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static char sendBuffer[LORA_MSG_LEN + 1] = "0,0,0";
static char auxBuf[10];
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 30;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, sendBuffer, strlen(sendBuffer), 0);
        Serial.println(F("Packet queued"));
    }
}

void onEvent (ev_t ev) {
    if (ev == EV_TXCOMPLETE)
    {
        if(LMIC.dataLen) 
        {
            // data received in rx slot after tx
            Serial.print(F("Data Received: "));
            Serial.write(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
            Serial.println();
        }
        // Schedule next transmission
        os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
    }
}

void setup() {

    Serial.begin(115200);
    Serial.println(F("Starting"));

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

    // LMIC init
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    #ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
    // If not running an AVR with PROGMEM, just use the arrays directly 
    LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set.

    // Single Channel Opeartion, disable the other channels
    for (uint8_t i = 0; i < 9; i++) {
        if (i != LORA_CHANNEL) {
            LMIC_disableChannel(i);
        }
    }
    // LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    // LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    // LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    // LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    // LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    // LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    // LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    // LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band

    // TTN defines an additional channel at 869.525Mhz using SF9 for class B
    // devices' ping slots. LMIC does not have an easy way to define set this
    // frequency and support for class B is spotty and untested, so this
    // frequency is not configured here.

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(SPREADING_FACTOR,14);

    gpsPort.begin(NEO6M_BAUDRATE);  // Start GPS

}

void loop() {
    os_runloop_once();

    // currentTime = millis();
    if(true)
    // while (gps.available( gpsPort) )
    { // check for gps data 
        fix = gps.read();
        if(true)
        // if (fix.valid.location)
        {  // encode gps data 
            latitude = fix.latitude();
            longitude = fix.longitude();
            dtostrf(latitude, 5, 5, auxBuf);
            sprintf(sendBuffer, "%s", auxBuf);
            dtostrf(longitude, 5, 5, auxBuf);
            sprintf(sendBuffer + strlen(sendBuffer), ",%s,%d", auxBuf,MEASUREMENT_SITE);
            Serial.print(F("Latitude: "));
            Serial.print(latitude);
            Serial.print(F(", longitude: "));
            Serial.print(longitude);        
            Serial.print(F(". Measured and buffered: "));
            Serial.println(sendBuffer);

            if (!validData)
            {
                // Start job
                do_send(&sendjob);
                validData = true;
            }
        }
    }
}