#pragma once
void setup();
void loop();
void pixelsInit();
//void tryWifiInit();
void bleInit();
void pinsInit();
void updateMoisture();
void updatePixels();
//void updateNetworkTime();
//unsigned long getTime();
void floatToString(char* buffer, float value);
void boolToString(char* buffer, bool value);
class ServerCallback : public BLEServerCallbacks {
	void onConnect(BLEServer* pServer);
	void onDisconect(BLEServer* pServer);
};
class CharacteristicCallback : public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic* pCharactersitic);
};