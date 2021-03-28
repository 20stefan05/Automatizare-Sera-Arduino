#include "Mqttcon.h"

int lostConNo = 0;
RTC_DATA_ATTR int bootCount = 0;
TaskHandle_t hibernateTaskHandle = NULL;

char* FLORA_DEVICES[] = {
    "C4:7C:8D:67:6A:23"//, 
  //  "C4:7C:8D:67:22:22", 
  //  "C4:7C:8D:67:33:33"
};
BLEClient* floraClient;
//BLEClient* floraClient1;
//BLEClient* floraClients[10];
std::vector<BLEClient*> floraClients;
BLERemoteService* floraService[10];
BLERemoteService* floraService1;
static int deviceCount =sizeof FLORA_DEVICES / sizeof FLORA_DEVICES[0];
bool readBattery = ((bootCount % BATTERY_INTERVAL) == 0);
bool runnedonce = false;
void setup() {
  // all action is done when device is woken up
 // if(!runnedonce){
 if(runnedonce==false){ pinMode(19, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(18, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(34, OUTPUT); }
  Serial.begin(115200);
  delay(1000);
  Serial.print("inceput ");
  Serial.println(runnedonce);
  
  // increase boot count
  bootCount++;

  // create a hibernate task in case something gets stuck
  //xTaskCreate(delayedHibernate, "hibernate", 4096, NULL, 1, &hibernateTaskHandle);

  Serial.println("Initialize BLE client...");
  
  BLEDevice::init("");
  BLEDevice::setPower(ESP_PWR_LVL_P7);
  if(!runnedonce){
  // connecting wifi and mqtt server
  connectWifi();
  connectMqtt(); runnedonce = true;}
  getNTPClient().begin();
  // check if battery status should be read - based on boot count
  bool readBattery = ((bootCount % BATTERY_INTERVAL) == 0);
  
  // process devices
  for (int i=0; i<deviceCount; i++) {
    int tryCount = 0;
    char* deviceMacAddress = FLORA_DEVICES[i];
    BLEAddress floraAddress(deviceMacAddress);
    floraClient = getFloraClient(floraAddress);//adaugat de mine
    floraService[i] = getFloraService(floraClient);//adaugat de mine
  
    while (tryCount < RETRY) {
      tryCount++;
      if (processFloraDevice(floraAddress, deviceMacAddress, true, tryCount)) {
        break;
      }
      delay(1000);
    }
    delay(1500);
  }
//  for(int i = 0; i<deviceCount; i++){
//    char* deviceMacAddress = FLORA_DEVICES[i];
//    BLEAddress floraAddress(deviceMacAddress);
//    floraClient = getFloraClient(floraAddress);
//    floraService[i] = getFloraService(floraClient);
//  }
  // disconnect wifi and mqtt
  //disconnectWifi();
  //disconnectMqtt();

  // delete emergency hibernate task
  //vTaskDelete(hibernateTaskHandle);
//    char* deviceMacAddress1 = FLORA_DEVICES[0];
//    BLEAddress floraAddress1(deviceMacAddress1);
    //BLEClient* floraClient1 = getFloraClient(floraAddress1);//adaugat de mine
    // floraService1 = getFloraService(floraClient1);//adaugat de mine
  // go to sleep now
  //hibernate();
}


void loop() {
  
  getClient().loop();
  if(!getClient().connected()){
    lostConNo++;
    if(lostConNo==3)ESP.restart();//{ connectWifi(); connectMqtt();}
  }
  
  
  //Serial.println("1");
// if(!getClient().connected()) Serial.println("deconectat de la mqtt");
// for(int i = 0; i<deviceCount; i++){
//    
//    //char* deviceMacAddress = FLORA_DEVICES[i];
//    //BLEAddress floraAddress(deviceMacAddress);
//    //floraClient1 = getFloraClient(floraAddress); //problema in linia asta
//    //floraService1 = getFloraService(floraClient);//trebuie verificata
    
    char* deviceMacAddress1 = FLORA_DEVICES[0];
    BLEAddress floraAddress1(deviceMacAddress1);
    if(getAdjParams()) adjustSpecificParameters(0, floraAddress1);
    
//    Serial.println("Temperatura: ");
//    Serial.println(readFloraTemperature(floraService1));
//    Serial.println("test");

//  }
//Serial.println("test");
 //delay(1000);

}
