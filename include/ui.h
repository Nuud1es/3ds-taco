#ifndef UI_H
#define UI_H

#include <3ds.h>
#include "obd.h"

// Color palette - Cassette Futurism inspired
#define COLOR_BG 0xFF1A1A1A         // Dark background
#define COLOR_PANEL 0xFF2A2A2A      // Panel background
#define COLOR_TEXT 0xFFE0E0E0       // Light text
#define COLOR_ACCENT 0xFFFF6B35     // Orange accent
#define COLOR_DIAL_BG 0xFF3A3A3A    // Dial background
#define COLOR_DIAL_MARKS 0xFF808080 // Dial marks
#define COLOR_NEEDLE 0xFFFF4444     // Red needle
#define COLOR_GREEN 0xFF44FF44      // Green indicator
#define COLOR_WARNING 0xFFFFAA00    // Yellow warning

void renderUI(OBDData* data, bool connected);
void drawAnalogDial(u8* fb, int centerX, int centerY, int radius, int value, int maxValue, const char* label);
void drawNeedle(u8* fb, int centerX, int centerY, int length, float angle);
void drawLine(u8* fb, int x0, int y0, int x1, int y1, u32 color);
void drawCircle(u8* fb, int centerX, int centerY, int radius, u32 color, bool filled);
void drawRect(u8* fb, int x, int y, int width, int height, u32 color, bool filled);
void drawText(u8* fb, int x, int y, const char* text, u32 color);
void setPixel(u8* fb, int x, int y, u32 color);

#endif
