
#include <ArduinoBLE.h>

// listen for BLE connect events:
void connectHandler(BLEDevice central);
void disconnectHandler(BLEDevice central);
void characteristicUpdated(BLEDevice central, BLECharacteristic thisChar);
