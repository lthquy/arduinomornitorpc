# arduinomornitorpc
Using Arduino and LCD 2004 to display computer's system Information

# Introduction:
Using ESP32 and LCD2004 (I2C) to display system info.

The host PC must install HWiNFO32/64 and add-on Remote Sensor Monitor to feed json.

# Demo
https://youtu.be/gmVy-M73R-g

# Requirement:
- Install Library

  -- ArduinoJson
  
  -- LiquidCrystal_I2C
  
  -- WiFi
  
  -- Wire
  
- Installation install HWiNFO32/64 and add-on Remote Sensor Monitor in host PC, firewall open port 55555

- Disable monitoring and hide all sensor value which you don't want to display on LCD. This will reduce size of json file.

- Use http://arduinojson.org/assistant to compute the capacity json Buffer

- Open arduinomornitorpc.ino then update SensorName to match your system value
