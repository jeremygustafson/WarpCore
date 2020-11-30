/*
 * WarpCoreVisual.cpp
 *
 *  Created on: 2 Mar 2018
 *      Author: dancl
 */

#include "WarpCoreVisual.h"
#include <math.h>
#include "esp_log.h"

const char *CORE_TAG = "Core";

WarpCoreVisual::WarpCoreVisual(uint8_t maxBrightness, uint16_t maxCombinedBrightness)
	: maxBrightness(maxBrightness), maxCombinedBrightness(maxCombinedBrightness)
{
}

WarpCoreVisual::~WarpCoreVisual()
{
	if (rows != NULL)
	{
		free(rows);
		rows = NULL;
	}
}

void WarpCoreVisual::init(coreSection_t *_sections, uint8_t _numSections)
{
	// Internal state
	numSections = _numSections;
	sections = _sections;
	numRows = 0;

	// Configure GPIO pins for data output
	gpio_config_t io_conf = {};
	io_conf.mode = GPIO_MODE_OUTPUT;

	for (uint8_t i = 0; i < _numSections; ++i)
		io_conf.pin_bit_mask |= (1ULL << _sections[i].pin);
	gpio_config(&io_conf);

	// Init RGB lib
	strands = (strand_t*)malloc(sizeof(strand_t) * _numSections);
	memset(strands, 0, sizeof(strand_t) * _numSections);

	for (uint8_t i = 0; i < _numSections; ++i)
	{
		strands[i].rmtChannel = i;
		strands[i].gpioNum = _sections[i].pin;
		strands[i].ledType = LED_WS2812B_V3;
		strands[i].brightLimit = 255;
		strands[i].numPixels = _sections[i].numLeds;

		numRows += _sections[i].numRows;
		if (_sections[i].leds == NULL)
			_sections[i].leds = (led_t*)malloc(sizeof(led_t) * _sections[i].numLeds);
	}

	// Rows for easy direct access
	setupRows();

	if (digitalLeds_initStrands(strands, _numSections))
	{
		ESP_LOGE(CORE_TAG, "Could not init LED strips");
	}
	else
	{
		ESP_LOGI(CORE_TAG, "LEDs initialised");
	}
}

void WarpCoreVisual::display()
{
	uint16_t maxColourVal = 0;

	// Update lib with our internal LED representation
	for (uint8_t sectionIndex = 0; sectionIndex < numSections; ++sectionIndex)
	{
		coreSection_t section = sections[sectionIndex];
		strand_t strand = strands[sectionIndex];

		for (uint8_t ledIndex = 0; ledIndex < section.numLeds; ++ledIndex)
		{
			strand.pixels[ledIndex].r = (section.leds[ledIndex].r / 255.0f) * maxBrightness;
			strand.pixels[ledIndex].g = (section.leds[ledIndex].g / 255.0f) * maxBrightness;
			strand.pixels[ledIndex].b = (section.leds[ledIndex].b / 255.0f) * maxBrightness;

			uint16_t maxVal = strand.pixels[ledIndex].r +
						 	  strand.pixels[ledIndex].g +
							  strand.pixels[ledIndex].b;

			if (maxVal > maxColourVal)
				maxColourVal = maxVal;
		}
	}

	// Check if max brightness is too high, if not just display
	if (maxColourVal <= maxCombinedBrightness)
	{
		sendPixelData();
		return;
	}

	// Too bright, reduce brightness
	float multi = (float)maxCombinedBrightness / (float)maxColourVal;
	for (uint8_t sectionIndex = 0; sectionIndex < numSections; ++sectionIndex)
	{
		uint8_t numLeds = sections[sectionIndex].numLeds;
		strand_t strand = strands[sectionIndex];

		for (uint8_t ledIndex = 0; ledIndex < numLeds; ++ledIndex)
		{
			strand.pixels[ledIndex].r = floorf(strand.pixels[ledIndex].r * multi);
			strand.pixels[ledIndex].g = floorf(strand.pixels[ledIndex].g * multi);
			strand.pixels[ledIndex].b = floorf(strand.pixels[ledIndex].b * multi);
		}
	}

	// All done with colour manipulation
	sendPixelData();
}

void WarpCoreVisual::sendPixelData()
{
	for (uint8_t sectionIndex = 0; sectionIndex < numSections; ++sectionIndex)
		digitalLeds_updatePixels(&strands[sectionIndex]);
}

led_t WarpCoreVisual::getLed(uint8_t row, uint8_t col)
{
	if (rows[row].winding == STANDARD)
		return rows[row].leds[col];
	else
		return rows[row].leds[rows[row].numLeds - col];
}

uint8_t WarpCoreVisual::getNumCols()
{
	uint8_t numCols = 0;

	for (uint8_t row = 0; row < numRows; ++row)
	{
		if (rows[row].section->rowType == CENTRAL_CHAMBER)
			continue;
		if (rows[row].numLeds > numCols)
			numCols = rows[row].numLeds;
	}

	return numCols;
}

void WarpCoreVisual::clear()
{
	for (uint8_t rowIndex = 0; rowIndex < numRows; ++rowIndex)
		memset(rows[rowIndex].leds, 0, sizeof(led_t) * rows[rowIndex].numLeds);
}

void WarpCoreVisual::clear(uint8_t r, uint8_t g, uint8_t b)
{
	led_t setting;
	setting.r = r;
	setting.g = g;
	setting.b = b;

	for (uint8_t rowIndex = 0; rowIndex < numRows; ++rowIndex)
	{
		for (uint8_t colIndex = 0; colIndex < rows[rowIndex].numLeds; ++colIndex)
			memcpy(&rows[rowIndex].leds[colIndex], &setting, sizeof(setting));
	}
}

void WarpCoreVisual::setLed(uint8_t row, uint8_t col, uint8_t r, uint8_t g, uint8_t b, bool autoHandleChambeAlignment)
{
	if (row >= numRows)
		return;
	if (col >= rows[row].numLeds)
		return;

	if (rows[row].winding == REVERSE)
		col = rows[row].numLeds - col - 1;

	// Special-case for central chamber, it has more LEDs so we have to figure out how many LEDs are per 'real' column
	// Then light up that many LEDs at the correct position
	if (row == centralRowIndex && autoHandleChambeAlignment)
	{
		float multi = rows[row].numLeds / getNumCols();
		uint8_t multiInt = ceilf(multi);
		uint8_t actualCol = roundf(col * multi);
		for (uint8_t i = 0; i < multiInt; ++i)
		{
			uint8_t ledCol = actualCol + i;
			rows[row].leds[ledCol].r = r;
			rows[row].leds[ledCol].g = g;
			rows[row].leds[ledCol].b = b;
		}
		return;
	}

	rows[row].leds[col].r = r;
	rows[row].leds[col].g = g;
	rows[row].leds[col].b = b;
}

void WarpCoreVisual::setRow(uint8_t row, uint8_t r, uint8_t g, uint8_t b)
{
	if (row >= numRows)
		return;

	led_t setting;
	setting.r = r;
	setting.g = g;
	setting.b = b;

	coreRow_t ledRow = rows[row];
	uint8_t numLeds = ledRow.numLeds;
	for (uint8_t i = 0; i < numLeds; ++i)
		memcpy(&ledRow.leds[i], &setting, sizeof(setting));
}

void WarpCoreVisual::setCol(uint8_t col, uint8_t r, uint8_t g, uint8_t b, bool includeChamber)
{
	for (uint8_t rowIndex = 0; rowIndex < numRows; ++rowIndex)
	{
		if (rowIndex == centralRowIndex && !includeChamber)
			continue;

		setLed(rowIndex, col, r, g, b);
	}
}

void WarpCoreVisual::setupRows()
{
	if (rows != NULL)
		free(rows);
	rows = (coreRow_t*)malloc(sizeof(coreRow_t) * numRows);

	uint8_t rowIndex = 0;
	for (uint8_t sIndex = 0; sIndex < numSections; ++sIndex)
	{
		coreSection_t section = sections[sIndex];
		uint8_t ledsPerRow = section.numLeds / section.numRows;

		for (uint8_t sectionRowIndex = 0; sectionRowIndex < section.numRows; ++sectionRowIndex)
		{
			rows[rowIndex].section = &sections[sIndex];
			rows[rowIndex].numLeds = ledsPerRow;
			rows[rowIndex].winding = section.winding;
			if (section.winding == STANDARD)
				rows[rowIndex].leds = &section.leds[sectionRowIndex * ledsPerRow];
			else
				rows[rowIndex].leds = &section.leds[(section.numRows - sectionRowIndex - 1) * ledsPerRow];

			if (section.rowType == CENTRAL_CHAMBER)
				centralRowIndex = rowIndex;

			++rowIndex;
		}
	}
}
