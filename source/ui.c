#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ui.h"
#include "obd.h"

#define TOP_WIDTH 400
#define TOP_HEIGHT 240
#define BOTTOM_WIDTH 320
#define BOTTOM_HEIGHT 240

static inline void setPixel(u8* fb, int x, int y, u32 color) {
    if (x < 0 || y < 0 || x >= TOP_WIDTH || y >= TOP_HEIGHT) return;

    // 3DS framebuffer is rotated and in BGR8 format
    int offset = ((TOP_HEIGHT - 1 - y) + x * TOP_HEIGHT) * 3;
    fb[offset] = color & 0xFF;         // Blue
    fb[offset + 1] = (color >> 8) & 0xFF;  // Green
    fb[offset + 2] = (color >> 16) & 0xFF; // Red
}

void drawLine(u8* fb, int x0, int y0, int x1, int y1, u32 color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        setPixel(fb, x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void drawCircle(u8* fb, int centerX, int centerY, int radius, u32 color, bool filled) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        if (filled) {
            drawLine(fb, centerX - x, centerY + y, centerX + x, centerY + y, color);
            drawLine(fb, centerX - x, centerY - y, centerX + x, centerY - y, color);
            drawLine(fb, centerX - y, centerY + x, centerX + y, centerY + x, color);
            drawLine(fb, centerX - y, centerY - x, centerX + y, centerY - x, color);
        } else {
            setPixel(fb, centerX + x, centerY + y, color);
            setPixel(fb, centerX + y, centerY + x, color);
            setPixel(fb, centerX - y, centerY + x, color);
            setPixel(fb, centerX - x, centerY + y, color);
            setPixel(fb, centerX - x, centerY - y, color);
            setPixel(fb, centerX - y, centerY - x, color);
            setPixel(fb, centerX + y, centerY - x, color);
            setPixel(fb, centerX + x, centerY - y, color);
        }

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

void drawRect(u8* fb, int x, int y, int width, int height, u32 color, bool filled) {
    if (filled) {
        for (int i = 0; i < height; i++) {
            drawLine(fb, x, y + i, x + width - 1, y + i, color);
        }
    } else {
        drawLine(fb, x, y, x + width - 1, y, color);
        drawLine(fb, x, y + height - 1, x + width - 1, y + height - 1, color);
        drawLine(fb, x, y, x, y + height - 1, color);
        drawLine(fb, x + width - 1, y, x + width - 1, y + height - 1, color);
    }
}

void drawNeedle(u8* fb, int centerX, int centerY, int length, float angle) {
    // Convert angle to radians (0 degrees = bottom, clockwise)
    float radians = (angle - 90.0f) * M_PI / 180.0f;

    int endX = centerX + (int)(cos(radians) * length);
    int endY = centerY + (int)(sin(radians) * length);

    // Draw needle with thickness
    drawLine(fb, centerX, centerY, endX, endY, COLOR_NEEDLE);
    drawLine(fb, centerX + 1, centerY, endX + 1, endY, COLOR_NEEDLE);
    drawLine(fb, centerX, centerY + 1, endX, endY + 1, COLOR_NEEDLE);

    // Draw center cap
    drawCircle(fb, centerX, centerY, 5, COLOR_NEEDLE, true);
}

void drawAnalogDial(u8* fb, int centerX, int centerY, int radius, int value, int maxValue, const char* label) {
    // Draw dial background panel
    drawCircle(fb, centerX, centerY, radius + 10, COLOR_PANEL, true);
    drawCircle(fb, centerX, centerY, radius + 5, COLOR_DIAL_BG, true);

    // Draw dial markings (tick marks)
    for (int i = 0; i <= 10; i++) {
        float angle = 135.0f + (i * 27.0f); // 135° to 405° (270° sweep)
        float radians = (angle - 90.0f) * M_PI / 180.0f;

        int markLength = (i % 2 == 0) ? 15 : 8;
        int innerRadius = radius - markLength;

        int x1 = centerX + (int)(cos(radians) * innerRadius);
        int y1 = centerY + (int)(sin(radians) * innerRadius);
        int x2 = centerX + (int)(cos(radians) * radius);
        int y2 = centerY + (int)(sin(radians) * radius);

        drawLine(fb, x1, y1, x2, y2, COLOR_DIAL_MARKS);
    }

    // Draw dial border
    drawCircle(fb, centerX, centerY, radius + 5, COLOR_ACCENT, false);
    drawCircle(fb, centerX, centerY, radius + 6, COLOR_ACCENT, false);

    // Calculate needle angle based on value
    float percentage = (float)value / (float)maxValue;
    if (percentage > 1.0f) percentage = 1.0f;
    if (percentage < 0.0f) percentage = 0.0f;

    float needleAngle = 135.0f + (percentage * 270.0f); // 135° to 405°

    // Draw needle
    drawNeedle(fb, centerX, centerY, radius - 20, needleAngle);

    // Note: Text rendering would require a font library
    // For now, we'll skip the label text
}

void renderTopScreen(u8* fb, OBDData* data) {
    // Clear screen with background color
    for (int y = 0; y < TOP_HEIGHT; y++) {
        drawLine(fb, 0, y, TOP_WIDTH - 1, y, COLOR_BG);
    }

    // Draw title area
    drawRect(fb, 10, 10, TOP_WIDTH - 20, 30, COLOR_PANEL, true);
    drawRect(fb, 10, 10, TOP_WIDTH - 20, 30, COLOR_ACCENT, false);

    // Draw decorative lines (cassette futurism style)
    drawLine(fb, 20, 50, 380, 50, COLOR_ACCENT);
    drawLine(fb, 20, 52, 380, 52, COLOR_ACCENT);

    // Draw MPH dial (left side)
    drawAnalogDial(fb, 100, 150, 70, data->speed, 140, "MPH");

    // Draw RPM dial (right side)
    drawAnalogDial(fb, 300, 150, 70, data->rpm / 100, 80, "RPM x100");

    // Draw connection indicator
    if (data->valid) {
        drawCircle(fb, TOP_WIDTH - 30, 25, 8, COLOR_GREEN, true);
    } else {
        drawCircle(fb, TOP_WIDTH - 30, 25, 8, COLOR_WARNING, true);
    }
}

void renderBottomScreen(u8* fb, OBDData* data, bool connected) {
    // Clear screen
    for (int y = 0; y < BOTTOM_HEIGHT; y++) {
        drawLine(fb, 0, y, BOTTOM_WIDTH - 1, y, COLOR_BG);
    }

    // Draw panels for telemetry data
    int panelWidth = 140;
    int panelHeight = 50;
    int spacing = 20;

    // Panel layout (2x3 grid)
    int positions[6][2] = {
        {20, 20},
        {180, 20},
        {20, 90},
        {180, 90},
        {20, 160},
        {180, 160}
    };

    for (int i = 0; i < 6; i++) {
        drawRect(fb, positions[i][0], positions[i][1], panelWidth, panelHeight, COLOR_PANEL, true);
        drawRect(fb, positions[i][0], positions[i][1], panelWidth, panelHeight, COLOR_ACCENT, false);
    }

    // Note: In a real implementation, we would render text with actual values
    // This would require integrating a font rendering library

    // Draw connection status message area
    drawRect(fb, 10, 10, BOTTOM_WIDTH - 20, 220, COLOR_PANEL, false);

    if (!connected) {
        // Draw warning indicator
        drawCircle(fb, BOTTOM_WIDTH / 2, 120, 30, COLOR_WARNING, false);
        drawCircle(fb, BOTTOM_WIDTH / 2, 120, 28, COLOR_WARNING, false);
    }
}

void renderUI(OBDData* data, bool connected) {
    // Get framebuffers
    u8* fbTop = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    u8* fbBottom = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);

    // Render top screen (analog dials)
    renderTopScreen(fbTop, data);

    // Render bottom screen (telemetry data)
    renderBottomScreen(fbBottom, data, connected);
}
