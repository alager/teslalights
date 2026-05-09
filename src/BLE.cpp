
#include "BLE.h"

// listen for BLE connect events:
void connectHandler(BLEDevice central)
{
	// central connected event handler
	Serial.print("Connected event, central: ");
	Serial.println(central.address());
	digitalWrite(LED_BUILTIN, HIGH);
}

// listen for BLE disconnect events:
void disconnectHandler(BLEDevice central)
{
	// central disconnected event handler
	Serial.print("Disconnected event, central: ");
	Serial.println(central.address());
	digitalWrite(LED_BUILTIN, LOW);
}

// listen for characteristic updated events:
void characteristicUpdated(BLEDevice central, BLECharacteristic thisChar)
{
	// central wrote new value to characteristic, update LED
	Serial.print("Characteristic updated. UUID: ");
	Serial.print(thisChar.uuid());
	Serial.print("   value: ");
	byte incoming = 0;
	thisChar.readValue(incoming);
	Serial.println(incoming);
}