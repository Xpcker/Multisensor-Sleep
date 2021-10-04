#include <WiFi.h>
#include <PubSubClient.h>

#include <ESP32Time.h>
ESP32Time rtc;
String timeStamp;

String __macA = String(WiFi.macAddress());
const char *mqtt_client_name = __macA.c_str();
String internetStatus = "";
WiFiClient wclient;

PubSubClient mqttClient(wclient);

void MQTTUnSubscribe();
void MQTTSubscriptions();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
bool mqttConnect();
uint16_t mqttPublish(String path, String msg);
int pubPackets = 0;
void MQTTUnSubscribe()
{
    String topicN = String("ntp/time");

    mqttClient.unsubscribe(topicN.c_str());
}
void MQTTSubscriptions()
{

    String topicN = String("ntp/time");

    mqttClient.subscribe(topicN.c_str());
}
void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    String messageDateTime;

    for (int i = 0; i < length; i++)
    {
        messageDateTime += (char)payload[i];
    }

    String rawNTP;
    rawNTP = messageDateTime;
    int a, m, d, h, n, s;
    a = rawNTP.substring(0, 4).toInt();
    m = rawNTP.substring(5, 7).toInt();
    d = rawNTP.substring(8, 10).toInt();
    h = rawNTP.substring(11, 13).toInt();
    n = rawNTP.substring(14, 16).toInt();
    s = rawNTP.substring(17, 19).toInt();
    rtc.setTime(s, n, h, d, m, a);
    timeStamp = rtc.getEpoch();

    String dateTime = rtc.getDateTime();
    Serial.println((String) "SETTING UP DATE AND TIME: " + dateTime);

    delay(1000);
}
void reconnect()
{
    // Loop until we're reconnected
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP32Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass))
        {
            Serial.println("Established:" + String(clientId));
            //mqttClient.subscribe("SmartTControl/data/v");
            MQTTSubscriptions();
            // return true;
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
bool mqttConnect()
{
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"; // For random generation of client ID.
    char clientId[9];

    uint8_t retry = 3;
    while (!mqttClient.connected())
    {
        // if (String(mqtt_server).length() <= 0)
        //     break;

        //mqttClient.setServer(mqtt_server, 1883);
        mqttClient.setServer(MQTT_HOST, mqtt_port);
        mqttClient.setCallback(callback);
        Serial.println(String("Attempting MQTT broker connection."));
        internetStatus = "Connecting...";

        for (uint8_t i = 0; i < 8; i++)
        {
            clientId[i] = alphanum[random(62)];
        }
        clientId[8] = '\0';

        if (mqttClient.connect(clientId, mqtt_user, mqtt_pass))
        {
            Serial.println("Established:" + String(clientId));
            internetStatus = "Connected";
            //mqttClient.subscribe("SmartTControl/data/v");
            MQTTSubscriptions();
            return true;
        }
        else
        {
            Serial.println("Connection failed:" + String(mqttClient.state()));
            internetStatus = "Not-Connected. Retrying...";
            if (!--retry)
                break;
            delay(3000);
        }
    }
    return false;
}

uint16_t mqttPublish(String path, String msg)
{
    //String path = String("channels/") + channelId + String("/publish/") + apiKey;
    Serial.print("Publishing to: ");
    Serial.println(path);
    mqttClient.publish(path.c_str(), msg.c_str());
    pubPackets++;
    return pubPackets;
}
