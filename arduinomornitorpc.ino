/********************************************************************************
Arduinomornitorpc
https://github.com/lthquy/arduinomornitorpc
Created by Luu Thanh Quy deatheyes at gmail.com
---------------------------------------------------------------------------------
Copyright © 2017 Luu Thanh Quy
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
---------------------------------------------------------------------------------
Change Activity:
    Date       Description
   ------      -------------
  17 Dec 2017  Created.
********************************************************************************/


/********************************************************************************
                                    includes
********************************************************************************/

#define ARDUINOJSON_ENABLE_PROGMEM 0
#define ARDUINOJSON_DEFAULT_NESTING_LIMIT 50

#include <WiFi.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid     = "Wifi Name";
const char* password = "wifi Password";
const char* host = "192.168.1.100"; // IP address your PC


// Variable for storing data decode from json
int  loadcpu = 0; // %
int tempcpu = 0; // *C
String vcore = ""; // volt
int  powercpu = 0; // watt
int fancpu = 0; // RPM
int tempgpu = 0; // *C
int fangpu = 0; // RPM
int fansys = 0; // RPM
int powergpu = 0; // W
String dlrate = ""; // KB/s
String ulrate = ""; //KB/s

// 4 line to diplay on LCD
char line0[20];
char line1[20];
char line2[20];
char line3[20];



void setup()
{
  Serial.begin(115200);
  delay(10);
  lcd.begin();
  lcd.backlight();
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int value = 0;
void loop(){
  
    unsigned long timeout = millis();
    // Turn off LCD backlight after 4 minutes connect fail
    if (value > 240) lcd.noBacklight(); 
    // Display "PC OFF" when connect to PC fail
    if (value > 1) {
      lcd.clear();
      lcd.setCursor(7, 1);
      lcd.print("PC OFF");
    }

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 55555;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    ++value;
    return;

  // Connect success
  //Serial.println(F("Connected!"));
  value = 0;
  lcd.backlight();

  // Send HTTP request
  client.println(F("GET / HTTP/1.0"));
  client.println(F("Host: "+*host)); 
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    return;
  }

  // Check HTTP status
  char status[32] = {0};
  client.readBytesUntil('\r', status, sizeof(status));
  if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
    Serial.print(F("Unexpected response: "));
    Serial.println(status);
    return;
  }

  // Skip HTTP headers
  char endOfHeaders[] = "\r\n\r\n";
  if (!client.find(endOfHeaders)) {
    Serial.println(F("Invalid response"));
    return;
  }

  // Allocate JsonBuffer
  // Use arduinojson.org/assistant to compute the capacity.
  const size_t capacity = JSON_ARRAY_SIZE(18) + 18*JSON_OBJECT_SIZE(6);
  DynamicJsonBuffer jsonBuffer(capacity);

  // Parse JSON Array
  JsonArray& root = jsonBuffer.parseArray(client);
  if (!root.success()) {
    Serial.println(F("Parsing array failed!"));
    return;
  }

  // Decode JSON data
  int numberE = root.size();
  for (int i2 = 0; i2 < numberE; i2++)
  {
    String objec = root[i2];
    // Parse JSON Object
    JsonObject& sensorval = jsonBuffer.parseObject(objec);
    if (!sensorval.success()) {
      Serial.println(F("Parsing obj failed!"));
      return;
    }
    if (sensorval["SensorName"].as<String>() == "CPU Package") tempcpu = sensorval["SensorValue"].as<int>();
    if (sensorval["SensorName"].as<String>() == "Total CPU Usage") loadcpu = sensorval["SensorValue"].as<int>();
    if (sensorval["SensorName"].as<String>() == "Vcore") vcore = sensorval["SensorValue"].as<String>();
    if (sensorval["SensorName"].as<String>() == "CPU Package Power") powercpu = sensorval["SensorValue"].as<int>();
    if (sensorval["SensorName"].as<String>() == "CPU-FAN") fancpu = sensorval["SensorValue"].as<int>(); 
    if (sensorval["SensorName"].as<String>() == "System 1") fansys = sensorval["SensorValue"].as<int>(); 
    if (sensorval["SensorName"].as<String>() == "GPU Temperature") tempgpu = sensorval["SensorValue"].as<int>();
    if (sensorval["SensorName"].as<String>() == "GPU Fan") fangpu = sensorval["SensorValue"].as<int>();
    if (sensorval["SensorName"].as<String>() == "GPU Power") powergpu = sensorval["SensorValue"].as<int>();
    if (sensorval["SensorName"].as<String>() == "Current DL rate") dlrate = formatBytes(sensorval["SensorValue"].as<int>()); // multiply 1024 in HWinfo customize value
    if (sensorval["SensorName"].as<String>() == "Current UP rate") ulrate = formatBytes(sensorval["SensorValue"].as<int>()); // multiply 1024 in HWinfo customize value
  }

  sprintf(line0,"CPU %3d%% %3d%cC %3dW",loadcpu,tempcpu,(char)223,powercpu);
  //Serial.print(line0);
  //Serial.print("\n");
  sprintf(line1,"%5sV%4dRPM%4dRP",vcore.c_str(),fansys,fancpu);
  //Serial.print(line1);
  //Serial.print("\n");
  sprintf(line2,"GPU %3dW %2d%cC %4dR",powergpu,tempgpu,(char)223,fangpu);
  //Serial.print(line2);
  //Serial.print("\n");
  sprintf(line3,"D %7s U %6s",dlrate.c_str(),ulrate.c_str());
  //Serial.print(line3);
  //Serial.print("\n");
  
  lcd.setCursor(0, 0);
  lcd.print(line0);
  lcd.setCursor(0, 1);
  lcd.print(line1);
  lcd.setCursor(0, 2);
  lcd.print(line2);
  lcd.setCursor(0, 3);
  lcd.print(line3);
  client.flush();
  // Disconnect
  //client.stop();
  //Serial.println();
  //Serial.println("closing connection");
}
  ++value;
  if (millis() - timeout < 999)  delay(999-(millis() - timeout));
}

//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}
