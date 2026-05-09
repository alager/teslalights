/*
  LED

  This example creates a Bluetooth® Low Energy peripheral with service that contains a
  characteristic to control an LED.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic Bluetooth® Low Energy central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Adafruit_NeoPixel.h>

#include "main.h"
#include "BLE.h"


// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN    D0

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 42
#define NUM_PIXELS	(LED_COUNT)

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

unsigned long pixelPrevious = 0;        // Previous Pixel Millis
unsigned long patternPrevious = 0;      // Previous Pattern Millis
int           patternCurrent = 0;       // Current Pattern Number
int           patternInterval = 5000;   // Pattern Interval (ms)
bool          patternComplete = false;

int           pixelInterval = 50;       // Pixel Interval (ms)
int           pixelQueue = 0;           // Pattern Pixel Queue
int           pixelCycle = 0;           // Pattern Pixel Cycle
uint16_t      pixelNumber = LED_COUNT;  // Total Number of Pixels

uint16_t	stripMode = 0;
uint16_t 	ble_loopCounter = 0;

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
//BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLEByteCharacteristic switchCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

// const int ledPin = LED_BUILTIN; // pin to use for the LED
const int sleepTime = 10000; // Sleep time in milli-seconds for BLE sleep mode
byte mode = 0;

void setup() {
	Serial.begin(115200);
	// while (!Serial);

	Serial.println("Setup START 1");

	strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
	clearStrip(); 			// Initialize all pixels to 'off'
	strip.show();            // Turn OFF all pixels ASAP
	strip.setBrightness(200); // Set BRIGHTNESS to about 1/5 (max = 255)

	// begin initialization
	if (!BLE.begin()) {
		Serial.println("starting Bluetooth® Low Energy module failed!");
		while (1);
  	}

  // set advertised local name and service UUID:
  BLE.setLocalName("Tesla Lights");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characteristic:
  switchCharacteristic.writeValue(0);

  // assign event handlers for device connection and disconnection:
  BLE.setEventHandler(BLEConnected, connectHandler);
  BLE.setEventHandler(BLEDisconnected, disconnectHandler);

  // assign event handler for characteristic written:
  switchCharacteristic.setEventHandler(BLEWritten, characteristicUpdated);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() 
{
	BLE.poll();
	switchCharacteristic.readValue( mode );
	lightLoop( mode );

	// delay(10);
}


void lightLoop( u_int8_t stripMode )
{
	// Serial.println("loop mode: " + String(stripMode) );
	// LED strip service loop
	if (stripMode == 0)
	{
		clearStrip();
		if (ble_loopCounter++ > 50)
			ble_loopCounter = 0; // reset loop counter to prevent overflow
	}
	else if (stripMode == 1)
	{
		// knight rider
		#define KNIGHT_RIDER_CYCLES (1)
		#define KNIGHT_RIDER_SPEED (32)
		#define KNIGHT_RIDER_WIDTH (7)
		#define KNIGHT_RIDER_COLOR (0xFF1000) // original orange-red

		// knightRider(KNIGHT_RIDER_CYCLES, KNIGHT_RIDER_SPEED, KNIGHT_RIDER_WIDTH, KNIGHT_RIDER_COLOR); // Cycles, Speed, Width, RGB Color (original orange-red)
		knightRider( KNIGHT_RIDER_SPEED, KNIGHT_RIDER_WIDTH, KNIGHT_RIDER_COLOR); // Cycles, Speed, Width, RGB Color (original orange-red)
		clearStrip();																				  // Clear strip after pattern completes
	}
	else if (stripMode == 2)
	{
		// red hazard flash, 2 fast then 2 slow flashes
		flash(strip.Color(255, 0, 0), 100); // flash red 100ms
		flash(strip.Color(255, 0, 0), 100);
		delay(100);
		flash(strip.Color(255, 0, 0), 200); // flash red every 200ms
		flash(strip.Color(255, 0, 0), 200);
		delay(100);
	}
	else if (stripMode == 3)
	{
		// popo mode -- yeah you know it
		popoFlash(strip.Color(255, 0, 0), strip.Color(0, 0, 0), 200); // red first
		popoFlash(strip.Color(0, 0, 0), strip.Color(0, 0, 255), 200); // blue second
		clearStrip();
	}
	else if (stripMode == 4)
	{
		// alternate yellow and white flashes
		popoFlash(strip.Color(255, 255, 0), strip.Color(0, 0, 0), 200);
		popoFlash(strip.Color(0, 0, 0), strip.Color(255, 255, 255), 200);
		clearStrip();

		popoFlash(strip.Color(255, 255, 255), strip.Color(0, 0, 0), 200);
		popoFlash(strip.Color(0, 0, 0), strip.Color(255, 255, 0), 200);
		clearStrip();
	}
	else if (stripMode == 5)
	{
		#define BREATH_MIN_BRIGHTNESS (20)
		#define BREATH_MAX_BRIGHTNESS (255)
		#define BREATHING_DELAY (5)

		// red breathing effect
		for (uint8_t brightness = BREATH_MIN_BRIGHTNESS; brightness < BREATH_MAX_BRIGHTNESS; brightness++)
		{
			strip.fill(strip.Color(strip.gamma8(brightness), 0, 0)); // Red color with varying brightness
			strip.show();
			delay(BREATHING_DELAY);
		}
		for (uint8_t brightness = BREATH_MAX_BRIGHTNESS; brightness > BREATH_MIN_BRIGHTNESS; brightness--)
		{
			strip.fill(strip.Color(strip.gamma8(brightness), 0, 0)); // Red color with varying brightness
			strip.show();
			delay(BREATHING_DELAY);
		}
	}

	// delay(100); // 100ms delay to allow BLE processing and prevent overwhelming the stack
}

// flash the strip
void flash(uint32_t color, int wait)
{
	strip.fill(color);
	strip.show();
	delay(wait);
	strip.clear();
	strip.show();
	delay(wait);
}

void popoFlash(uint32_t color1, uint32_t color2, int wait)
{
	strip.fill( color1, 0, NUM_PIXELS/2);
	strip.fill( color2, NUM_PIXELS/2, NUM_PIXELS/2);
	strip.show();
	delay(wait);
}


// Cycles - one cycle is scanning through all pixels left then right (or right then left)
// Speed - how fast one cycle is (32 with 16 pixels is default KnightRider speed)
// Width - how wide the trail effect is on the fading out LEDs.  The original display used
//         light bulbs, so they have a persistance when turning off.  This creates a trail.
//         Effective range is 2 - 8, 4 is default for 16 pixels.  Play with this.
// Color - 32-bit packed RGB color value.  All pixels will be this color.
// knightRider(cycles, speed, width, color);
// void knightRider(uint16_t cycles, uint16_t speed, uint8_t width, uint32_t color)
void knightRider( uint16_t speed, uint8_t width, uint32_t color)
{
	uint32_t old_val[NUM_PIXELS];		// up to 256 lights!
	unsigned long previousMillis = 0;	// Stores last time the action was performed

	// Larson time baby!
	// for(uint16_t i = 0; i < cycles; i++)
	// {
		// Serial.println("cycles: " + String(cycles));
		for (uint16_t count = 0; count < NUM_PIXELS-1; ) 
		{
			unsigned long currentMillis = millis(); // Get the current time

			// allow BLE processing while waiting for the next pixel update
			BLE.poll();

			if (currentMillis - previousMillis >= speed)
			{
				// Save the last time you performed the action
				previousMillis = currentMillis;

				// Serial.println("count up: " + String(count));
				// increment the loop counter when we do a loop
				count++;

				strip.setPixelColor(count, color);
				old_val[count] = color;
				for (int16_t x = count; x > 0; x--)
				{
					old_val[x - 1] = dimColor(old_val[x - 1], width);
					strip.setPixelColor(x - 1, old_val[x - 1]);
				}
				strip.show();
			}
		}

		for (int count = NUM_PIXELS-1; count > 0; ) 
		{
			unsigned long currentMillis = millis(); // Get the current time

			// allow BLE processing while waiting for the next pixel update
			BLE.poll();

			if (currentMillis - previousMillis >= speed)
			{

				// Serial.println("count down: " + String(count));
				// decrement the loop counter when we do a loop
				count--;

				strip.setPixelColor(count, color);
				old_val[count] = color;

				for (uint16_t x = count; x <= NUM_PIXELS; x++)
				{
					old_val[x - 1] = dimColor(old_val[x - 1], width);
					strip.setPixelColor(x + 1, old_val[x + 1]);
				}
				strip.show();
			}
		}
	// }
}


void clearStrip() 
{
	for( int i = 0; i<NUM_PIXELS; i++){
		strip.setPixelColor(i, 0x000000); strip.show();
	}
}

// Dims a color by a given width, which is used to create the trailing effect in the knightRider pattern.
uint32_t dimColor(uint32_t color, uint8_t width) 
{
	uint8_t inv = 10 - width;  // maps 2→8, 3→7, ..., 8→2
	return (((color&0xFF0000)/inv)&0xFF0000) + (((color&0x00FF00)/inv)&0x00FF00) + (((color&0x0000FF)/inv)&0x0000FF);
}