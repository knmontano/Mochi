#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define TIMER_ANIM 1

// Day Mode Color Palette
#define COLOR_WALL_DAY       RGB(252, 247, 241)
#define COLOR_FLOOR_DAY      RGB(226, 185, 142)
#define COLOR_FLOOR_LINE_DAY RGB(198, 155, 114)
#define COLOR_RUG_DAY        RGB(255, 218, 186)
#define COLOR_RUG_RING_DAY   RGB(255, 240, 225)
#define COLOR_SKY_DAY        RGB(180, 226, 252)

// Night / Dark Mode Color Palette
#define COLOR_WALL_NIGHT       RGB(28, 24, 38)
#define COLOR_FLOOR_NIGHT      RGB(48, 38, 34)
#define COLOR_FLOOR_LINE_NIGHT RGB(34, 26, 24)
#define COLOR_RUG_NIGHT        RGB(76, 50, 68)
#define COLOR_RUG_RING_NIGHT   RGB(95, 65, 84)
#define COLOR_SKY_NIGHT        RGB(12, 14, 28)

// The Chubby Cat Fur Palette
#define COLOR_CAT_BASE       RGB(252, 168, 76)
#define COLOR_CAT_STRIPE     RGB(214, 112, 36)
#define COLOR_CAT_BELLY      RGB(255, 240, 222)
#define COLOR_CAT_NOSE       RGB(255, 150, 165)
#define COLOR_CAT_EYES       RGB(92, 196, 128)
#define COLOR_PUPIL          RGB(30, 24, 25)

// Ceramic Grey Feeding Bowl
#define COLOR_BOWL_OUTER     RGB(175, 180, 188)
#define COLOR_BOWL_INNER     RGB(142, 148, 158)
#define COLOR_BOWL_RIM       RGB(210, 215, 222)
#define COLOR_SALMON         RGB(255, 122, 98)
#define COLOR_SHRIMP         RGB(255, 144, 124)

// Props & Accessories
#define COLOR_STAND_WOOD     RGB(165, 115, 80)
#define COLOR_WATER_BLUE     RGB(182, 230, 255)
#define COLOR_FISH_GOLD      RGB(255, 118, 48)
#define COLOR_BOWL_GLASS     RGB(140, 200, 235)
#define COLOR_PLANT_GREEN    RGB(75, 152, 92)
#define COLOR_YARN           RGB(255, 102, 160)

// Lamp Colors
#define COLOR_LAMP_POLE      RGB(110, 95, 85)
#define COLOR_SHADE_OFF      RGB(190, 175, 160)
#define COLOR_SHADE_ON       RGB(255, 235, 150)
#define COLOR_LAMP_GLOW      RGB(255, 242, 180)

// UI Accent Colors
#define COLOR_CARD_BORDER    RGB(236, 212, 190)
#define COLOR_TEXT_MAIN      RGB(72, 48, 38)
#define COLOR_TEXT_MUTED     RGB(152, 122, 108)
#define COLOR_BTN_BG         RGB(255, 242, 230)
#define COLOR_BTN_BORDER     RGB(232, 190, 154)

#define COLOR_BAR_BG         RGB(242, 236, 230)
#define COLOR_BAR_HEART      RGB(255, 122, 148)
#define COLOR_BAR_FOOD       RGB(255, 164, 68)
#define COLOR_BAR_ENERGY     RGB(110, 202, 238)

typedef struct {
    float x, y;
    float vx, vy;
    int life;
    COLORREF color;
    wchar_t text[8];
} FloatParticle;

#define MAX_PARTS 40
FloatParticle particles[MAX_PARTS];

typedef struct {
    char name[32];
    int happiness;
    int fullness;
    int energy;
    float weight;
    int mood; // 0: Normal, 1: Eating, 2: Zoomies, 3: In Box, 4: Purring, 5: Catnip Frenzy
    int moodTimer;
    float posX;
    float dirX;
} ChubbyCat;

ChubbyCat cat = {"Mochi", 88, 70, 85, 6.2f, 0, 0, 215.0f, 1.0f};
float animTimer = 0.0f;
float fishSwim = 0.0f;
float yarnBounce = 0.0f;
float bubbleTimer = 0.0f;
bool lampOn = true; // Lamp & Daylight toggle
char statusMessage[128] = "Mochi happily loafs on her rug. Click the lamp to switch lighting!";

HFONT hFontTitle = NULL;
HFONT hFontBody  = NULL;
HFONT hFontSmall = NULL;
HFONT hFontPop   = NULL;

RECT btnFeed    = { 40,  375, 130, 415 };
RECT btnPlay    = { 140, 375, 230, 415 };
RECT btnBox     = { 240, 375, 330, 415 };
RECT btnCatnip  = { 340, 375, 430, 415 };
RECT btnZoomies = { 440, 375, 530, 415 };
RECT btnNap     = { 540, 375, 630, 415 };

void spawnPopText(float x, float y, const wchar_t* txt, COLORREF col) {
    for (int i = 0; i < MAX_PARTS; i++) {
        if (particles[i].life <= 0) {
            particles[i].x = x;
            particles[i].y = y;
            particles[i].vx = ((rand() % 11) - 5) * 0.3f;
            particles[i].vy = -1.2f - ((rand() % 10) * 0.1f);
            particles[i].life = 35;
            particles[i].color = col;
            wcsncpy(particles[i].text, txt, 7);
            particles[i].text[7] = L'\0';
            break;
        }
    }
}

void drawRoundedBox(HDC hdc, RECT rc, COLORREF bg, COLORREF border, int radius) {
    HBRUSH b = CreateSolidBrush(bg);
    HPEN p = CreatePen(PS_SOLID, 1, border);
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, b);
    HPEN oldP = (HPEN)SelectObject(hdc, p);
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
    SelectObject(hdc, oldB);
    SelectObject(hdc, oldP);
    DeleteObject(b);
    DeleteObject(p);
}

void drawMeter(HDC hdc, int x, int y, int w, int h, int value, COLORREF fillCol, const wchar_t* label) {
    SetTextColor(hdc, COLOR_TEXT_MAIN);
    SelectObject(hdc, hFontSmall);
    TextOutW(hdc, x, y - 18, label, wcslen(label));

    wchar_t valBuf[16];
    swprintf(valBuf, 16, L"%d%%", value);
    TextOutW(hdc, x + w - 32, y - 18, valBuf, wcslen(valBuf));

    RECT bgRc = { x, y, x + w, y + h };
    drawRoundedBox(hdc, bgRc, COLOR_BAR_BG, COLOR_CARD_BORDER, 6);

    int fillW = (w * value) / 100;
    if (fillW > 0) {
        RECT fillRc = { x, y, x + fillW, y + h };
        drawRoundedBox(hdc, fillRc, fillCol, fillCol, 6);
    }
}

void drawCozyRoom(HDC hdc, int left, int top, int right, int bottom) {
    int floorY = top + 155;

    COLORREF colWall = lampOn ? COLOR_WALL_DAY : COLOR_WALL_NIGHT;
    COLORREF colFloor = lampOn ? COLOR_FLOOR_DAY : COLOR_FLOOR_NIGHT;
    COLORREF colPlank = lampOn ? COLOR_FLOOR_LINE_DAY : COLOR_FLOOR_LINE_NIGHT;
    COLORREF colRug = lampOn ? COLOR_RUG_DAY : COLOR_RUG_NIGHT;
    COLORREF colRugRing = lampOn ? COLOR_RUG_RING_DAY : COLOR_RUG_RING_NIGHT;
    COLORREF colSky = lampOn ? COLOR_SKY_DAY : COLOR_SKY_NIGHT;

    // 1. Room Wall
    HBRUSH wallB = CreateSolidBrush(colWall);
    RECT wallRc = { left, top, right, floorY };
    FillRect(hdc, &wallRc, wallB);
    DeleteObject(wallB);

    // 2. Window (Sun in Day / Moon & Stars at Night)
    RECT winRc = { left + 35, top + 18, left + 135, top + 115 };
    HBRUSH skyB = CreateSolidBrush(colSky);
    FillRect(hdc, &winRc, skyB);
    DeleteObject(skyB);

    if (lampOn) {
        // Daytime Sun
        HBRUSH sunB = CreateSolidBrush(RGB(255, 235, 120));
        SelectObject(hdc, sunB);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, winRc.right - 38, winRc.top + 8, winRc.right - 8, winRc.top + 38);
        DeleteObject(sunB);
    } else {
        // Nighttime Crescent Moon & Stars
        HBRUSH moonB = CreateSolidBrush(RGB(245, 240, 190));
        SelectObject(hdc, moonB);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, winRc.right - 36, winRc.top + 10, winRc.right - 10, winRc.top + 36);
        // Shadow to make crescent
        HBRUSH skyShadow = CreateSolidBrush(COLOR_SKY_NIGHT);
        SelectObject(hdc, skyShadow);
        Ellipse(hdc, winRc.right - 42, winRc.top + 10, winRc.right - 16, winRc.top + 36);
        DeleteObject(moonB);
        DeleteObject(skyShadow);

        // Twinkling Stars
        HBRUSH starB = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, starB);
        Ellipse(hdc, winRc.left + 15, winRc.top + 20, winRc.left + 18, winRc.top + 23);
        Ellipse(hdc, winRc.left + 45, winRc.top + 35, winRc.left + 47, winRc.top + 37);
        Ellipse(hdc, winRc.left + 25, winRc.top + 60, winRc.left + 28, winRc.top + 63);
        DeleteObject(starB);
    }

    HPEN winP = CreatePen(PS_SOLID, 4, lampOn ? RGB(255, 255, 255) : RGB(85, 75, 95));
    SelectObject(hdc, winP);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, winRc.left, winRc.top, winRc.right, winRc.bottom);
    MoveToEx(hdc, (winRc.left + winRc.right) / 2, winRc.top, NULL);
    LineTo(hdc, (winRc.left + winRc.right) / 2, winRc.bottom);
    MoveToEx(hdc, winRc.left, (winRc.top + winRc.bottom) / 2, NULL);
    LineTo(hdc, winRc.right, (winRc.top + winRc.bottom) / 2);
    DeleteObject(winP);

    // 3. Wooden Floor
    HBRUSH floorB = CreateSolidBrush(colFloor);
    RECT floorRc = { left, floorY, right, bottom };
    FillRect(hdc, &floorRc, floorB);
    DeleteObject(floorB);

    HPEN baseP = CreatePen(PS_SOLID, 4, lampOn ? RGB(188, 142, 102) : RGB(40, 30, 30));
    SelectObject(hdc, baseP);
    MoveToEx(hdc, left, floorY, NULL);
    LineTo(hdc, right, floorY);
    DeleteObject(baseP);

    HPEN plankP = CreatePen(PS_SOLID, 1, colPlank);
    SelectObject(hdc, plankP);
    for (int y = floorY + 28; y < bottom; y += 28) {
        MoveToEx(hdc, left, y, NULL);
        LineTo(hdc, right, y);
    }
    DeleteObject(plankP);

    // 4. Center Oval Rug
    HBRUSH rugB = CreateSolidBrush(colRug);
    HPEN rugP = CreatePen(PS_SOLID, 3, colRugRing);
    SelectObject(hdc, rugB);
    SelectObject(hdc, rugP);
    Ellipse(hdc, left + 90, floorY + 24, left + 345, bottom - 18);
    DeleteObject(rugB);
    DeleteObject(rugP);

    // 5. Left Zone: Stand with Aquarium Bowl
    int fbx = left + 55;
    int fby = floorY + 30;

    HBRUSH standB = CreateSolidBrush(lampOn ? COLOR_STAND_WOOD : RGB(75, 45, 30));
    SelectObject(hdc, standB);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    RoundRect(hdc, fbx - 26, fby + 28, fbx + 26, fby + 76, 6, 6);
    DeleteObject(standB);

    HBRUSH waterB = CreateSolidBrush(lampOn ? COLOR_WATER_BLUE : RGB(70, 110, 140));
    SelectObject(hdc, waterB);
    Ellipse(hdc, fbx - 24, fby - 4, fbx + 24, fby + 34);
    DeleteObject(waterB);

    HPEN weedP = CreatePen(PS_SOLID, 2, COLOR_PLANT_GREEN);
    SelectObject(hdc, weedP);
    MoveToEx(hdc, fbx - 12, fby + 26, NULL); LineTo(hdc, fbx - 14, fby + 10);
    MoveToEx(hdc, fbx - 8, fby + 26, NULL);  LineTo(hdc, fbx - 6, fby + 14);
    DeleteObject(weedP);

    int fishX = fbx + (int)(sinf(fishSwim) * 11.0f);
    int fishY = fby + 14 + (int)(cosf(fishSwim * 1.6f) * 4.0f);
    int fishDir = (cosf(fishSwim) >= 0.0f) ? 1 : -1;

    HBRUSH fishB = CreateSolidBrush(COLOR_FISH_GOLD);
    SelectObject(hdc, fishB);
    Ellipse(hdc, fishX - 6, fishY - 3, fishX + 6, fishY + 3);
    POINT tailPts[3] = {
        { fishX - (6 * fishDir), fishY },
        { fishX - (11 * fishDir), fishY - 4 },
        { fishX - (11 * fishDir), fishY + 4 }
    };
    Polygon(hdc, tailPts, 3);
    DeleteObject(fishB);

    int bubY = fby + 20 - ((int)(bubbleTimer * 20.0f) % 22);
    HBRUSH bubB = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, bubB);
    Ellipse(hdc, fbx + 4, bubY, fbx + 8, bubY + 4);
    DeleteObject(bubB);

    HPEN glassP = CreatePen(PS_SOLID, 2, lampOn ? COLOR_BOWL_GLASS : RGB(90, 130, 160));
    SelectObject(hdc, glassP);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Ellipse(hdc, fbx - 26, fby - 6, fbx + 26, fby + 36);
    DeleteObject(glassP);

    // 6. Right Zone: Grey Ceramic Feeding Bowl
    int bx = left + 325;
    int by = floorY + 74;

    HBRUSH bShadB = CreateSolidBrush(lampOn ? RGB(196, 160, 126) : RGB(30, 24, 26));
    SelectObject(hdc, bShadB);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, bx - 26, by - 6, bx + 26, by + 16);
    DeleteObject(bShadB);

    HBRUSH bowlOuterB = CreateSolidBrush(COLOR_BOWL_OUTER);
    HPEN bowlBorderP = CreatePen(PS_SOLID, 1, RGB(120, 126, 136));
    SelectObject(hdc, bowlOuterB);
    SelectObject(hdc, bowlBorderP);
    Ellipse(hdc, bx - 24, by - 12, bx + 24, by + 12);
    DeleteObject(bowlOuterB);
    DeleteObject(bowlBorderP);

    HBRUSH bowlInnerB = CreateSolidBrush(COLOR_BOWL_INNER);
    SelectObject(hdc, bowlInnerB);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, bx - 20, by - 9, bx + 20, by + 9);
    DeleteObject(bowlInnerB);

    HPEN rimP = CreatePen(PS_SOLID, 1, COLOR_BOWL_RIM);
    SelectObject(hdc, rimP);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Ellipse(hdc, bx - 23, by - 11, bx + 23, by + 11);
    DeleteObject(rimP);

    HBRUSH salmonB = CreateSolidBrush(COLOR_SALMON);
    SelectObject(hdc, salmonB);
    Ellipse(hdc, bx - 14, by - 6, bx + 4, by + 4);
    DeleteObject(salmonB);

    HPEN salmP = CreatePen(PS_SOLID, 1, RGB(255, 230, 222));
    SelectObject(hdc, salmP);
    MoveToEx(hdc, bx - 10, by - 4, NULL); LineTo(hdc, bx - 6, by + 2);
    MoveToEx(hdc, bx - 4, by - 5, NULL);  LineTo(hdc, bx, by + 1);
    DeleteObject(salmP);

    HBRUSH shrimpB = CreateSolidBrush(COLOR_SHRIMP);
    SelectObject(hdc, shrimpB);
    Arc(hdc, bx + 2, by - 7, bx + 15, by + 6, bx + 15, by + 2, bx + 2, by - 4);
    DeleteObject(shrimpB);

    // 7. Interactive Yarn Ball
    int yx = left + 105;
    int yy = floorY + 84 - (int)yarnBounce;
    HBRUSH yarnB = CreateSolidBrush(COLOR_YARN);
    SelectObject(hdc, yarnB);
    Ellipse(hdc, yx - 13, yy - 13, yx + 13, yy + 13);
    DeleteObject(yarnB);

    HPEN yarnStrP = CreatePen(PS_SOLID, 2, COLOR_YARN);
    SelectObject(hdc, yarnStrP);
    MoveToEx(hdc, yx + 8, yy + 8, NULL);
    LineTo(hdc, yx + 24, yy + 16);
    LineTo(hdc, yx + 36, yy + 12);
    DeleteObject(yarnStrP);

    // 8. Interactive Standing Lamp (Right Side between rug & HUD)
    int lx = left + 365;
    int ly = floorY - 60;

    // Floor Base
    HBRUSH lampBaseB = CreateSolidBrush(RGB(75, 60, 52));
    SelectObject(hdc, lampBaseB);
    Ellipse(hdc, lx - 18, floorY + 45, lx + 18, floorY + 55);
    DeleteObject(lampBaseB);

    // Tall Brass Lamp Pole
    HPEN poleP = CreatePen(PS_SOLID, 3, COLOR_LAMP_POLE);
    SelectObject(hdc, poleP);
    MoveToEx(hdc, lx, floorY + 48, NULL);
    LineTo(hdc, lx, ly + 25);
    DeleteObject(poleP);

    // Lamp Shade
    HBRUSH shadeB = CreateSolidBrush(lampOn ? COLOR_SHADE_ON : COLOR_SHADE_OFF);
    HPEN shadeP = CreatePen(PS_SOLID, 1, lampOn ? RGB(235, 200, 100) : RGB(140, 130, 120));
    SelectObject(hdc, shadeB);
    SelectObject(hdc, shadeP);
    POINT shadePts[4] = {
        { lx - 12, ly + 4 },
        { lx + 12, ly + 4 },
        { lx + 22, ly + 26 },
        { lx - 22, ly + 26 }
    };
    Polygon(hdc, shadePts, 4);
    DeleteObject(shadeB);
    DeleteObject(shadeP);

    // Warm Light Ambient Glow if Lamp is On
    if (lampOn) {
        HBRUSH bulbB = CreateSolidBrush(RGB(255, 255, 220));
        SelectObject(hdc, bulbB);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, lx - 7, ly + 24, lx + 7, ly + 32);
        DeleteObject(bulbB);
    }
}

void drawTheChubbyCat(HDC hdc, int cx, int cy) {
    float breathe = sinf(animTimer * 2.2f) * 3.0f;
    float tailWag = sinf(animTimer * (cat.mood == 2 || cat.mood == 5 ? 7.5f : 2.5f)) * 14.0f;

    if (cat.mood == 3) {
        HBRUSH boxB = CreateSolidBrush(RGB(198, 148, 92));
        HPEN boxP = CreatePen(PS_SOLID, 2, RGB(145, 102, 58));
        SelectObject(hdc, boxB);
        SelectObject(hdc, boxP);
        RoundRect(hdc, cx - 85, cy + 10, cx + 85, cy + 82, 10, 10);

        HPEN tapeP = CreatePen(PS_SOLID, 4, RGB(230, 192, 140));
        SelectObject(hdc, tapeP);
        MoveToEx(hdc, cx - 40, cy + 44, NULL);
        LineTo(hdc, cx + 40, cy + 44);
        DeleteObject(tapeP);
        DeleteObject(boxB);
        DeleteObject(boxP);
    }

    if (cat.mood != 3) {
        HBRUSH shadB = CreateSolidBrush(lampOn ? RGB(226, 186, 154) : RGB(45, 30, 42));
        SelectObject(hdc, shadB);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 115, cy + 52, cx + 115, cy + 86);
        DeleteObject(shadB);

        int tailEndX = cx + 86 + (int)tailWag;
        int tailEndY = cy + 16 + (int)(tailWag * 0.4f);

        HPEN tailBase = CreatePen(PS_SOLID, 15, COLOR_CAT_BASE);
        SelectObject(hdc, tailBase);
        MoveToEx(hdc, cx + 55, cy + 46, NULL);
        LineTo(hdc, tailEndX, tailEndY);
        DeleteObject(tailBase);

        HPEN tailStripe = CreatePen(PS_SOLID, 15, COLOR_CAT_STRIPE);
        SelectObject(hdc, tailStripe);
        MoveToEx(hdc, cx + 68 + (int)(tailWag * 0.4f), cy + 34, NULL);
        LineTo(hdc, cx + 76 + (int)(tailWag * 0.6f), cy + 26);
        DeleteObject(tailStripe);

        HBRUSH bodyB = CreateSolidBrush(COLOR_CAT_BASE);
        SelectObject(hdc, bodyB);
        Ellipse(hdc, cx - 82, cy - 22 + (int)breathe, cx + 82, cy + 70 + (int)breathe);

        HBRUSH bellyB = CreateSolidBrush(COLOR_CAT_BELLY);
        SelectObject(hdc, bellyB);
        Ellipse(hdc, cx - 50, cy + 4 + (int)breathe, cx + 50, cy + 66 + (int)breathe);
        DeleteObject(bellyB);

        HPEN stripeP = CreatePen(PS_SOLID, 4, COLOR_CAT_STRIPE);
        SelectObject(hdc, stripeP);
        MoveToEx(hdc, cx - 78, cy + 10 + (int)breathe, NULL); LineTo(hdc, cx - 52, cy + 18 + (int)breathe);
        MoveToEx(hdc, cx - 74, cy + 28 + (int)breathe, NULL); LineTo(hdc, cx - 48, cy + 32 + (int)breathe);
        MoveToEx(hdc, cx + 78, cy + 10 + (int)breathe, NULL); LineTo(hdc, cx + 52, cy + 18 + (int)breathe);
        MoveToEx(hdc, cx + 74, cy + 28 + (int)breathe, NULL); LineTo(hdc, cx + 48, cy + 32 + (int)breathe);
        DeleteObject(stripeP);

        HBRUSH pawB = CreateSolidBrush(COLOR_CAT_BELLY);
        SelectObject(hdc, pawB);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 36, cy + 54 + (int)breathe, cx - 10, cy + 72 + (int)breathe);
        Ellipse(hdc, cx + 10, cy + 54 + (int)breathe, cx + 36, cy + 72 + (int)breathe);
        DeleteObject(pawB);
        DeleteObject(bodyB);
    } else {
        HBRUSH pawB = CreateSolidBrush(COLOR_CAT_BELLY);
        SelectObject(hdc, pawB);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 48, cy + 8, cx - 18, cy + 28);
        Ellipse(hdc, cx + 18, cy + 8, cx + 48, cy + 28);
        DeleteObject(pawB);
    }

    HBRUSH bodyB = CreateSolidBrush(COLOR_CAT_BASE);
    SelectObject(hdc, bodyB);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    POINT earLeft[3]  = { { cx - 52, cy - 36 }, { cx - 28, cy - 72 }, { cx - 12, cy - 38 } };
    POINT earRight[3] = { { cx + 12, cy - 38 }, { cx + 28, cy - 72 }, { cx + 52, cy - 36 } };
    Polygon(hdc, earLeft, 3);
    Polygon(hdc, earRight, 3);

    HBRUSH inEarB = CreateSolidBrush(COLOR_CAT_NOSE);
    SelectObject(hdc, inEarB);
    POINT inEarL[3] = { { cx - 44, cy - 36 }, { cx - 28, cy - 62 }, { cx - 18, cy - 38 } };
    POINT inEarR[3] = { { cx + 18, cy - 38 }, { cx + 28, cy - 62 }, { cx + 44, cy - 36 } };
    Polygon(hdc, inEarL, 3);
    Polygon(hdc, inEarR, 3);
    DeleteObject(inEarB);

    SelectObject(hdc, bodyB);
    Ellipse(hdc, cx - 58, cy - 56, cx + 58, cy + 18);
    DeleteObject(bodyB);

    HPEN stripeP = CreatePen(PS_SOLID, 4, COLOR_CAT_STRIPE);
    SelectObject(hdc, stripeP);
    MoveToEx(hdc, cx - 16, cy - 50, NULL); LineTo(hdc, cx - 12, cy - 36);
    MoveToEx(hdc, cx - 12, cy - 36, NULL); LineTo(hdc, cx, cy - 42);
    MoveToEx(hdc, cx, cy - 42, NULL);      LineTo(hdc, cx + 12, cy - 36);
    MoveToEx(hdc, cx + 12, cy - 36, NULL); LineTo(hdc, cx + 16, cy - 50);

    MoveToEx(hdc, cx - 52, cy - 12, NULL); LineTo(hdc, cx - 38, cy - 8);
    MoveToEx(hdc, cx + 52, cy - 12, NULL); LineTo(hdc, cx + 38, cy - 8);
    DeleteObject(stripeP);

    HBRUSH muzB = CreateSolidBrush(COLOR_CAT_BELLY);
    SelectObject(hdc, muzB);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, cx - 24, cy - 12, cx + 24, cy + 12);
    DeleteObject(muzB);

    HBRUSH noseB = CreateSolidBrush(COLOR_CAT_NOSE);
    SelectObject(hdc, noseB);
    POINT nosePt[3] = { { cx - 6, cy - 7 }, { cx + 6, cy - 7 }, { cx, cy - 1 } };
    Polygon(hdc, nosePt, 3);
    DeleteObject(noseB);

    HPEN mouthP = CreatePen(PS_SOLID, 2, COLOR_TEXT_MAIN);
    SelectObject(hdc, mouthP);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Arc(hdc, cx - 11, cy - 4, cx, cy + 7, cx, cy - 1, cx - 11, cy + 2);
    Arc(hdc, cx, cy - 4, cx + 11, cy + 7, cx + 11, cy + 2, cx, cy - 1);

    MoveToEx(hdc, cx - 22, cy - 2, NULL); LineTo(hdc, cx - 60, cy - 10);
    MoveToEx(hdc, cx - 22, cy + 3, NULL); LineTo(hdc, cx - 58, cy + 8);
    MoveToEx(hdc, cx + 22, cy - 2, NULL); LineTo(hdc, cx + 60, cy - 10);
    MoveToEx(hdc, cx + 22, cy + 3, NULL); LineTo(hdc, cx + 58, cy + 8);
    DeleteObject(mouthP);

    if (cat.mood == 5) {
        HBRUSH bigEye = CreateSolidBrush(COLOR_PUPIL);
        SelectObject(hdc, bigEye);
        Ellipse(hdc, cx - 35, cy - 26, cx - 13, cy - 2);
        Ellipse(hdc, cx + 13, cy - 26, cx + 35, cy - 2);
        DeleteObject(bigEye);

        HBRUSH sparkle = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, sparkle);
        Ellipse(hdc, cx - 31, cy - 22, cx - 23, cy - 14);
        Ellipse(hdc, cx + 17, cy - 22, cx + 25, cy - 14);
        DeleteObject(sparkle);
    } else if (cat.mood == 4) {
        HPEN eyeP = CreatePen(PS_SOLID, 3, COLOR_TEXT_MAIN);
        SelectObject(hdc, eyeP);
        MoveToEx(hdc, cx - 32, cy - 14, NULL); LineTo(hdc, cx - 16, cy - 14);
        MoveToEx(hdc, cx + 16, cy - 14, NULL); LineTo(hdc, cx + 32, cy - 14);
        DeleteObject(eyeP);
    } else {
        HBRUSH eyeB = CreateSolidBrush(COLOR_CAT_EYES);
        SelectObject(hdc, eyeB);
        SelectObject(hdc, GetStockObject(NULL_PEN));
        Ellipse(hdc, cx - 33, cy - 24, cx - 13, cy - 6);
        Ellipse(hdc, cx + 13, cy - 24, cx + 33, cy - 6);
        DeleteObject(eyeB);

        HBRUSH pupB = CreateSolidBrush(COLOR_PUPIL);
        SelectObject(hdc, pupB);
        Ellipse(hdc, cx - 26, cy - 21, cx - 20, cy - 8);
        Ellipse(hdc, cx + 20, cy - 21, cx + 26, cy - 8);
        DeleteObject(pupB);

        HBRUSH glintB = CreateSolidBrush(RGB(255, 255, 255));
        SelectObject(hdc, glintB);
        Ellipse(hdc, cx - 27, cy - 21, cx - 23, cy - 17);
        Ellipse(hdc, cx + 19, cy - 21, cx + 23, cy - 17);
        DeleteObject(glintB);
    }
}

void drawActionButton(HDC hdc, RECT rc, const wchar_t* icon, const wchar_t* title) {
    drawRoundedBox(hdc, rc, COLOR_BTN_BG, COLOR_BTN_BORDER, 8);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_TEXT_MAIN);
    SelectObject(hdc, hFontBody);
    
    wchar_t buf[32];
    swprintf(buf, 32, L"%ls %ls", icon, title);
    RECT tr = rc;
    DrawTextW(hdc, buf, -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void soundBoop() { Beep(880, 80); }
void soundPurr() { Beep(220, 100); Beep(240, 100); }
void soundZoom() { Beep(600, 50); Beep(800, 50); Beep(1100, 70); }
void soundNip()  { Beep(300, 120); }
void soundSwitch() { Beep(1200, 40); Beep(900, 50); }

void toggleLamp() {
    lampOn = !lampOn;
    soundSwitch();
    if (lampOn) {
        snprintf(statusMessage, sizeof(statusMessage), "*Click!* Warm morning light fills the room. Good morning, Mochi!");
        spawnPopText(380, 150, L"LIGHTS ON", RGB(255, 200, 60));
    } else {
        snprintf(statusMessage, sizeof(statusMessage), "*Click!* Cozy dark mode activated. The moon and stars shine outside.");
        spawnPopText(380, 150, L"DARK MODE", RGB(160, 180, 240));
    }
}

void feedCat() {
    if (cat.fullness >= 95) {
        snprintf(statusMessage, sizeof(statusMessage), "Mochi pats her round belly. Fully packed with salmon!");
        spawnPopText(cat.posX, 135, L"FULL!", RGB(235, 140, 60));
    } else {
        cat.fullness = (cat.fullness + 20 > 100) ? 100 : cat.fullness + 20;
        cat.happiness = (cat.happiness + 10 > 100) ? 100 : cat.happiness + 10;
        cat.weight += 0.15f;
        cat.mood = 4;
        cat.moodTimer = 60;
        spawnPopText(350, 210, L"🐟 YUM!", RGB(255, 120, 95));
        spawnPopText(cat.posX, 135, L"+CHONK!", RGB(245, 140, 50));
        snprintf(statusMessage, sizeof(statusMessage), "*Crunch crunch!* Mochi ate fresh salmon & shrimp! (+0.15 kg Chonk)");
        soundPurr();
    }
}

void playCat() {
    if (cat.energy < 20) {
        snprintf(statusMessage, sizeof(statusMessage), "Mochi is out of battery. She needs a warm catnap.");
        spawnPopText(cat.posX, 135, L"TIRED", RGB(150, 150, 150));
    } else {
        cat.energy -= 25;
        cat.fullness = (cat.fullness - 15 < 0) ? 0 : cat.fullness - 15;
        cat.happiness = (cat.happiness + 20 > 100) ? 100 : cat.happiness + 20;
        cat.mood = 1;
        cat.moodTimer = 50;
        yarnBounce = 20.0f;
        snprintf(statusMessage, sizeof(statusMessage), "Mochi batted the pink yarn ball and did a cute somersault!");
        spawnPopText(cat.posX, 125, L"POUNCE!", RGB(255, 95, 155));
        soundBoop();
    }
}

void boxCat() {
    cat.mood = 3;
    cat.moodTimer = 100;
    cat.happiness = (cat.happiness + 15 > 100) ? 100 : cat.happiness + 15;
    snprintf(statusMessage, sizeof(statusMessage), "If it fits, I sits! Her round fur spills over the box rim.");
    spawnPopText(cat.posX, 125, L"I SITS!", RGB(175, 125, 75));
    soundPurr();
}

void catnipCat() {
    cat.mood = 5;
    cat.moodTimer = 110;
    cat.energy = 100;
    cat.happiness = 100;
    snprintf(statusMessage, sizeof(statusMessage), "🌿 CATNIP PARTY! Mochi's emerald eyes went completely wide!");
    spawnPopText(cat.posX, 115, L"HYPER!!", RGB(90, 195, 115));
    soundZoom();
}

void zoomiesCat() {
    if (cat.energy < 30) {
        snprintf(statusMessage, sizeof(statusMessage), "Too chonky to zoom without a nap. Give catnip or sleep first!");
    } else {
        cat.mood = 2;
        cat.moodTimer = 90;
        cat.energy -= 30;
        cat.happiness = 100;
        snprintf(statusMessage, sizeof(statusMessage), "💨 CAT ZOOMIES! Mochi is galloping across the room!");
        spawnPopText(cat.posX, 115, L"ZOOM!!", RGB(235, 75, 75));
        soundZoom();
    }
}

void napCat() {
    cat.energy = (cat.energy + 40 > 100) ? 100 : cat.energy + 40;
    cat.fullness = (cat.fullness - 10 < 0) ? 0 : cat.fullness - 10;
    cat.mood = 4;
    cat.moodTimer = 80;
    snprintf(statusMessage, sizeof(statusMessage), "Mochi curled into a round loaf and drifted off to sleep.");
    spawnPopText(cat.posX, 125, L"zzz...", RGB(120, 180, 240));
    soundPurr();
}

void clickInteractiveScene(int mx, int my) {
    int cx = (int)cat.posX;
    int cy = 205;

    // 1. Click Standing Lamp (Toggle Dark Mode)
    if (mx >= 355 && mx <= 410 && my >= 95 && my <= 220) {
        toggleLamp();
        return;
    }

    // 2. Click Aquarium Stand & Goldfish
    if (mx >= 45 && mx <= 115 && my >= 160 && my <= 250) {
        spawnPopText(80, 160, L"SPLASH! 🐠", RGB(75, 175, 245));
        snprintf(statusMessage, sizeof(statusMessage), "🐠 The goldfish did a flip! Mochi's emerald eyes lit up.");
        soundBoop();
        return;
    }

    // 3. Click Pink Yarn Ball
    if (mx >= 115 && mx <= 155 && my >= 220 && my <= 260) {
        yarnBounce = 22.0f;
        spawnPopText(135, 220, L"BOING!", RGB(255, 105, 160));
        snprintf(statusMessage, sizeof(statusMessage), "🧶 You flicked the pink yarn ball across the floorboards!");
        soundBoop();
        cat.happiness = (cat.happiness + 5 > 100) ? 100 : cat.happiness + 5;
        return;
    }

    // 4. Click Ceramic Grey Food Bowl
    if (mx >= 320 && mx <= 365 && my >= 210 && my <= 255) {
        feedCat();
        return;
    }

    // 5. Click Nose / Boop
    if (mx >= cx - 12 && mx <= cx + 12 && my >= cy - 12 && my <= cy + 4) {
        spawnPopText(cx, cy - 25, L"BOOP! ♥", RGB(255, 130, 160));
        snprintf(statusMessage, sizeof(statusMessage), "👉 *BOOP!* You tapped her soft pink button nose!");
        soundBoop();
        cat.happiness = (cat.happiness + 5 > 100) ? 100 : cat.happiness + 5;
        return;
    }

    // 6. Click Chubby Cheeks
    if ((mx >= cx - 55 && mx <= cx - 20 && my >= cy - 20 && my <= cy + 15) ||
        (mx >= cx + 20 && mx <= cx + 55 && my >= cy - 20 && my <= cy + 15)) {
        spawnPopText(cx, cy - 35, L"SQUISH! ♥", RGB(255, 140, 170));
        snprintf(statusMessage, sizeof(statusMessage), "You squished her chubby cheeks. Maximum purring activated!");
        soundPurr();
        cat.happiness = (cat.happiness + 10 > 100) ? 100 : cat.happiness + 10;
        cat.mood = 4;
        cat.moodTimer = 50;
        return;
    }

    // 7. Click Warm Tummy
    if (mx >= cx - 40 && mx <= cx + 40 && my >= cy + 10 && my <= cy + 60) {
        if (rand() % 3 == 0) {
            snprintf(statusMessage, sizeof(statusMessage), "😼 TRAP! Mochi playfully bunny-kicked your hand!");
            spawnPopText(cx, cy, L"NIP!", RGB(230, 80, 80));
            soundNip();
        } else {
            snprintf(statusMessage, sizeof(statusMessage), "Mochi allows warm belly rubs on her soft tummy.");
            spawnPopText(cx, cy, L"PURRRR", RGB(255, 120, 160));
            soundPurr();
            cat.happiness = (cat.happiness + 15 > 100) ? 100 : cat.happiness + 15;
            cat.mood = 4;
            cat.moodTimer = 60;
        }
        return;
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        hFontTitle = CreateFontW(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        hFontBody = CreateFontW(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        hFontSmall = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        hFontPop = CreateFontW(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

        SetTimer(hwnd, TIMER_ANIM, 16, NULL);
        break;

    case WM_TIMER:
        if (wParam == TIMER_ANIM) {
            animTimer += 0.04f;
            fishSwim += 0.05f;
            bubbleTimer += 0.03f;

            if (yarnBounce > 0.0f) {
                yarnBounce -= 0.8f;
                if (yarnBounce < 0.0f) yarnBounce = 0.0f;
            }

            if (cat.mood == 2) {
                cat.posX += cat.dirX * 8.5f;
                if (cat.posX > 275.0f) cat.dirX = -1.0f;
                if (cat.posX < 135.0f) cat.dirX = 1.0f;
            } else {
                cat.posX += (215.0f - cat.posX) * 0.1f;
            }

            if (cat.moodTimer > 0) {
                cat.moodTimer--;
                if (cat.moodTimer == 0) cat.mood = 0;
            }

            for (int i = 0; i < MAX_PARTS; i++) {
                if (particles[i].life > 0) {
                    particles[i].x += particles[i].vx;
                    particles[i].y += particles[i].vy;
                    particles[i].life--;
                }
            }
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_LBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        if (mx >= btnFeed.left && mx <= btnFeed.right && my >= btnFeed.top && my <= btnFeed.bottom) {
            feedCat();
        } else if (mx >= btnPlay.left && mx <= btnPlay.right && my >= btnPlay.top && my <= btnPlay.bottom) {
            playCat();
        } else if (mx >= btnBox.left && mx <= btnBox.right && my >= btnBox.top && my <= btnBox.bottom) {
            boxCat();
        } else if (mx >= btnCatnip.left && mx <= btnCatnip.right && my >= btnCatnip.top && my <= btnCatnip.bottom) {
            catnipCat();
        } else if (mx >= btnZoomies.left && mx <= btnZoomies.right && my >= btnZoomies.top && my <= btnZoomies.bottom) {
            zoomiesCat();
        } else if (mx >= btnNap.left && mx <= btnNap.right && my >= btnNap.top && my <= btnNap.bottom) {
            napCat();
        } else {
            clickInteractiveScene(mx, my);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdcWin = BeginPaint(hwnd, &ps);

        RECT clientRc;
        GetClientRect(hwnd, &clientRc);
        int winW = clientRc.right - clientRc.left;
        int winH = clientRc.bottom - clientRc.top;

        HDC hdc = CreateCompatibleDC(hdcWin);
        HBITMAP hbm = CreateCompatibleBitmap(hdcWin, winW, winH);
        HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hbm);

        COLORREF windowBg = lampOn ? COLOR_WALL_DAY : COLOR_WALL_NIGHT;
        HBRUSH bgB = CreateSolidBrush(windowBg);
        FillRect(hdc, &clientRc, bgB);
        DeleteObject(bgB);

        RECT cardRc = { 25, 20, winW - 25, 350 };
        drawCozyRoom(hdc, cardRc.left, cardRc.top, cardRc.right, cardRc.bottom);

        HPEN borderPen = CreatePen(PS_SOLID, 2, lampOn ? COLOR_CARD_BORDER : RGB(65, 55, 75));
        SelectObject(hdc, borderPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        RoundRect(hdc, cardRc.left, cardRc.top, cardRc.right, cardRc.bottom, 16, 16);
        DeleteObject(borderPen);

        // Header Title: "Mochi the Chubby Cat"
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, lampOn ? COLOR_TEXT_MAIN : RGB(245, 235, 225));
        SelectObject(hdc, hFontTitle);
        TextOutW(hdc, 45, 32, L"Mochi the Chubby Cat 🐾", 23);

        SelectObject(hdc, hFontSmall);
        SetTextColor(hdc, lampOn ? COLOR_TEXT_MUTED : RGB(180, 165, 185));
        wchar_t weightStr[80];
        swprintf(weightStr, 80, L"Weight: %.2f kg  |  Click the lamp to toggle Dark Mode!", cat.weight);
        TextOutW(hdc, 45, 60, weightStr, wcslen(weightStr));

        drawTheChubbyCat(hdc, (int)cat.posX, 205);

        // HUD Card
        int mx = 415;
        RECT meterCard = { mx - 10, 95, mx + 225, 260 };
        drawRoundedBox(hdc, meterCard, lampOn ? RGB(255, 255, 255) : RGB(40, 32, 48), 
                                       lampOn ? COLOR_CARD_BORDER : RGB(75, 60, 85), 12);

        drawMeter(hdc, mx, 125, 205, 10, cat.happiness, COLOR_BAR_HEART, L"Happiness ♥");
        drawMeter(hdc, mx, 175, 205, 10, cat.fullness,  COLOR_BAR_FOOD,  L"Fullness 🐟");
        drawMeter(hdc, mx, 225, 205, 10, cat.energy,    COLOR_BAR_ENERGY, L"Energy ⚡");

        SelectObject(hdc, hFontPop);
        for (int i = 0; i < MAX_PARTS; i++) {
            if (particles[i].life > 0) {
                SetTextColor(hdc, particles[i].color);
                TextOutW(hdc, (int)particles[i].x, (int)particles[i].y, particles[i].text, wcslen(particles[i].text));
            }
        }

        // Status Bubble
        RECT bubbleRc = { 45, 295, winW - 45, 335 };
        drawRoundedBox(hdc, bubbleRc, lampOn ? RGB(255, 255, 255) : RGB(40, 32, 48), 
                                       lampOn ? COLOR_CARD_BORDER : RGB(75, 60, 85), 10);
        SetTextColor(hdc, lampOn ? COLOR_TEXT_MAIN : RGB(245, 235, 225));
        SelectObject(hdc, hFontSmall);

        wchar_t wMsg[256];
        mbstowcs(wMsg, statusMessage, 256);
        RECT textBubble = { bubbleRc.left + 14, bubbleRc.top, bubbleRc.right - 14, bubbleRc.bottom };
        DrawTextW(hdc, wMsg, -1, &textBubble, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // 6 Action Buttons
        drawActionButton(hdc, btnFeed,    L"🐟", L"Treat");
        drawActionButton(hdc, btnPlay,    L"🧶", L"Yarn");
        drawActionButton(hdc, btnBox,     L"📦", L"Box");
        drawActionButton(hdc, btnCatnip,  L"🌿", L"Catnip");
        drawActionButton(hdc, btnZoomies, L"💨", L"Zoomies");
        drawActionButton(hdc, btnNap,     L"💤", L"Nap");

        BitBlt(hdcWin, 0, 0, winW, winH, hdc, 0, 0, SRCCOPY);
        SelectObject(hdc, oldBmp);
        DeleteObject(hbm);
        DeleteDC(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ANIM);
        DeleteObject(hFontTitle);
        DeleteObject(hFontBody);
        DeleteObject(hFontSmall);
        DeleteObject(hFontPop);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    srand((unsigned int)time(NULL));

    const wchar_t CLASS_NAME[] = L"MochiTheChubbyCatWindow";
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_HAND);
    wc.hbrBackground = NULL;

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Mochi the Chubby Cat",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 685, 480,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {0};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}