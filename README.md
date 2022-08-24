# RingSurveillanceStationBridge

I use both a Ring alarm system and Synology Surveillance Station. I got tired of having to manually set both when I left the house, and while Synology's DSCam app can theoretically turn "home mode" on/off using geofencing, this feature never worked reliably for me. I spent a long time looking for a way to integrate Ring and Surveillance Station via API, but Ring does not expose an API. There are a few projects that attempt to interact with Ring using the web interface and pretending to be a human user, but these all have pitfalls such as not working if you have 2FA turned on or breaking frequently as Ring updates their website.

My solution was to use a ESP32 coupled to my Ring Alarm Keypad to detect alarm status changes and call the Surveillance Station API to toggle "home mode" on or off. 

## Hardware Setup:

You will need:
- An ESP32 derivative or similar with 5v support
- A Ring Alarm keypad paired with your Ring Alarm
- 28AWG wire
- Photoresistors (only needed if going the non-permanent route)

I chose to permanently attach the ESP32 to a spare Ring keypad I had. Doing this requires you to open the keypad case, which due to the design is a destructive process. Once open, you need to solder a wire to the negative side of each of the mode status LEDs. Not that these LED packages are actually two LEDs in one; you want to solder the wires to the **bottom** negative pad of each. You will also need to solder a wire to a ground connector (there is a GND test point pad in the top middle of the board) to a +5v source (there is a +5V test pad on the bottom right of the board). Once done, you should have 5 wires coming off the Ring keypad PCB.

If you prefer to not destroy a keypad in the process, you can instead wireup 3 photoresistors between a ground on the ESP32 and the input pins used on the ESP32. If you go this route, you will need to find a way to attach the photoresistors to the front of the keypad mode buttons and shield them from external light.

Regardless of which option you go with, you will need to turn on "power save" mode for the Keypad from within the Ring app. This ensures the mode buttons only light up when the alarm's mode is changed, and not when the keypad detects motion.

## Software setup:

Once you have downloaded the sourcecode and opened it in Arduino IDE, you need to set a few values:
- ssid - this is the name of a WiFi network the ESP32 will connect to. It needs to be able to reach your Synology from whatever network it joins.
- password - Password for the WiFi.
- ssServer - This is the full DNS name and port of your Synology.
- ssuser - The username for a user you have setup in Surveillance Station with the rights to turn home mode on and off.
- ssuserpass - Password of the user
- disarmedPin - The pin you have connected to the "Disarmed" LED or photoresistor
- homePin - The pin you have connected to the "Home Mode" LED or photoresistor
- awayPin - The pin you have connected to the "Away Mode" LED or photoresistor
- rootCACertificate - This is a base64 encoded representation of the Root CA certificate that issued the TLS certificate your Synology is using

If you don't already have them installed, you will need to download these libraries:

- WiFi.h
- HTTPClient.h
- WiFiClientSecure.h
- time.h
- Arduino_JSON.h

That's it, compile and upload the code and then wire your ESP32 up to your Ring keypad. You may want to leave the ESP32 connected to your computer initially and monitor the output on the serial console. This will report any errors connecting to the WiFi network or your Synology.
