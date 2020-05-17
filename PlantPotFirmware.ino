#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
//#include <WiFi.h>
//#include <NTPClient.h>
//#include <WiFIUdp.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include "UUIDs.h"
#include "pins.h"
#include "PlantPotFirmware.h"

/*
 Name:		PlantPotv2.ino
 Created:	5/14/2020 11:01:40 PM
 Author:	markj
*/

Adafruit_NeoPixel strips[2];

BLEServer* bleServer;
bool bleconnected = false;

float moisture = 0;
BLEService* MoistureService;
BLECharacteristic* MoistureCharacteristic;

float threshold = 0.5;
BLEService* ThresholdService;
BLECharacteristic* ThresholdCharacteristic;

unsigned char colour[3] = {255, 0, 128};
BLEService* ColourService;
BLECharacteristic* RedCharacteristic;
BLECharacteristic* GreenCharacteristic;
BLECharacteristic* BlueCharacteristic;

bool lightsOn = false;
BLEService* LightsOnService;
BLECharacteristic* LightsOnCharacteristic;

/*
char ssid[20] = "VM8280875";
char pass[20] = "s3bkCttHnpbr";
byte ip[4];
bool* wificonnected = false;
BLEService* WiFiCredentailsService;
BLECharacteristic* SSIDCharacteristic;
BLECharacteristic* PASSCharacteristic;
BLECharacteristic* IPCharacteristic;
BLECharacteristic* ConnectedCharactersitic;

WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP);
unsigned long* lastTimeGet;
unsigned int* networktime;
*/

void setup()
{
	Serial.begin(115200);
	pinsInit();         // Initialise the GPIO pins for the pixels, moisture power and data and status LED.
	pixelsInit();       // Initialise the pixels.
	//tryWifiInit();      // Initialise the WiFi and NTP clients, may fail if WiFi not available.
	bleInit();          // Initialise the bluetooth low energy server.
	updateMoisture();   // Update the moisture for the first time.
	moisture *= 2;
	updatePixels();     // Update the pixels for the first time.
}

void loop()
{
	updateMoisture();
	delay(10000);
}

void pixelsInit()
{
	strips[0] = Adafruit_NeoPixel(NUM_LEDS, L1, NEO_GRB + NEO_KHZ800);
	strips[1] = Adafruit_NeoPixel(NUM_LEDS, L2, NEO_GRB + NEO_KHZ800);
	for (byte i = 0; i < 2; i++) {
		strips[i].begin();
	}
	return;
}
/*
void tryWifiInit()
{
	// Starts WiFi and initialises the NTP client
	WiFi.begin(ssid, pass);
	for (unsigned short i; i < 10 * 60 * 2; i++) { // If not connected after 10 minutes, give up.
		if (WiFi.status() == WL_CONNECTED) {
			*wificonnected = true;
			goto connected;
		}
		delay(500);
	}
	return;
connected:
	for (byte i = 0; i < 4; i++) { ip[i] = WiFi.localIP()[i]; } // Load ip into memory address. 

	ntpClient.begin();
}
*/
void bleInit()
{
	BLEDevice::init("SmartPotv2");
	bleServer = BLEDevice::createServer();
	bleServer->setCallbacks(new ServerCallback());
	MoistureService = bleServer->createService(MOISTURE_SERVICE_UUID);
	MoistureCharacteristic = MoistureService->createCharacteristic(
		MOISTURE_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

	ThresholdService = bleServer->createService(THRESHOLD_SERVICE_UUID);
	ThresholdCharacteristic = ThresholdService->createCharacteristic(
		THRESHOLD_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
	ThresholdCharacteristic->setCallbacks(new CharacteristicCallback());

	ColourService = bleServer->createService(COLOUR_SERVICE_UUID);
	RedCharacteristic = ColourService->createCharacteristic(
		RED_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
	RedCharacteristic->setCallbacks(new CharacteristicCallback());
	GreenCharacteristic = ColourService->createCharacteristic(
		GREEN_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
	GreenCharacteristic->setCallbacks(new CharacteristicCallback());
	BlueCharacteristic = ColourService->createCharacteristic(
		BLUE_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
	BlueCharacteristic->setCallbacks(new CharacteristicCallback());

	LightsOnService = bleServer->createService(LIGHTSON_SERVICE_UUID);
	LightsOnCharacteristic = LightsOnService->createCharacteristic(
		LIGHTSON_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
	LightsOnCharacteristic->setCallbacks(new CharacteristicCallback());

	/*
	WiFiCredentailsService = bleServer->createService(WIFICREDENTIALS_SERVICE_UUID);
	SSIDCharacteristic = WiFiCredentailsService->createCharacteristic(
		SSID_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
	SSIDCharacteristic->setCallbacks(new CharacteristicCallback());
	PASSCharacteristic = WiFiCredentailsService->createCharacteristic(
		PASS_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY);
	PASSCharacteristic->setCallbacks(new CharacteristicCallback());
	IPCharacteristic = WiFiCredentailsService->createCharacteristic(
		IP_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
	ConnectedCharactersitic = WiFiCredentailsService->createCharacteristic(
		CONNECTED_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
	*/
	uint8_t value;

	value = (int)(moisture * 100);
	MoistureCharacteristic->setValue(&value, sizeof(value));
	char buffer[8];
	floatToString(buffer, threshold);
	ThresholdCharacteristic->setValue(buffer);
	clearBuffer(buffer,8);

	value = colour[0];
	RedCharacteristic->setValue(&value, sizeof(value));
	value = colour[1];
	RedCharacteristic->setValue(&value, sizeof(value));
	value = colour[2];
	RedCharacteristic->setValue(&value, sizeof(value));
	boolToString(buffer, lightsOn);
	LightsOnCharacteristic->setValue(buffer);
	/*
	SSIDCharacteristic->setValue(ssid);
	char ipstr[17];
	sprintf(ipstr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	IPCharacteristic->setValue(ipstr);
	ConnectedCharactersitic->setValue(boolToString(wificonnected));
	*/
	MoistureService->start();
	ThresholdService->start();
	ColourService->start();
	LightsOnService->start();
	//WiFiCredentailsService->start();

	BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(MOISTURE_SERVICE_UUID);
	pAdvertising->addServiceUUID(THRESHOLD_SERVICE_UUID);
	pAdvertising->addServiceUUID(COLOUR_SERVICE_UUID);
	pAdvertising->addServiceUUID(LIGHTSON_SERVICE_UUID);
	//pAdvertising->addServiceUUID(WIFICREDENTIALS_SERVICE_UUID);
	pAdvertising->setScanResponse(true);
	pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
	pAdvertising->setMinPreferred(0x12);
	BLEDevice::startAdvertising();
	MoistureCharacteristic->notify();
	ThresholdCharacteristic->notify();
	RedCharacteristic->notify();
	GreenCharacteristic->notify();
	BlueCharacteristic->notify();
	LightsOnCharacteristic->notify();
	return;
}

void pinsInit()
{
	pinMode(moisturePower, OUTPUT);
	pinMode(moistureData, INPUT);
	pinMode(LED, OUTPUT);
	pinMode(L1, OUTPUT);
	pinMode(L2, OUTPUT);

	digitalWrite(LED, HIGH);
	digitalWrite(moisturePower, LOW);
}

void updateMoisture()
{
	Serial.write("updateMoisture called\n");
	digitalWrite(moisturePower, HIGH);
	digitalWrite(LED, HIGH);
	Serial.write("Pins set to high\n");
	delay(100);
	moisture = ((((float)analogRead(moistureData)) / 4096) + moisture)/2;
	Serial.printf("Read value %f\n",moisture);
	digitalWrite(moisturePower, LOW);
	digitalWrite(LED, LOW);
	Serial.write("Finished update moisture\n");
	char buffer[8];
	sprintf(buffer, "%1.4f", moisture);
	MoistureCharacteristic->setValue(buffer);
	MoistureCharacteristic->notify();
	return;
}

void updatePixels()
{
	for (byte i = 0; i < 2; i++) {

		if (lightsOn) {
			strips[i].fill(strips[i].Color(colour[0], colour[1], colour[2]));
			if (moisture < threshold)
			{
				unsigned char b = (int)((moisture / threshold) * 255);
				unsigned char g = 255 - b;
				//strips[i].setPixelColor(0, 0, g, b);
				//strips[i].setPixelColor(NUM_LEDS - 1, 0, g, b);
			}
		}
		else {
			strips[i].fill(strips[i].Color(0, 0, 0));
		}
		strips[i].show();

	}
}
/*
void updateNetworkTime()
{
	 // Every 10 minutes
	if (*wificonnected) {
		while (!ntpClient.update()) {
			ntpClient.forceUpdate();
		}
		*networktime = (int)(ntpClient.getHours() * 3600) + (int)(ntpClient.getMinutes() * 60) + (int)ntpClient.getSeconds();
	}
	lastTimeGet = millis();
	return;
}

unsigned long getTime() {
	return networktime + (int)((millis()-lastTimeGet)/1000);
}
*/

void floatToString(char* buffer,float value)
{
	Serial.printf("\t| Float to string called on value %f\n", value);
	sprintf(buffer, "%.4f", value);
	Serial.printf("\t| Returned a string: %s\n",buffer);
	return;
}

void boolToString(char* buffer, bool value)
{
	if (value) { buffer = "true"; }
	else { buffer = "false"; }
}

void clearBuffer(char* buffer, byte length)
{
	for (byte i = 0; i < length; i++) {
		buffer[i] = 0;
	}
}

void ServerCallback::onConnect(BLEServer* pServer)
{
	bleconnected = true;
}

void ServerCallback::onDisconect(BLEServer* pServer)
{
	bleconnected = false;
}

void CharacteristicCallback::onWrite(BLECharacteristic* pCharactersitic) {
	BLEUUID targetUUID = pCharactersitic->getUUID();
	Serial.write("Write callback called\n");
	if (targetUUID.equals(ThresholdCharacteristic->getUUID())) {
		threshold = ((String)(char*)(pCharactersitic->getData())).toFloat();
		updatePixels();
	}
	else if (targetUUID.equals(RedCharacteristic->getUUID())) {
		colour[0] = (unsigned char)(*(char*)pCharactersitic->getData());
		updatePixels();
	}
	else if (targetUUID.equals(GreenCharacteristic->getUUID())) {
		colour[1] = (unsigned char)(*(char*)pCharactersitic->getData());
		updatePixels();
	}
	else if (targetUUID.equals(BlueCharacteristic->getUUID())) {
		colour[2] = (unsigned char)(*(char*)pCharactersitic->getData());
		updatePixels();
	}
	else if (targetUUID.equals(LightsOnCharacteristic->getUUID())) {
		lightsOn = ((String)(char*)pCharactersitic->getData()).equals("true");
		updatePixels();
	}
	else {
		Serial.write("Target UUID didn't match anything\n");
	}
	/*
	else if (targetUUID.equals(SSIDCharacteristic->getUUID())) {
		//((String)(char*)pCharactersitic->getData()).toCharArray(ssid, 20, 0);
	}
	else if (targetUUID.equals(PASSCharacteristic->getUUID())) {
		//((String)(char*)pCharactersitic->getData()).toCharArray(pass, 20, 0);
	}
	*/
}