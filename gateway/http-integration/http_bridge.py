from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs

from chirpstack_api.as_pb import integration
from google.protobuf.json_format import Parse

import json

import requests

class Handler(BaseHTTPRequestHandler):
    # True -  JSON marshaler
    # False - Protobuf marshaler (binary)
    json = True

    def do_POST(self):
        self.send_response(200)
        self.end_headers()
        query_args = parse_qs(urlparse(self.path).query)

        content_len = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_len)

        if query_args["event"][0] == "up":
            self.up(body)

        elif query_args["event"][0] == "join":
            self.join(body)

        else:
            print("handler for event %s is not implemented" % query_args["event"][0])

    def up(self, body):
        up = self.unmarshal(body, integration.UplinkEvent())
        print("Uplink received from: %s with payload: %s" % (up.dev_eui.hex(), up.data))
        # Obtain latitude and longitude from payload
        coords = up.data.decode("utf-8").split(",")
        print("SNR:", up.rx_info[0].lora_snr)
        print("RSSI:",up.rx_info[0].rssi)
        sf = up.tx_info.lora_modulation_info.spreading_factor
        print(f"Lat: {coords[0]}, Long: {coords[1] if len(coords) > 1 else 0}")
        # Send data to ThingsSpeak
        print("Posting data to ThingSpeak...")
        url = "https://api.thingspeak.com/update.json"
        body = {
            "api_key": api_key,
            "field1": sf,
            "field2": up.rx_info[0].rssi,
            "field3": up.rx_info[0].lora_snr,
            "field4": f"{coords[2]}",
            "lat": f"{coords[0]}",
            "long": f"{coords[1]}"
        }
        print(f"Body: {body}")
        r = requests.post(url, data=body)
        print(f'Response: {r.text}')

    def join(self, body):
        join = self.unmarshal(body, integration.JoinEvent())
        print("Device: %s joined with DevAddr: %s" % (join.dev_eui.hex(), join.dev_addr.hex()))

    def unmarshal(self, body, pl):
        if self.json:
            return Parse(body, pl)

        pl.ParseFromString(body)
        return pl

api_key = ""
with open('config.json') as config_file:
    data = json.load(config_file)
    port = data['port']
    api_key = data['write_api_key']
    httpd = HTTPServer(('', port), Handler)
    print(f'Server starting to listen on port {port}')
    httpd.serve_forever()




