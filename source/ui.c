#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
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

// Citro2D/3D state
static C3D_RenderTarget* topScreen;
static C3D_RenderTarget* bottomScreen;
static C2D_TextBuf textBuf;
static C2D_Font font;

bool initUI(void) {
    // Initialize Citro3D
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialize Citro2D
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // Create render targets
    topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottomScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Create text buffer (can hold 4096 glyphs)
    textBuf = C2D_TextBufNew(4096);

    // Load system font
    font = C2D_FontLoadSystem(CFG_REGION_USA);

    return true;
}

void exitUI(void) {
    // Free resources
    C2D_TextBufDelete(textBuf);
    C2D_FontFree(font);

    // Cleanup Citro2D/3D
    C2D_Fini();
    C3D_Fini();
}

void drawText(float x, float y, float size, u32 color, const char* text) {
    C2D_TextBufClear(textBuf);

    C2D_Text c2dText;
    C2D_TextParse(&c2dText, font, textBuf, text);
    C2D_TextOptimize(&c2dText);

    C2D_DrawText(&c2dText, C2D_WithColor, x, y, 0.5f, size, size, color);
}

void drawTextCentered(float y, float size, u32 color, const char* text) {
    C2D_TextBufClear(textBuf);

    C2D_Text c2dText;
    C2D_TextParse(&c2dText, font, textBuf, text);
    C2D_TextOptimize(&c2dText);

    float width, height;
    C2D_TextGetDimensions(&c2dText, size, size, &width, &height);

    float x = (TOP_WIDTH - width) / 2.0f;

    C2D_DrawText(&c2dText, C2D_WithColor, x, y, 0.5f, size, size, color);
}

void drawLine(int x0, int y0, int x1, int y1, u32 color, float thickness) {
    // Calculate line angle and length
    float dx = x1 - x0;
    float dy = y1 - y0;
    float length = sqrtf(dx * dx + dy * dy);
    float angle = atan2f(dy, dx);

    // Draw as a thin rectangle
    C2D_DrawRectangle(x0, y0 - thickness/2, 0.5f, length, thickness, color, color, color, color);
}

void drawCircle(int centerX, int centerY, int radius, u32 color, bool filled) {
    if (filled) {
        C2D_DrawCircleSolid(centerX, centerY, 0.5f, radius, color);
    } else {
        // Draw circle outline by drawing a ring
        int segments = 60;
        float angleStep = 2.0f * M_PI / segments;

        for (int i = 0; i < segments; i++) {
            float angle1 = i * angleStep;
            float angle2 = (i + 1) * angleStep;

            int x1 = centerX + (int)(cosf(angle1) * radius);
            int y1 = centerY + (int)(sinf(angle1) * radius);
            int x2 = centerX + (int)(cosf(angle2) * radius);
            int y2 = centerY + (int)(sinf(angle2) * radius);

            drawLine(x1, y1, x2, y2, color, 2.0f);
        }
    }
}

void drawRect(int x, int y, int width, int height, u32 color, bool filled) {
    if (filled) {
        C2D_DrawRectSolid(x, y, 0.5f, width, height, color);
    } else {
        // Draw rectangle outline
        C2D_DrawRectSolid(x, y, 0.5f, width, 2, color);  // Top
        C2D_DrawRectSolid(x, y + height - 2, 0.5f, width, 2, color);  // Bottom
        C2D_DrawRectSolid(x, y, 0.5f, 2, height, color);  // Left
        C2D_DrawRectSolid(x + width - 2, y, 0.5f, 2, height, color);  // Right
    }
}

void drawNeedle(int centerX, int centerY, int length, float angle) {
    // Convert angle to radians (0 degrees = right, counter-clockwise)
    float radians = (angle - 90.0f) * M_PI / 180.0f;

    int endX = centerX + (int)(cosf(radians) * length);
    int endY = centerY + (int)(sinf(radians) * length);

    // Draw needle with thickness
    drawLine(centerX, centerY, endX, endY, COLOR_NEEDLE, 3.0f);

    // Draw center cap
    drawCircle(centerX, centerY, 6, COLOR_NEEDLE, true);
    drawCircle(centerX, centerY, 4, COLOR_BG, true);
}

void drawAnalogDial(int centerX, int centerY, int radius, int value, int maxValue, const char* label, bool showValue) {
    // Draw dial background panel
    drawCircle(centerX, centerY, radius + 10, COLOR_PANEL, true);
    drawCircle(centerX, centerY, radius + 5, COLOR_DIAL_BG, true);

    // Draw dial markings (tick marks)
    for (int i = 0; i <= 10; i++) {
        float angle = 135.0f + (i * 27.0f); // 135° to 405° (270° sweep)
        float radians = (angle - 90.0f) * M_PI / 180.0f;

        int markLength = (i % 2 == 0) ? 15 : 8;
        int innerRadius = radius - markLength;

        int x1 = centerX + (int)(cosf(radians) * innerRadius);
        int y1 = centerY + (int)(sinf(radians) * innerRadius);
        int x2 = centerX + (int)(cosf(radians) * radius);
        int y2 = centerY + (int)(sinf(radians) * radius);

        drawLine(x1, y1, x2, y2, COLOR_DIAL_MARKS, 2.0f);
    }

    // Draw dial border
    drawCircle(centerX, centerY, radius + 5, COLOR_ACCENT, false);

    // Calculate needle angle based on value
    float percentage = (float)value / (float)maxValue;
    if (percentage > 1.0f) percentage = 1.0f;
    if (percentage < 0.0f) percentage = 0.0f;

    float needleAngle = 135.0f + (percentage * 270.0f); // 135° to 405°

    // Draw needle
    drawNeedle(centerX, centerY, radius - 20, needleAngle);

    // Draw label at top of dial
    if (label) {
        C2D_TextBufClear(textBuf);
        C2D_Text c2dText;
        C2D_TextParse(&c2dText, font, textBuf, label);
        C2D_TextOptimize(&c2dText);

        float textWidth, textHeight;
        C2D_TextGetDimensions(&c2dText, 0.5f, 0.5f, &textWidth, &textHeight);

        float textX = centerX - textWidth / 2.0f;
        float textY = centerY - radius - 20;

        C2D_DrawText(&c2dText, C2D_WithColor, textX, textY, 0.5f, 0.5f, 0.5f, COLOR_ACCENT);
    }

    // Draw value at bottom of dial
    if (showValue) {
        char valueStr[32];
        snprintf(valueStr, sizeof(valueStr), "%d", value);

        C2D_TextBufClear(textBuf);
        C2D_Text c2dText;
        C2D_TextParse(&c2dText, font, textBuf, valueStr);
        C2D_TextOptimize(&c2dText);

        float textWidth, textHeight;
        C2D_TextGetDimensions(&c2dText, 0.8f, 0.8f, &textWidth, &textHeight);

        float textX = centerX - textWidth / 2.0f;
        float textY = centerY + radius - 30;

        C2D_DrawText(&c2dText, C2D_WithColor, textX, textY, 0.5f, 0.8f, 0.8f, COLOR_TEXT);
    }
}

void renderTopScreen(OBDData* data) {
    // Clear with background color
    C2D_TargetClear(topScreen, COLOR_BG);
    C2D_SceneBegin(topScreen);

    // Draw title panel
    drawRect(10, 10, TOP_WIDTH - 20, 30, COLOR_PANEL, true);
    drawRect(10, 10, TOP_WIDTH - 20, 30, COLOR_ACCENT, false);

    // Draw title text
    drawText(20, 17, 0.6f, COLOR_TEXT, "3DS-TACO");
    drawText(TOP_WIDTH - 150, 17, 0.5f, COLOR_ACCENT, "OBD-II TELEMETRY");

    // Draw decorative lines (cassette futurism style)
    C2D_DrawRectSolid(20, 50, 0.5f, 360, 2, COLOR_ACCENT);
    C2D_DrawRectSolid(20, 53, 0.5f, 360, 1, COLOR_ACCENT);

    // Draw MPH dial (left side)
    drawAnalogDial(100, 150, 70, data->speed, 140, "MPH", true);

    // Draw RPM dial (right side)
    drawAnalogDial(300, 150, 70, data->rpm / 100, 80, "RPM x100", true);

    // Draw connection indicator
    if (data->valid) {
        drawCircle(TOP_WIDTH - 30, 25, 8, COLOR_GREEN, true);
    } else {
        drawCircle(TOP_WIDTH - 30, 25, 8, COLOR_WARNING, true);
    }
}

void renderBottomScreen(OBDData* data, bool connected) {
    // Clear with background color
    C2D_TargetClear(bottomScreen, COLOR_BG);
    C2D_SceneBegin(bottomScreen);

    if (!connected) {
        // Show connection prompt
        drawRect(10, 10, BOTTOM_WIDTH - 20, BOTTOM_HEIGHT - 20, COLOR_PANEL, true);
        drawRect(10, 10, BOTTOM_WIDTH - 20, BOTTOM_HEIGHT - 20, COLOR_ACCENT, false);

        drawCircle(BOTTOM_WIDTH / 2, 80, 30, COLOR_WARNING, false);
        drawCircle(BOTTOM_WIDTH / 2, 80, 28, COLOR_WARNING, false);

        drawTextCentered(130, 0.6f, COLOR_TEXT, "NOT CONNECTED");
        drawTextCentered(155, 0.5f, COLOR_DIAL_MARKS, "Press A to connect");
        drawTextCentered(175, 0.5f, COLOR_DIAL_MARKS, "Press START to exit");

        return;
    }

    // Draw telemetry data panels
    char buffer[64];

    // Panel layout (2x3 grid)
    struct {
        int x, y;
        const char* label;
        int value;
        const char* unit;
    } panels[] = {
        {20, 20, "THROTTLE", data->throttle, "%"},
        {170, 20, "COOLANT", data->coolantTemp, "F"},
        {20, 90, "ENGINE LOAD", data->engineLoad, "%"},
        {170, 90, "INTAKE", data->intakeTemp, "F"},
        {20, 160, "FUEL LEVEL", data->fuelLevel, "%"},
        {170, 160, "MAF", (int)data->maf, "g/s"}
    };

    for (int i = 0; i < 6; i++) {
        int panelWidth = 140;
        int panelHeight = 60;

        // Draw panel background
        drawRect(panels[i].x, panels[i].y, panelWidth, panelHeight, COLOR_PANEL, true);
        drawRect(panels[i].x, panels[i].y, panelWidth, panelHeight, COLOR_ACCENT, false);

        // Draw label
        drawText(panels[i].x + 5, panels[i].y + 5, 0.4f, COLOR_DIAL_MARKS, panels[i].label);

        // Draw value
        snprintf(buffer, sizeof(buffer), "%d %s", panels[i].value, panels[i].unit);

        C2D_TextBufClear(textBuf);
        C2D_Text c2dText;
        C2D_TextParse(&c2dText, font, textBuf, buffer);
        C2D_TextOptimize(&c2dText);

        float textWidth, textHeight;
        C2D_TextGetDimensions(&c2dText, 0.7f, 0.7f, &textWidth, &textHeight);

        float textX = panels[i].x + (panelWidth - textWidth) / 2.0f;
        float textY = panels[i].y + 30;

        C2D_DrawText(&c2dText, C2D_WithColor, textX, textY, 0.5f, 0.7f, 0.7f, COLOR_TEXT);
    }
}

void renderUI(OBDData* data, bool connected) {
    // Render top screen
    renderTopScreen(data);

    // Render bottom screen
    renderBottomScreen(data, connected);

    // Finalize frame
    C3D_FrameEnd(0);
}
