#ifndef ESP_MQTT_CLIENT_H
#define ESP_MQTT_CLIENT_H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

#define MQTT_MAX_PAYLOAD_SIZE                 256 // Maximum payload size, must correspond to MQTT_MAX_PACKET_SIZE of PubSubClient.h
#define MQTT_CONNECTION_RETRY_DELAY           10 * 1000
#define MQTT_MAX_TOPIC_SUBSCRIPTION_LIST_SIZE 10
#define MAX_DELAYED_EXECUTION_LIST_SIZE       10

typedef void(*ConnectionEstablishedCallback) ();
typedef void(*MessageReceivedCallback) (const String &message);
typedef void(*DelayedExecutionCallback) ();

class EspMQTTClient {
private:
  bool mWifiConnected;
  unsigned long mLastWifiConnectionAttemptMillis;
  unsigned long mLastWifiConnectionSuccessMillis;
  bool mMqttConnected;
  unsigned long mLastMqttConnectionMillis;

  const char* mWifiSsid;
  const char* mWifiPassword;

  const char* mMqttServerIp;
  const short mMqttServerPort;
  const char* mMqttUsername;
  const char* mMqttPassword;
  const char* mMqttClientName;

  ConnectionEstablishedCallback mConnectionEstablishedCallback;

  const bool mEnableWebUpdater;
  const bool mEnableSerialLogs;
 
  ESP8266WebServer* mHttpServer;
  ESP8266HTTPUpdateServer* mHttpUpdater;

  WiFiClient* mWifiClient;
  PubSubClient* mMqttClient;

  struct TopicSubscriptionRecord {
    String topic;
    MessageReceivedCallback callback;
  };
  TopicSubscriptionRecord mTopicSubscriptionList[MQTT_MAX_TOPIC_SUBSCRIPTION_LIST_SIZE];
  byte mTopicSubscriptionListSize;

  struct DelayedExecutionRecord {
    unsigned long targetMillis;
    DelayedExecutionCallback callback;
  };
  DelayedExecutionRecord mDelayedExecutionList[MAX_DELAYED_EXECUTION_LIST_SIZE];
  byte mDelayedExecutionListSize = 0;

public:
  EspMQTTClient(
    const char wifiSsid[], const char* wifiPassword,
    ConnectionEstablishedCallback connectionEstablishedCallback, const char* mqttServerIp, const short mqttServerPort = 1883,
    const char* mqttUsername = "", const char* mqttPassword = "", const char* mqttClientName = "ESP8266",
#ifdef ESPMQTTCLIENT_WEB_AND_LOGS_OFF_BY_DEFAULT
      const bool enableWebUpdater = false, const bool enableSerialLogs = false);
#elif defined ESPMQTTCLIENT_WEB_OFF_BY_DEFAULT
      const bool enableWebUpdater = false, const bool enableSerialLogs = true);
#else
      const bool enableWebUpdater = true, const bool enableSerialLogs = true);
#endif
  ~EspMQTTClient();

  // Main routine
  void loop();
  bool isConnected() const;

  // MQTT interface
  void publish(const String &topic, const String &payload, bool retain = false);  // Sends the string to the topic
  void subscribe(const String &topic, MessageReceivedCallback messageReceivedCallback); // Subscribes to the topic and registers the callback
  void unsubscribe(const String &topic);  //Unsubscribes from the topic and removes the callback from the list

  // Delayed functions
  void executeDelayed(const long delay, DelayedExecutionCallback callback);

private:
  void connectToWifi();
  void connectToMqttBroker();
  void mqttMessageReceivedCallback(char* topic, byte* payload, unsigned int length);
};

#endif