#include <WiFi.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

//WiFi Credentials
#define WIFI_SSID "ivy"
#define WIFI_PASSWORD "12345ivy"

//#define WIFI_SSID "equipos_medicos"
//#define WIFI_PASSWORD "Cl1n1c@2018."

String idSensor = "6";
#define MQTT_PUB_ID "Sensores/6"

//WiFi Connection Timeout
#define WIFI_TIMEOUT 1000 * 6 // 10seconds
int WiFiConnectionLostSince=0;
//MQTT Config

#define MQTT_HOST IPAddress(186, 64, 123, 26) // For a cloud MQTT broker, type the domain name  #define MQTT_HOST "example.com"
const int mqtt_port = 1883;
const char *mqtt_user = "mosquittoivy";
const char *mqtt_pass = "mosquittoivy!";
#include "MQTTHandler.h"
TimerHandle_t wifiReconnectTimer;
int modemInSleep = 0; //whether the wifi is in sleep or not

void connectToWifi()
{
  // if (modemInSleep == 0)
  // {
    Serial.print("Connecting to WiFi... ");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long startAttemptTime = millis(); // Keep track of when we started our attempt to get a WiFi connection
    while (WiFi.status() != WL_CONNECTED &&    // Keep looping while we're not connected AND haven't reached the timeout
           millis() - startAttemptTime < WIFI_TIMEOUT)
    {
      delay(10);
    }
    randomSeed(micros());
    if (WiFi.status() != WL_CONNECTED)
    { // Make sure that we're actually connected, otherwise go to deep sleep
      Serial.println("Connection Failed, Going to Sleep in connectToWifi");
    }
  //}
}

void setModemSleep()
{
  modemInSleep = 1;

  Serial.print("Turning off WiFi");
  // if (!setCpuFrequencyMhz(80)){
  //     //Serial2.println("Not valid frequency!");
  // }
  // Use this if 40Mhz is not supported
  // setCpuFrequencyMhz(80);
}

void wakeModemSleep()
{

  Serial.print("Waking Up WiFi");
  //setCpuFrequencyMhz(240);
  modemInSleep = 0;
}

void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event)
  {
  case SYSTEM_EVENT_STA_GOT_IP:
    //connectToMqtt();
    WiFiConnectionLostSince=0;
    //mqttConnect();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("WiFi lost connection");
    Serial.println("Trying to Reconnect");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFiConnectionLostSince++;
    if(WiFiConnectionLostSince==WIFI_TIMEOUT){
      Serial.println("Connection Not Restored, restarting ESP now...");
      ESP.restart();
    }

    break;
  }
}

/*
  WiFi Events
 
  0  SYSTEM_EVENT_WIFI_READY               < ESP32 WiFi ready
  1  SYSTEM_EVENT_SCAN_DONE                < ESP32 finish scanning AP
  2  SYSTEM_EVENT_STA_START                < ESP32 station start
  3  SYSTEM_EVENT_STA_STOP                 < ESP32 station stop
  4  SYSTEM_EVENT_STA_CONNECTED            < ESP32 station connected to AP
  5  SYSTEM_EVENT_STA_DISCONNECTED         < ESP32 station disconnected from AP
  6  SYSTEM_EVENT_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
  7  SYSTEM_EVENT_STA_GOT_IP               < ESP32 station got IP from connected AP
  8  SYSTEM_EVENT_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
  9  SYSTEM_EVENT_STA_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
  10 SYSTEM_EVENT_STA_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
  11 SYSTEM_EVENT_STA_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
  12 SYSTEM_EVENT_STA_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
  13 SYSTEM_EVENT_AP_START                 < ESP32 soft-AP start
  14 SYSTEM_EVENT_AP_STOP                  < ESP32 soft-AP stop
  15 SYSTEM_EVENT_AP_STACONNECTED          < a station connected to ESP32 soft-AP
  16 SYSTEM_EVENT_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
  17 SYSTEM_EVENT_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
  18 SYSTEM_EVENT_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
  19 SYSTEM_EVENT_GOT_IP6                  < ESP32 station or ap or ethernet interface v6IP addr is preferred
  20 SYSTEM_EVENT_ETH_START                < ESP32 ethernet start
  21 SYSTEM_EVENT_ETH_STOP                 < ESP32 ethernet stop
  22 SYSTEM_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
  23 SYSTEM_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
  24 SYSTEM_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
  25 SYSTEM_EVENT_MAX

  */
