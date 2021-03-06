# LoRaWAN-Gateway

This repository is organised as follows:
* [end-device:](./end-device/) Contains PlatformIO projects used in the end device. One is for testing the GPS sensor and the other is the used in the final implementation.
* [gateway:](./gateway/) Contains code that runs in the Raspberry Pi:
  * Packet Forwarder: receives the LoRaWAN packets and sends them to ChirpStack on the local network.
  * [http-integration:](./gateway/http-integration/main.py) Receives the payload from the ChirpStack Application Server and sends the data to ThingSpeak.
* website: Both backend and frontend of the website ([live-test](https://lora-location-tracker.herokuapp.com/)) using the Google Maps API.