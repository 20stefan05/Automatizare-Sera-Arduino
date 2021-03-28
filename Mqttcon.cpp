#include "Mqttcon.h"
#include <bits/stdc++.h>
#include "config.h"
// boot count used to check if battery status should be read
//RTC_DATA_ATTR int bootCount = 0;
bool adjParams = false;
// device count
//static int deviceCount = sizeof FLORA_DEVICES / sizeof FLORA_DEVICES[0];
const long utcOffsetInSeconds = 2*3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
int checkLight = 0;
// the remote service we wish to connect to
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");

// the characteristic of the remote service we are interested in
static BLEUUID uuid_version_battery("00001a02-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");

const char*   WIFI_SSID       = "stefan-X541NA";
const char*   WIFI_PASSWORD   = "gN5BthOG";

// MQTT topic gets defined by "<MQTT_BASE_TOPIC>/<MAC_ADDRESS>/<property>"
// where MAC_ADDRESS is one of the values from FLORA_DEVICES array
// property is either temperature, moisture, conductivity, light or battery

const char*   MQTT_HOST       = "broker.mqttdashboard.com";
const int     MQTT_PORT       = 1883;
const char*   MQTT_CLIENTID   = "miflora-client";
const char*   MQTT_USERNAME   = "username";
const char*   MQTT_PASSWORD   = "password";
const String  MQTT_BASE_TOPIC = "flora"; 
const int     MQTT_RETRY_WAIT = 5000;

plant* PLANTS[3];

char* TOPICS[] = {"L", "V", "F", "H"};

//TaskHandle_t hibernateTaskHandle = NULL;

WiFiClient espClient;
PubSubClient client(espClient);

void connectWifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("");
}

void disconnectWifi() {
  WiFi.disconnect(true);
  Serial.println("WiFi disonnected");
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


  if(topic[0]=='L'&& payload[0] == '1') lightOn(topic[1]-'0');
    else if(topic[0]=='L'&& payload[0] == '0') lightOff(topic[1]-'0');
    else if(topic[0]=='V'&& payload[0] == '1') waterOn(topic[1]-'0'); 
    else if(topic[0]=='V'&& payload[0] == '0') waterOff(topic[1]-'0');
    else if(topic[0]=='F'&& payload[0] == '1') fertilizerOn(topic[1]-'0');
    else if(topic[0]=='F'&& payload[0] == '0') fertilizerOff(topic[1]-'0');
    else if(topic[0]=='H'&& payload[0] == '1') heaterOn();
    else if(topic[0]=='H'&& payload[0] == '0') heaterOff();
    else if(topic[0] == 'C' && payload[0] == '1') client.publish("CheckCon", "Confirm");
    else if(topic[0] == 'M'){ checkLight = payload[0] - '0'; Serial.println("Primit prin mqtt:"); Serial.println(payload[0] - '0');}
    else if(topic[0] == 'A' && payload[0] == '1') adjParams = true;
    else if(topic[0] == 'A' && payload[0] == '0') adjParams = false;// client.publish("V0", "0", true);}
    else if(topic[0] == 'R' && payload[0] == '1') {Serial.println("Entering setup()");ESP.restart();}//setup();}
    else if(topic[0]=='P'){
      
       std::string PARAMETERS[10];
       int PARAMETERS_INT[10];
       int k = 0;
       for(int i = 0;i<10; i++){
        while(payload[k]!=' '){
          
          //PARAMETERS[i]+=(char)payload[k];
          PARAMETERS[i].push_back((char)payload[k]);
          if(payload[k]<'0'|| payload[k]>'9') break;
          
          k++;
        }
        k++;
       }
       for(int i = 0; i<10; i++){
        std::stringstream tostring(PARAMETERS[i]); 
        tostring>> PARAMETERS_INT[i];
        //PARAMETERS_INT[i] = std::stoi(PARAMETERS[i]);//atoi(PARAMETERS[i]);
        
       }
       
       plant* p = new plant();
       p->maxLightLux = PARAMETERS_INT[0];
       p->minLightLux = PARAMETERS_INT[1];
       p->maxTemp = PARAMETERS_INT[2];
       p->minTemp = PARAMETERS_INT[3];
       p->maxEnvHumid = PARAMETERS_INT[4];
       p->minEnvHumid = PARAMETERS_INT[5];
       p->maxSoilMoist = PARAMETERS_INT[6];
       p->minSoilMoist = PARAMETERS_INT[7];
       p->maxSoilEc = PARAMETERS_INT[8];
       p->minSoilEc = PARAMETERS_INT[9];
       
       PLANTS[topic[1] - '0'] = p;
       Serial.println(topic[1] - '0');
    }
}


void connectMqtt() {
  Serial.println("Connecting to MQTT...");
  client.setServer(MQTT_HOST, MQTT_PORT);
  int trycon = 0;
  while (!client.connected()) {
    if (!client.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.print("MQTT connection failed:");
      Serial.print(client.state());
      Serial.println("Retrying...");
      trycon++;
      if(trycon == 3) ESP.restart();
      delay(MQTT_RETRY_WAIT);
    }
  }
  
   for(int i = 0; i<(sizeof TOPICS/ sizeof TOPICS[0]); i++){
    for(int j = 0; j<10; j++){
        std::string str = "";//"flora/Stefan/";
        char ac = TOPICS[i][0];
        int a = int(ac);
        str.push_back(a);
        str.push_back(j+'0');
        client.subscribe(str.c_str());
        
    }
  }
  for(int i = 0; i<10; i++){
    
    std::string str = "";//"flora/Stefan/";
    str.push_back('P');
    
    str.push_back(i+'0');
    client.subscribe(str.c_str());
    
    
  }
  client.subscribe("MLight");client.subscribe("ReqData"); client.subscribe("CheckCon"); client.subscribe("AdjParams"); 
  client.setCallback(callback);
  
  //client.setCallback(callback);
  Serial.println("MQTT connected");
  Serial.println("");
}

void disconnectMqtt() {
  client.disconnect();
  Serial.println("MQTT disconnected");
}

BLEClient* getFloraClient(BLEAddress floraAddress) {
  BLEClient* floraClient = BLEDevice::createClient();

  if (!floraClient->connect(floraAddress)) {
    Serial.println("- Connection failed, skipping");
    return nullptr;
  }

  Serial.println("- Connection successful");
  return floraClient;
}

BLERemoteService* getFloraService(BLEClient* floraClient) {
  BLERemoteService* floraService = nullptr;

  try {
    floraService = floraClient->getService(serviceUUID);
  }
  catch (...) {
    // something went wrong
  }
  if (floraService == nullptr) {
    Serial.println("- Failed to find data service");
  }
  else {
    Serial.println("- Found data service");
  }

  return floraService;
}

bool forceFloraServiceDataMode(BLERemoteService* floraService) {
  BLERemoteCharacteristic* floraCharacteristic;
  
  // get device mode characteristic, needs to be changed to read data
  Serial.println("- Force device in data mode");
  floraCharacteristic = nullptr;
  try {
    floraCharacteristic = floraService->getCharacteristic(uuid_write_mode);
  }
  catch (...) {
    // something went wrong
  }
  if (floraCharacteristic == nullptr) {
    Serial.println("-- Failed, skipping device");
    return false;
  }

  // write the magic data
  uint8_t buf[2] = {0xA0, 0x1F};
  floraCharacteristic->writeValue(buf, 2, true);

  delay(500);
  return true;
}

bool readFloraDataCharacteristic(BLERemoteService* floraService, String baseTopic) {
  BLERemoteCharacteristic* floraCharacteristic = nullptr;

  // get the main device data characteristic
  Serial.println("- Access characteristic from device");
  try {
    floraCharacteristic = floraService->getCharacteristic(uuid_sensor_data);
  }
  catch (...) {
    // something went wrong
  }
  if (floraCharacteristic == nullptr) {
    Serial.println("-- Failed, skipping device");
    return false;
  }

  // read characteristic value
  Serial.println("- Read value from characteristic");
  std::string value;
  try{
    value = floraCharacteristic->readValue();
  }
  catch (...) {
    // something went wrong
    Serial.println("-- Failed, skipping device");
    return false;
  }
  const char *val = value.c_str();

  Serial.print("Hex: ");
  for (int i = 0; i < 16; i++) {
    Serial.print((int)val[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  int16_t* temp_raw = (int16_t*)val;
  float temperature = (*temp_raw) / ((float)10.0);
  Serial.print("-- Temperature: ");
  Serial.println(temperature);

  int moisture = val[7];
  Serial.print("-- Moisture: ");
  Serial.println(moisture);

  int light = val[3] + val[4] * 256;
  Serial.print("-- Light: ");
  Serial.println(light);
 
  int conductivity = val[8] + val[9] * 256;
  Serial.print("-- Conductivity: ");
  Serial.println(conductivity);

  if (temperature > 200) {
    Serial.println("-- Unreasonable values received, skip publish");
    return false;
  }

  char buffer[64];

  snprintf(buffer, 64, "%f", temperature);
  client.publish((baseTopic + "temperature").c_str(), buffer); 
  Serial.println("Publicat la: ");
  Serial.print((baseTopic + "temperature").c_str());
  snprintf(buffer, 64, "%d", moisture); 
  client.publish((baseTopic + "moisture").c_str(), buffer);
  snprintf(buffer, 64, "%d", light);
  client.publish((baseTopic + "light").c_str(), buffer);
  snprintf(buffer, 64, "%d", conductivity);
  client.publish((baseTopic + "conductivity").c_str(), buffer);
  //client.publish("test", "smth");
  return true;
}
//------------------------------------------------------------------------------------------
int readFloraTemperature(BLERemoteService* floraService) {
  BLERemoteCharacteristic* floraCharacteristic = nullptr;

  // get the main device data characteristic
  try {
    floraCharacteristic = floraService->getCharacteristic(uuid_sensor_data);
  }
  catch (...) {
    // something went wrong
  }
  if (floraCharacteristic == nullptr) {
    return 200;// false;
  }

  // read characteristic value
  std::string value;
  try{
    value = floraCharacteristic->readValue();
  }
  catch (...) {
    // something went wrong
 
    return 200;// false;
  }
  const char *val = value.c_str();

  int16_t* temp_raw = (int16_t*)val;
  float temperature = (*temp_raw) / ((float)10.0);
  return temperature;
}
//-----------------------------------------------------------------------------------------
int readFloraMoisture(BLERemoteService* floraService) {
  BLERemoteCharacteristic* floraCharacteristic = nullptr;

  // get the main device data characteristic
  try {
    floraCharacteristic = floraService->getCharacteristic(uuid_sensor_data);
  }
  catch (...) {
    // something went wrong
  }
  if (floraCharacteristic == nullptr) {
    return 10000;// false;
  }

  // read characteristic value
  std::string value;
  try{
    value = floraCharacteristic->readValue();
  }
  catch (...) {
    // something went wrong
    
    return 10000;// false;
  }
  const char *val = value.c_str();
  int moisture = val[7];
  return moisture;
}
//-----------------------------------------------------------------------------------------------------------
int readFloraLight(BLERemoteService* floraService) {
  BLERemoteCharacteristic* floraCharacteristic = nullptr;

  // get the main device data characteristic
  try {
    floraCharacteristic = floraService->getCharacteristic(uuid_sensor_data);
  }
  catch (...) {
    // something went wrong
  }
  if (floraCharacteristic == nullptr) {
    return 100000;// false;
  }

  // read characteristic value
  std::string value;
  try{
    value = floraCharacteristic->readValue();
  }
  catch (...) {
    // something went wrong
    return 100000;// false;
  }
  const char *val = value.c_str();
  int light = val[3] + val[4] * 256;
  return light;
}
//----------------------------------------------------------------------------------------------------------
int readFloraSoilConductivity(BLERemoteService* floraService) {
  BLERemoteCharacteristic* floraCharacteristic = nullptr;

  // get the main device data characteristic
  try {
    floraCharacteristic = floraService->getCharacteristic(uuid_sensor_data);
  }
  catch (...) {
    // something went wrong
  }
  if (floraCharacteristic == nullptr) {
    return 200;// false;
  }

  // read characteristic value
  std::string value;
  try{
    value = floraCharacteristic->readValue();
  }
  catch (...) {
    // something went wrong
 
    return 200;// false;
  }
  const char *val = value.c_str();
  int conductivity = val[8] + val[9] * 256;
  return conductivity;
}




bool readFloraBatteryCharacteristic(BLERemoteService* floraService, String baseTopic) {
  BLERemoteCharacteristic* floraCharacteristic = nullptr;

  // get the device battery characteristic
  Serial.println("- Access battery characteristic from device");
  try {
    floraCharacteristic = floraService->getCharacteristic(uuid_version_battery);
  }
  catch (...) {
    // something went wrong
  }
  if (floraCharacteristic == nullptr) {
    Serial.println("-- Failed, skipping battery level");
    return false;
  }

  // read characteristic value
  Serial.println("- Read value from characteristic");
  std::string value;
  try{
    value = floraCharacteristic->readValue();
  }
  catch (...) {
    // something went wrong
    Serial.println("-- Failed, skipping battery level");
    return false;
  }
  const char *val2 = value.c_str();
  int battery = val2[0];

  char buffer[64];

  Serial.print("-- Battery: ");
  Serial.println(battery);
  snprintf(buffer, 64, "%d", battery);
  client.publish((baseTopic + "battery").c_str(), buffer);
  Serial.println("Battery at:");
  Serial.println((baseTopic + "battery").c_str());
  return true;
}

bool processFloraService(BLERemoteService* floraService, char* deviceMacAddress, bool readBattery) {
  // set device in data mode
  if (!forceFloraServiceDataMode(floraService)) {
    return false;
  }

  String baseTopic = MQTT_BASE_TOPIC + "/" + deviceMacAddress + "/";
  bool dataSuccess = readFloraDataCharacteristic(floraService, baseTopic);

  bool batterySuccess = true;
  if (readBattery) {
    batterySuccess = readFloraBatteryCharacteristic(floraService, baseTopic);
  }

  return dataSuccess && batterySuccess;
}

bool processFloraDevice(BLEAddress floraAddress, char* deviceMacAddress, bool getBattery, int tryCount) {
  Serial.print("Processing Flora device at ");
  Serial.print(floraAddress.toString().c_str());
  Serial.print(" (try ");
  Serial.print(tryCount);
  Serial.println(")");

  // connect to flora ble server
  BLEClient* floraClient = getFloraClient(floraAddress);
  if (floraClient == nullptr) {
    return false;
  }

  // connect data service
  BLERemoteService* floraService = getFloraService(floraClient);
  if (floraService == nullptr) {
    floraClient->disconnect();
    return false;
  }

  // process devices data
  bool success = processFloraService(floraService, deviceMacAddress, getBattery);

  // disconnect from device
  floraClient->disconnect();

  return success;
}

void hibernate() {
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ll);//SLEEP_DURATION * 1000000ll
  Serial.println("Going to sleep now.");
  delay(100);
  esp_deep_sleep_start();
}

void delayedHibernate(void *parameter) {
  delay(EMERGENCY_HIBERNATE*1000); // delay for five minutes
  Serial.println("Something got stuck, entering emergency hibernate...");
  hibernate();
}
PubSubClient getClient(){

  return client;
  
}
void adjustSpecificParameters(int index, BLEAddress floraAddress){
  Serial.println("Adjusting parameters");
  BLEClient* floraClient = getFloraClient(floraAddress);
   if (floraClient == nullptr) {
    return;
  }
  BLERemoteService* floraService = getFloraService(floraClient);
   if (floraService == nullptr) {
    floraClient->disconnect();
    return;
  }
  int moisture = readFloraMoisture(floraService);
   
  int light = readFloraLight(floraService);
   
  int conductivity = readFloraSoilConductivity(floraService);
   
  int temperature  = readFloraTemperature(floraService);
  if(PLANTS[index] == nullptr) return;
  int minLight = PLANTS[index]->minLightLux, minMoist = PLANTS[index]->minSoilMoist, minConductivity = PLANTS[index]->minSoilEc, minTemp = PLANTS[index]-> minTemp;
  
  //if(temperature==200)  ESP.restart();//setup();
  
  //if(moisture!=10000){
  
  if(moisture<minMoist){ waterOn(index);std::string str = "";str.push_back('V'); str.push_back(index+'0'); client.publish(str.c_str(), "1", true);}
  else if(moisture>=minMoist){ waterOff(index);std::string str = "";str.push_back('V'); str.push_back(index+'0'); client.publish(str.c_str(), "0", true);}
  //}
  //if(light!=100000){
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  String formattedDate = timeClient.getFormattedTime();
  Serial.println(formattedDate);
  int splitT = formattedDate.indexOf("T");
  std::string hour = "";
  hour.push_back(formattedDate[splitT+1]);
  hour.push_back(formattedDate[splitT+2]);
  std::stringstream ss(hour);
  int hourInt = 0;
  ss>>hourInt;
  Serial.println("Ora:");
  Serial.println(hourInt);
  if ((light<minLight)&&(7<=hourInt&&hourInt<=22)){ lightOn(index); std::string str = "";str.push_back('L'); str.push_back(index+'0'); client.publish(str.c_str(), "1", true);}
  else if(light>=minLight){ if(checkLight==5){lightOff(index); std::string str = "";str.push_back('L'); str.push_back(index+'0');
  client.publish(str.c_str(), "0", true);checkLight = 0; client.publish("MLight","0", true);} else{ checkLight++; Serial.println("Check Light: "); Serial.println(checkLight); 
  std::string s = "";s.push_back(checkLight+'0');  client.publish("MLight", s.c_str(), true);}}//lightOff(index); std::string str = "";str.push_back('L'); str.push_back(index+'0'); client.publish(str.c_str(), "0", true);}
  //}
  if(!(7<=hourInt&&hourInt<=22)){lightOff(index); std::string str = "";str.push_back('L'); str.push_back(index+'0');
  client.publish(str.c_str(), "0", true);}
  if (conductivity<minConductivity){ fertilizerOn(index); std::string str = "";str.push_back('F'); str.push_back(index+'0'); client.publish(str.c_str(), "1", true);}
  else if(conductivity>=minConductivity){ fertilizerOff(index); std::string str = "";str.push_back('F'); str.push_back(index+'0'); client.publish(str.c_str(), "0", true);}
  if (temperature<minTemp){ heaterOn(); std::string str = "";str.push_back('H'); str.push_back(index+'0'); client.publish(str.c_str(), "1", true);}
  else if (temperature>=minTemp){ heaterOff();std::string str = "";str.push_back('H'); str.push_back(index+'0'); client.publish(str.c_str(), "0", true);}

  Serial.print("Temperatura: ");
  Serial.println(temperature);
  Serial.print("Umiditatea: ");
  Serial.println(moisture);
  Serial.print("Conductivitatea: ");
  Serial.println(conductivity);
  Serial.print("Luminozitate: ");
  Serial.println(light);
  
  delay(15000);
  ESP.restart();
  
}
bool getAdjParams(){ return adjParams;}
NTPClient getNTPClient(){ return timeClient;};
