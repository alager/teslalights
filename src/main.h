#include <ArduinoBLE.h>

void lightLoop(u_int8_t stripMode);
void flash(uint32_t color, uint16_t wait);
void popoFlash(uint32_t color1, uint32_t color2, uint16_t wait);
void knightRider(uint16_t speed, uint8_t width, uint32_t color);
void clearStrip(void);
uint32_t dimColor( uint32_t color, uint8_t width );