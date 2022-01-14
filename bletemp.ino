#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);

bool _BLEClientConnected = false;
//https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.temperature.xml
//#define CHAR_TEMP 0x2A6E
//https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.humidity.xml
//#define CHAR_HUMIDITY 0x2A6F
//https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.analog.xml
//#define CHAR_ANALOG 0x2A58  //could be used for CO2
//https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.pollen_concentration.xml&u=org.bluetooth.characteristic.pollen_concentration.xml
//#define CHAR_POLLEN_CONCENTRATION 0x2A75  //could be mis-used for CO2
//https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.environmental_sensing.xml
//#define ENV_SERVICE 0x181A
//https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.characteristic_user_description.xml
//#define USER_DESCRIPTION 0x2901

BLEDescriptor outdoorTemperatureDescriptor(BLEUUID((uint16_t)0x2901));

BLECharacteristic outTemperatureCharacteristic(BLEUUID((uint16_t)0x2A6E), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);


class MyServerCallbacks : public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
		_BLEClientConnected = true;
	};

	void onDisconnect(BLEServer* pServer) {
		_BLEClientConnected = false;
	}
};

void InitBLE() {
	BLEDevice::init("Temperature Sensor");

	// Create the BLE Server
	BLEServer *pServer = BLEDevice::createServer();
	pServer->setCallbacks(new MyServerCallbacks());

	// Create the BLE Service
	BLEService *pService = pServer->createService(BLEUUID((uint16_t)0x181A));

	outdoorTemperatureDescriptor.setValue("Temperature");

	outTemperatureCharacteristic.addDescriptor(&outdoorTemperatureDescriptor);

	// https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
	// Create a BLE Descriptor
	outTemperatureCharacteristic.addDescriptor(new BLE2902());

	pService->addCharacteristic(&outTemperatureCharacteristic);
	pService->start();

	// Start advertising
	pServer->getAdvertising()->start();
}

void setup() {
	Serial.begin(115200);
  sensors.begin();
	delay(1000);
	InitBLE();
  Serial.println("BLE Init Finished");
}


void loop() {
  sensors.requestTemperatures(); 

		if(_BLEClientConnected) {
      float temp = sensors.getTempCByIndex(0);
      Serial.println(temp);

      int tempi = (int) (temp * 100);
      
      outTemperatureCharacteristic.setValue(tempi);
            
      outTemperatureCharacteristic.notify();
      vTaskDelay(1);
    }
}
