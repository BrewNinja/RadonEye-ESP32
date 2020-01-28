RadonEye RD200 sensors for the ESP32/ESPHome platform

To use, edit radoneye.h and at the top, change the mac address to be the mac address of your radoneye. Upload the radoneye.h file to the your ESPHome directory (/config/esphome on hassio), then create a new device. Use the sample radoneye.yaml file to create the sensors.

Initial code from https://community.home-assistant.io/u/wettermann/

TODO: 
  - Pass in MAC address from yaml instead of hardcode in .h file (not sure how to do this)
  - ~~Catch values < 0 or > 400~~
