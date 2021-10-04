/*
 *

 V18.19.3

- Removed RTOS

 V18.19.2

- Resolved wifi and mqtt disconnect issues

 V18.19.1
- removed everything related to NTP, DateTime comes from MQTT 
- Cleaned Code.
- removed OTA
- removed Sleep
- removed watchdog 


to Fix:

- device doesnt disconnects from wifi

-  when WiFi disconnects the ESP gets stuck in:
18:14:21.170 -> [D][WiFiGeneric.cpp:374] _eventCallback(): Event: 5 - STA_DISCONNECTED
18:14:21.170 -> [W][WiFiGeneric.cpp:391] _eventCallback(): Reason: 201 - NO_AP_FOUND
18:14:21.170 -> [WiFi-event] event: 5

- When the sensor boots sometimes it doesn't retrieve inmediately the timestamp from nodered, is it possible to add like a "wait until timestamp arrives, then continue"?

- Timestamp is not recording properly into influx I think



V18.19
- Removed FreeRTOS tasks, used virtual safe tasks.
V.18.16
    - Corrected reboot issue

 V.18.15
    - Corrected wifi crash
 
 V.18.14
  - Enabled OTA
  - Enabled WiFi.onEvent(WiFiEvent)
  - Reboot issue resolved
  - SPIFFS now working with RTOS
  - MQTT Publish function (publisher()) is now being handled by a separate timer
  - Note: SPIFFS is now storing a JSON of 6-10 objects on average. In order to increase the publishing time
          and data storage change eventTime_2_mqttPub, WDT_TIMEOUT and TIME_TO_SLEEP values. 
  - This version is tested with sensors
  
  V.18.13
  - Disabled OTA
  - disabled WiFi.onEvent(WiFiEvent);

  V.18.12
  -SPIFFS Error resolved, if SPIFFS is not formatted, the FW will auto-format it and try again
  -ESP32 Modem(WiFi+BT) will remain normally in sleep and will wake up when data needs to be published after 3 minutes

  
  V.18.11
  - Added unsigned long currentTime = 0;   , wasn't declared
  - in String readFileContent(), commented the return; it was crashing the compiler.




  V.18.10
  -Updated the FW to get sensor reading every 2 seconds
  -Append the Data to SPIFFS file on each reading
  -After 1000*60*3= 3 Minutes send the data to NodeRED and clear the file
  -Updated NodeRED flow to format the data, convert it into a matrix and send to GSheet


   V.18.9
  NTP moves to RTC
   
  
  V.18.8
  NTP arrives from MQTT
  
  
  V.18.7
  corrected NTP 
  added firmware version, ssid, ip, sensor number to setup
  
  V.18.6
  NTP time
    added to paylaod
  
  V.18.5
  Code cleaned
  
  V.18.4
  added watchdog
  
  V.18.3
  added KeepWiFi Alive
 
  V.18.2
  Sleep mode with interval
  
  V.18
  All code rewritten.

  V.16
  added 10 sensors
  added MultiWiFi
  Battery optimization
     commented non necessary calculations

  V.15
  added Watchdog

  V.14
  commented and removed from send data
  humidity
  atmospheric pressure
  battery voltage

  V.13
  Rearrange code in headers
  added readSensors
  added readNoise
  added wireless
  added googleScript

  V0.12
  Added Hostname, Commented it.
  Updated GScript
  OTA
  BH1750 Correction Multiplier Factor  (usually 1.2)
  removed unnecesarry IR filters for INMP441 and Corrected selected filter

  V0.11
  Go to Sleep if no connection.

  V0.1
  Wifi timeout
  Connection to Google Sheets
  Sensors:
  BH1750
  INM411
  BMW280
*/

#include <Arduino.h>
#include "neoTimer.h"
#include "SoftwareStack.h"
#include "readSensors.h"
#include "readNoise.h"
#include "wireless.h"

String FIRMWARE_VERSION = "0.18.19.3";

//SPIFFS
#include "storageHandler.h"
const long interval = 2000;                 // Interval at which to read sensors
Neotimer dataAcqTimer = Neotimer(interval); // Set timer's preset to 1s

unsigned long wifiCheckInterval = 2000;
unsigned long previousMillis = 0;
//TIMERS
unsigned long eventTime_2_mqttPub = 1000 * 12 * 1;     // Interval at which to publish data(after every 1 minutes)
Neotimer mqttPubTimer = Neotimer(eventTime_2_mqttPub); // Set timer's preset to 1s

RTC_DATA_ATTR int bootCount = 0;
TimerHandle_t sendDataTimer;
uint8_t writingToFile = 1;
bool wifi_connected = false;
void publisher()
{

  writingToFile = 0;
  //wakeModemSleep();
  Serial.print("Sending data: ");
  appendToFile("}"); //closes the JSON
  Serial.println(readFileContent());
  if (wifi_connected == true)
  {
    uint16_t packetIdPub1 = mqttPublish(String(MQTT_PUB_ID), readFileContent());
    Serial.printf("Publishing on topic %s at QoS 2, packetId: %i", MQTT_PUB_ID, packetIdPub1);
  }
  else
  {
    //don't do anything
  }
  Serial.println();
  Serial.print("Clearing the file");
  clearFile();
  startFileWithBracket();
  writingToFile = 1;
  setModemSleep(); //turn off WiFi
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  setupSPIFFS();
  startFileWithBracket();
  Wire.begin();
  lightMeter.begin(); //Initialize Sensors
  bme.begin(0x76);

  WiFi.onEvent(WiFiEvent);
  connectToWifi();
  mqttConnect();
  //setModemSleep(); //turn off WiFi

  delay(500);

  Serial.println("-------------------- SETUP --------------------");

  Serial.print("Sensor ID: ");
  Serial.println(idSensor);
  Serial.print("Firmware Version :");
  Serial.println(FIRMWARE_VERSION);
  Serial.print("WiFi MAC Address :");
  Serial.println(WiFi.macAddress());
  Serial.print("Connected to WiFi :");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void loop()
{
  if (mqttPubTimer.repeat())
  {
    Serial.println("Calling Publisher");
    publisher();
  }

  if (dataAcqTimer.repeat())
  {

    //Read Sensors and String Values
    Serial.println();
    Serial.println("----- Reading Sensors -----");
    readLux();
    readBME();
    readNoise();
    readBatt();
    timeStamp = rtc.getEpoch();
    Serial.println((String) "DateTime:" + timeStamp);
    Serial.println();
    String lux_s(lux);
    String temp_s(temp);
    String noise_s(noise);
    String batt_s(batt);
    String timeStamp_s(timeStamp);

    if (writingToFile == 1)
    {
      //Create Payload for MQTT
      String payload = "{";
      payload += "\"Sensor\":\"" + idSensor + "\",";
      payload += "\"timeStamp\":\"" + timeStamp_s + "\",";
      payload += "\"Lux\":\"" + lux_s + "\",";
      payload += "\"Temp\":\"" + temp_s + "\",";
      payload += "\"Noise\":\"" + noise_s + "\",";
      payload += "\"Battery\":\"" + batt_s + "\"";
      payload += "}";
      appendToFile(payload);
      payload = ""; //empty the variable
    }
  }

  if (!mqttClient.connected())
  {
    if (wifi_connected == true)
    {

      reconnect();
    }
  }
  if (wifi_connected == true)
  {
    mqttClient.loop();
  }
  unsigned long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= wifiCheckInterval))
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    wifi_connected = false;
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    wifi_connected = true;
  }
  if (WiFi.status() == WL_IDLE_STATUS)
  {
    ESP.restart();

    delay(1000);
  }
}
