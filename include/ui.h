#ifndef UI_H
#define UI_H

#include <3ds.h>
#include <citro2d.h>
#include "obd.h"

// Color palette - Cassette Futurism inspired (RGBA8 format for Citro2D)
#define COLOR_BG       C2D_Color32(0x1A, 0x1A, 0x1A, 0xFF)  // Dark background
#define COLOR_PANEL    C2D_Color32(0x2A, 0x2A, 0x2A, 0xFF)  // Panel background
#define COLOR_TEXT     C2D_Color32(0xE0, 0xE0, 0xE0, 0xFF)  // Light text
#define COLOR_ACCENT   C2D_Color32(0xFF, 0x6B, 0x35, 0xFF)  // Orange accent
#define COLOR_DIAL_BG  C2D_Color32(0x3A, 0x3A, 0x3A, 0xFF)  // Dial background
#define COLOR_DIAL_MARKS C2D_Color32(0x80, 0x80, 0x80, 0xFF) // Dial marks
#define COLOR_NEEDLE   C2D_Color32(0xFF, 0x44, 0x44, 0xFF)  // Red needle
#define COLOR_GREEN    C2D_Color32(0x44, 0xFF, 0x44, 0xFF)  // Green indicator
#define COLOR_WARNING  C2D_Color32(0xFF, 0xAA, 0x00, 0xFF)  // Yellow warning

// Initialize UI system
bool initUI(void);
void exitUI(void);

// Rendering functions
void renderUI(OBDData* data, bool connected);
void beginFrame(C3D_RenderTarget* target);
void endFrame(void);

// Drawing primitives
void drawAnalogDial(int centerX, int centerY, int radius, int value, int maxValue, const char* label, bool showValue);
void drawNeedle(int centerX, int centerY, int length, float angle);
void drawLine(int x0, int y0, int x1, int y1, u32 color, float thickness);
void drawCircle(int centerX, int centerY, int radius, u32 color, bool filled);
void drawRect(int x, int y, int width, int height, u32 color, bool filled);

// Text rendering
void drawText(float x, float y, float size, u32 color, const char* text);
void drawTextCentered(float y, float size, u32 color, const char* text);

#endif
