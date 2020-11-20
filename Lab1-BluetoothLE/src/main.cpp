#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// initialize the global variables
BLEDevice p_device;
BLEServer *p_server;
BLEService *p_service;
BLECharacteristic *p_temp_characteristic, *p_press_characteristic, *p_alc_characteristic;
float temp, pres, alco;

/***********************************************************************************
* Purpose: Simulates sampling a temperature sensor attached to the Whiskey Bug.
* Arguments: None
* Returns: A float representing the temperature of the barrel in degrees Celsius.
**********************************************************************************/
float getTemp() {
  // 15C to 20C is the ideal whiskey barrel temp :)
  int rn = random(1500, 2000);
  return (float)(rn / 100.0f);
}

/***********************************************************************************
* Purpose: Simulates sampling a pressure sensor attached to the Whiskey Bug.
* Arguments: None
* Returns: A float representing the pressure of the barrel in kilo-pascals.
**********************************************************************************/
float getPressure() {
  // 94200 Pa = 94.2 kPa @ 2000ft altitude
  // 101325 Pa = 101.3 kPa @ sea level
  int rn = random(94200, 101325);
  return (float)(rn / 1000.0f);
}

/***********************************************************************************
* Purpose: Simulates sampling an alcohol sensor attached to the Whiskey Bug.
* Arguments: None
* Returns: A float representing the alcohol content of the whiskey in the barrel.
**********************************************************************************/
float getAlcoholContent() {
  // linear regression of alcohol content
  static float abv = 62.5;
  abv = abv - 0.01;
  return abv;
}

/***********************************************************************************
* Purpose: Updates a Bluetooth characteristic with a new value and sends a
* notification to connected clients.
* Arguments:  bleCharacteristic – The BLE characteristic to update.
*             value – The new value of the BLE characteristic.
* Returns: None
**********************************************************************************/
void bleNotify(BLECharacteristic* ble_characteristic, float value) {
  // encode the float value as a string
  // (for easier reading in the BLE Scanner smartphone app).
  char str_value[16];
  snprintf(str_value, 16, "%.3f", value);

  // set the value in the characteristic
  ble_characteristic->setValue(str_value);

  // send a notification to connected clients
  ble_characteristic->notify();
}

void setup() {
  // seed the RNG
  randomSeed(analogRead(0));

  // initialize the serial port
  Serial.begin(115200,  SERIAL_8N1);

  // initialize the BLE (Bluetooth Low Energy) device
  p_device.init("Whiskey Bug"); 

  // create the BLE server
  p_server = p_device.createServer();

  // create BLE service
  p_service = p_server->createService("26586ab7-3a39-401b-b26e-b521a84e5a2c"); 

  // create BLE characteristics for each service
  p_temp_characteristic = p_service->createCharacteristic("e5a8ec67-3b68-4faf-a615-0d5628a39c3f", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  p_press_characteristic = p_service->createCharacteristic("550b2179-13b3-4f70-a841-2c4765038de6", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  p_alc_characteristic = p_service->createCharacteristic("c953dc94-c6ff-4336-82ed-873c2c9fc3e4", BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  // set the initial values for the BLE characteristics
  temp = 17;
  pres = 100;
  alco = 62.5;  

  // set the BLE characteristics values
  p_temp_characteristic->setValue(temp);
  p_press_characteristic->setValue(pres);
  p_alc_characteristic->setValue(alco);

  // start the server service and advertising BLE device
  p_service->start();
  p_server->getAdvertising()->start();
}

void loop() {
  // Pause one second
  delay(1000);

  // Read the sensors from the whiskey bug
  temp = getTemp();
  pres = getPressure();
  alco = getAlcoholContent(); 

  // Output the current values to the serial port (for debugging)
  Serial.print("temp = "); Serial.print(temp); Serial.println(" deg C");
  Serial.print("pres = "); Serial.print(pres); Serial.println(" kPa");
  Serial.print(" alc = "); Serial.print(alco); Serial.println(" % abv"); 

  // Update the Bluetooth characteristics and notify
  bleNotify(p_temp_characteristic, temp);
  bleNotify(p_press_characteristic, pres);
  bleNotify(p_alc_characteristic, alco);   
}
