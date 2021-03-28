#include "Arduino.h"
#include "BLEDevice.h"
#include "Actuators.h"
#include "config.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class plant{

  public:
  int maxLightLux;
  int minLightLux;
  int maxTemp;
  int minTemp;
  int maxEnvHumid;
  int minEnvHumid;
  int maxSoilMoist;
  int minSoilMoist;
  int maxSoilEc;
  int minSoilEc;
  
};


void connectWifi();
void disconnectWifi();
void connectMqtt();
void disconnectMqtt();
//BLEClient* getFloraClient(BLEAddress floraAddress);
//BLERemoteService* getFloraService(BLEClient* floraClient);
bool forceFloraServiceDataMode(BLERemoteService* floraService);
bool readFloraDataCharacteristic(BLERemoteService* floraService, String baseTopic);
bool readFloraBatteryCharacteristic(BLERemoteService* floraService, String baseTopic);
bool processFloraService(BLERemoteService* floraService, char* deviceMacAddress, bool readBattery);
bool processFloraDevice(BLEAddress floraAddress, char* deviceMacAddress, bool getBattery, int tryCount);
void hibernate();
void delayedHibernate(void *parameter);
void callback(char* topic, byte* payload, unsigned int length);
int readFloraMoisture(BLERemoteService* floraService);
int readFloraLight(BLERemoteService* floraService);
int readFloraTemperature(BLERemoteService* floraService);
int readFloraSoilConductivity(BLERemoteService* floraService);
void adjustSpecificParameters(int index, BLEAddress floraAddress);
BLEClient* getFloraClient(BLEAddress floraAddress);
BLERemoteService* getFloraService(BLEClient* floraClient);
PubSubClient getClient();
bool getAdjParams();
NTPClient getNTPClient();
