/*
 * WarpCoreVisual.h
 *
 *  Created on: 2 Mar 2018
 *      Author: dancl
 */

#ifndef MAIN_WARPCOREVISUAL_H_
#define MAIN_WARPCOREVISUAL_H_

#include <string.h>
#include "driver/gpio.h"
#include "esp32_digital_led_lib.h"
#include "Config.h"

enum WindingDirection
{
	STANDARD = 0,
	REVERSE = 1
};

enum RowType
{
	NORMAL = 0,
	CENTRAL_CHAMBER = 1
};

typedef struct led
{
	uint8_t r;
	uint8_t g;
	uint8_t b;

	led() : r(0), g(0), b(0) {}
	led (uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
} led_t;

typedef struct coreSection
{
	// GPIO pin the data connection for the LEDs in this section is hooked up to
	gpio_num_t pin;

	// Type of row, such as standard or central chamber
	RowType rowType;

	// Winding direction, such as clockwise / anticlockwise
	WindingDirection winding;

	// Total number of LEDs in this section
	uint8_t numLeds;

	// Number of rows in this section
	uint8_t numRows;

	// Colours for each LED in the section
	led_t *leds;
} coreSection_t;

typedef struct coreRow
{
	const coreSection *section;
	WindingDirection winding;
	uint8_t numLeds;
	led_t *leds;
} coreRow_t;

class WarpCoreVisual
{
public:
	WarpCoreVisual(uint8_t maxBrightness, uint16_t maxCombinedBrightness);
	virtual ~WarpCoreVisual();

	uint8_t getNumRows() { return numRows; }
	uint8_t getNumCols();
	uint8_t getCentralRowIndex() { return centralRowIndex; }

	void init(coreSection_t *sections, uint8_t numSections);
	void display();
	led_t getLed(uint8_t row, uint8_t col);
	void clear();
	void clear(uint8_t r, uint8_t g, uint8_t b);
	void setLed(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b, bool autoHandleChambeAlignment = true);
	void setRow(uint8_t row, uint8_t r, uint8_t g, uint8_t b);
	void setCol(uint8_t col, uint8_t r, uint8_t g, uint8_t b, bool includeChamber);

	void setupRows();

private:
	coreSection_t *sections = NULL;
	strand_t *strands = NULL;
	coreRow_t *rows = NULL;
	uint8_t centralRowIndex = 0;
	uint8_t numSections = 0;
	uint8_t numRows = 0;

	float maxBrightness;
	uint16_t maxCombinedBrightness;

	void sendPixelData();
};

#endif /* MAIN_WARPCOREVISUAL_H_ */
