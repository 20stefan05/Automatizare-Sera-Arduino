#include "Actuators.h"

int LIGHTS[] = {18, 34, 0};
int VALVES[] = {17, 16, 4};
int FERTILIZER_VALVES[] = {5, 2, 15};
int heater = 19;
void lightOn(int index){
   digitalWrite(LIGHTS[index], HIGH);
   Serial.println("light on");
}
void lightOff(int index){
   digitalWrite(LIGHTS[index], LOW);
}
void waterOn(int index){
   digitalWrite(VALVES[index], HIGH);
   //digitalWrite(2, HIGH);
}
void waterOff(int index){
   digitalWrite(VALVES[index], LOW);
//   digitalWrite(2, LOW);
}
void fertilizerOn(int index){
  digitalWrite(FERTILIZER_VALVES[index], HIGH);
  digitalWrite(2, HIGH);
}
void fertilizerOff(int index){
  digitalWrite(FERTILIZER_VALVES[index], LOW);
  digitalWrite(2, LOW);
}
void heaterOn(){
  digitalWrite(heater, HIGH);
  Serial.println("Heater on");
}
void heaterOff(){
  digitalWrite(heater, LOW);
}
//void adjustSpecificParameters(int index, BLERemoteService* floraService){
//  
//  int moisture = readFloraMoisture(BLERemoteService* floraService);
//  int light = readFloraLight(BLERemoteService* floraService);
//  int minLight = PLANTS[index]->minLightLux, minMoist = PLANTS[index]->minSoilMoist;
//  if(moisture!=10000){
//  if(moisture<minMoist) waterOn(index);
//  else if(moisture>=minMoist) waterOff(index);
//  }
//  if(light!=100000){
//  if (light<minLight) lightOn(index);
//  else if(light>=minLight) lightOff(index);
//  }
//}
