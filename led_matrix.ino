#include "FastLED.h"

#define LED_W 10
#define LED_H 10

#define LED_PIN 6

CRGB leds[LED_W * LED_H];

CRGBPalette16 purplyPalette;
extern const TProgmemPalette16 RGBPalette PROGMEM;

// Pollable by functions
bool beatChanged = false;
uint32_t beatCount = 0;
uint32_t currentBeatStart = 0;
uint8_t prevBeat;
uint8_t beat;

uint8_t prevFunc = 0;
uint8_t currentFunc = 7;

uint8_t (*drawFunc[8])(uint32_t startBeat);

void setup()
{
	delay(3000);
	FastLED.addLeds<WS2812, LED_PIN>(leds, LED_W * LED_H);

	drawFunc[0] = func0;
	drawFunc[1] = func1;
	drawFunc[2] = func2;
	drawFunc[3] = func3;
	drawFunc[4] = func4;
	drawFunc[5] = func5;
	drawFunc[7] = intro_func;
	buildRandomPurplyPalette();
}
void loop()
{
	beat = beat8(128);
	if (beat < prevBeat)
	{
		beatChanged = true;
		beatCount++;
	}
	prevBeat = beat;

	if (currentFunc != prevFunc)
	{
		currentBeatStart = beatCount;
		LEDS.setBrightness(255);
		buildRandomPurplyPalette();
	}
	prevFunc = currentFunc;

	currentFunc = (*drawFunc[currentFunc])(currentBeatStart);

	FastLED.show();
	beatChanged = false;
	delay(10);
}

#define INTRO_BEATS 12
uint8_t intro_func(uint32_t startBeat)
{
	CRGB blu = CRGB::Blue;
	for (uint8_t x = 0; x < LED_W; ++x)
	{
		for (uint8_t y = 0; y < LED_H; ++y)
		{
			CRGB ran = CRGB(random8(20),random8(200, 256),random8(100));
			leds[XY(x, y)] = blu.lerp8(ran, (fract8)((beatCount - startBeat) * 255 / INTRO_BEATS));
		}
	}
	LEDS.setBrightness(beat < 200 ? 200 : beat);
	return beatCount >= startBeat + INTRO_BEATS ? 0 : 7;
}

#define F0_BEATS 12 
uint8_t func0(uint32_t startBeat)
{
	static uint8_t t = 0;
	static uint8_t theta = 0;
	static bool fwd = true;
	if (beatChanged)
		fwd = !fwd;

	uint8_t x0 = (cos8(theta) * 10) / 256;
	uint8_t y0 = (sin8(theta) * 10) / 256;

	uint8_t x1 = (cos8(theta - 128) * 10) / 256;
	uint8_t y1 = (sin8(theta - 128) * 10) / 256;
	setLed(x0, y0, ColorFromPalette(RainbowColors_p, t, beat, LINEARBLEND));
	setLed(x1, y1, ColorFromPalette(RainbowColors_p, t - 128, beat, LINEARBLEND));
	blur2d(leds, LED_W, LED_H, 32);
	theta = fwd ? theta + 5 : theta - 3;
	t++;
	return beatCount >= startBeat + F0_BEATS ? /*getRandomExcluding(*/0/*)*/ : 0;
}

#define F1_BEATS 12
uint8_t func1(uint32_t startBeat)
{
	static uint8_t t = 0;
	int8_t dist = (int8_t)(beat / 5);
	for (uint8_t ang = 0; ang < 255; ++ang)
	{
		uint8_t x = (((int8_t)cos8(ang) * dist) / 256 + 5);
		uint8_t y = (((int8_t)sin8(ang) * dist) / 256 + 5);
		setLed(x, y, ColorFromPalette(RGBPalette, t, 255, LINEARBLEND));
	}
	blur2d(leds, LED_W, LED_H, 64);
	t += 5;
	return beatCount >= startBeat + F1_BEATS ? getRandomExcluding(1) : 1;
}

#define F2_BEATS 12
uint8_t func2(uint32_t startBeat)
{
	static uint8_t t;
	uint8_t col = t;
	for (int i = 0; i < 100; ++i)
	{
		leds[i] = ColorFromPalette(purplyPalette, col, beat < 100 ? 100 : beat, NOBLEND);
		col += 3;
	}
	t++;
	blur2d(leds, LED_W, LED_H, 64);
	return beatCount >= startBeat + F2_BEATS ? getRandomExcluding(2) : 2;
}

#define F3_BEATS 12
uint8_t func3(uint32_t startBeat)
{
	static int8_t x_coord = random(10);
	static int8_t y_coord = random(10);
	static int8_t x_dir = -1;
	static int8_t y_dir = 1;

	x_coord += x_dir;
	y_coord += y_dir;

	if (x_coord > 9)
	{
		x_coord = 8;
		x_dir = random(-3, 0);
	}

	if (x_coord < 0)
	{
		x_coord = 1;
		x_dir = random(1, 4);
	}

	if (y_coord > 9)
	{
		y_coord = 8;
		y_dir = random(-3, 0);
	}

	if (y_coord < 0)
	{
		y_coord = 1;
		y_dir = random(1, 4);
	}

	setLed(x_coord, y_coord, ColorFromPalette(PartyColors_p, beat, 255, LINEARBLEND));
	blur2d(leds, LED_W, LED_H, 32);

	delay(20);

	return beatCount >= startBeat + F3_BEATS ? getRandomExcluding(3) : 3;
}

#define COOLING 55
#define HOT 180
#define MAXHOT HOT*LED_H

#define F4_BEATS 14
uint8_t func4(uint32_t startBeat)
{
	static unsigned int spark[LED_W];
	CRGB stack[LED_W][LED_H];


	for( int i = 0; i < LED_W; i++) {
		if (spark[i] < HOT ) {
			int base = HOT * 2;
			spark[i] = random16( base, MAXHOT );
		}
	}

	for (int i = 0; i < LED_W; i++)
		spark[i] = qsub8(spark[i], random8(0, COOLING));

	for( int i = 0; i < LED_W; i++)
	{
		unsigned int heat = constrain(spark[i], HOT/2, MAXHOT);
		for( int j = LED_H-1; j >= 0; j--)
		{
			/* Calculate the color on the palette from how hot this
			pixel is */
			byte index = constrain(heat, 0, HOT);
			setLed(i, j, ColorFromPalette( HeatColors_p, index));

			/* The next higher pixel will be "cooler", so calculate
			the drop */
			unsigned int drop = random8(0,HOT);
			if (drop > heat) heat = 0; // avoid wrap-arounds from going "negative"
			else heat -= drop;

			heat = constrain(heat, 0, MAXHOT);
		}
	}
	blur2d(leds, LED_W, LED_H, 64);
	FastLED.setBrightness(beat);
	delay(40);
	return beatCount >= startBeat + F4_BEATS ? getRandomExcluding(4) : 4;
}

// TETRIS!!!
uint8_t func5(uint32_t startBeat)
{
	memset(leds, 0, LED_W * LED_H * 3);

	// paint bottom blocks
	setLed(0,8,CRGB::Red);
	setLed(0,9,CRGB::Red);
	setLed(1,8,CRGB::Red);
	setLed(1,9,CRGB::Red);

	for (int i = 0; i < 4; ++i)
		setLed(2, i + 5, CRGB::Yellow);
	for (int i = 0; i < 3; ++i)
		setLed(i + 2, 9, CRGB::Magenta);
	setLed(3, 8, CRGB::Magenta);

	setLed(6, 9, CRGB::Green);
	setLed(7, 9, CRGB::Green);
	setLed(7, 8, CRGB::Green);
	setLed(8, 8, CRGB::Green);

	setLed(8, 9, CRGB::Blue);
	for (int i = 0; i < 3; ++i)
		setLed(9, i + 7, CRGB::Blue);

	FastLED.show();
	//render tetris piece
	for (int i = 0; i < 9; ++i)
	{
		renderPiece(5, i-1, CRGB::Black);
		renderPiece(5, i, CRGB::Cyan);
		FastLED.show();
		delay(500);
	}

	delay(1000);

	CRGB buf[2 * LED_W];
	for (int i = 0; i < 2 * LED_W; ++i)
	{
		buf[i] = leds[8 * LED_W + i];
	}

	for (int i = 0; i < 4; ++i)
	{
		for (int x = 0; x < 2 * LED_W; ++x)
		{
			leds[8 * LED_W + x] = CRGB::Black;
		}
		FastLED.show();
		delay(100);
		for (int x = 0; x < 2 * LED_W; ++x)
		{
			leds[8 * LED_W + x] = buf[x];
		}
		FastLED.show();
		delay(100);
	}

	memset(leds, 0, LED_W * LED_H * 3);

	for (int i = 0; i < 3; ++i)
		setLed(2, i + 7, CRGB::Yellow);
	setLed(9, 9, CRGB::Blue);
	FastLED.show();
	delay(2000);

	return getRandomExcluding(5);
}

void renderPiece(uint8_t x, uint8_t y, CRGB col)
{
	setLed(x-1, y, col);
	setLed(x, y, col);
	setLed(x+1, y, col);
	setLed(x, y+1, col);
}

void setLed(uint8_t x, uint8_t y, CRGB col)
{
	if (y >= LED_H || y < 0 || x >= LED_W || x < 0)
		return;
	leds[(y * LED_W) + x] = col;
}

uint8_t getRandomExcluding(uint8_t exclude)
{
	uint8_t res;
	while ((res = random8(0, 6)) == exclude) ;
	return res;
}

uint16_t XY( uint8_t x, uint8_t y) { return (y * LED_W) + x; }

void buildRandomPurplyPalette()
{
	for (int i = 0; i < 16; i++)
		if (i % 2)
			purplyPalette[i] = CRGB(random8(70,200), random8(30),random8(100,256));
		else
			purplyPalette[i] = CRGB::Black;
}

const TProgmemPalette16 RGBPalette PROGMEM = 
{
	CRGB::Red,
	CRGB::Green,
	CRGB::Blue,
	CRGB::Black,

	CRGB::Red,
	CRGB::Green,
	CRGB::Blue,
	CRGB::Black,

	CRGB::Red,
	CRGB::Green,
	CRGB::Blue,
	CRGB::Black,

	CRGB::Red,
	CRGB::Green,
	CRGB::Blue,
	CRGB::Black
};
