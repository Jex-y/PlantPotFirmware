#pragma once
void setup();
void loop();
void tasksInit();
void task0_1(void* params);
void task0_2(void* params);
void task1_1(void* params);
void task1_2(void* params);
void pixelsInit();
void tryWifiInit();
void bleInit();
void pinsInit();
void updateMoisture();
void updatePixels();
void updateNetworkTime();
unsigned long getTime();
void updateBLE();
char* floatToString(float value);
char* boolToString(bool value);
class ServerCallback : public BLEServerCallbacks {
	void onConnect(BLEServer* pServer);
	void onDisconect(BLEServer* pServer);
};
class CharacteristicCallback : public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic* pCharactersitic);
};