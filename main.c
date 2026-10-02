#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>

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

// ---------------------------------------------------------------------------
// GDI object cache: pens/brushes are created once per distinct style/color and
// reused every frame (no per-frame allocation, and nothing is ever deleted
// while still selected into a DC). The macros below route the existing
// Create*/DeleteObject calls through the cache.
// ---------------------------------------------------------------------------
#define GDI_CACHE_MAX 256
typedef struct { int kind; int style; int width; COLORREF color; HGDIOBJ obj; } GdiEntry;
static GdiEntry gdiCache[GDI_CACHE_MAX];
static int gdiCount = 0;

static HGDIOBJ gdiLookup(int kind, int style, int width, COLORREF color) {
    for (int i = 0; i < gdiCount; i++) {
        GdiEntry* e = &gdiCache[i];
        if (e->kind == kind && e->style == style && e->width == width && e->color == color) return e->obj;
    }
    HGDIOBJ o = (kind == 0) ? (HGDIOBJ)CreateSolidBrush(color) : (HGDIOBJ)CreatePen(style, width, color);
    if (o && gdiCount < GDI_CACHE_MAX) {
        GdiEntry e = { kind, style, width, color, o };
        gdiCache[gdiCount++] = e;
    }
    return o;
}
static HBRUSH cachedBrush(COLORREF c) { return (HBRUSH)gdiLookup(0, 0, 0, c); }
static HPEN cachedPen(int style, int width, COLORREF c) { return (HPEN)gdiLookup(1, style, width, c); }
static BOOL cachedDelete(HGDIOBJ o) {
    for (int i = 0; i < gdiCount; i++) if (gdiCache[i].obj == o) return TRUE;
    return DeleteObject(o);
}
static void freeGdiCache(void) {
    for (int i = 0; i < gdiCount; i++) DeleteObject(gdiCache[i].obj);
    gdiCount = 0;
}
#define CreateSolidBrush(c) cachedBrush(c)
#define CreatePen(s, w, c)  cachedPen(s, w, c)
#define DeleteObject(o)     cachedDelete((HGDIOBJ)(o))

static COLORREF lerpColor(COLORREF a, COLORREF b, float t) {
    return RGB((int)(GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t),
               (int)(GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t),
               (int)(GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t));
}

static void fillVGradient(HDC hdc, RECT rc, COLORREF top, COLORREF bottom) {
    int h = rc.bottom - rc.top;
    if (h <= 0) return;
    HBRUSH dcBrush = (HBRUSH)GetStockObject(DC_BRUSH);
    for (int y = 0; y < h; y += 2) {
        RECT r = { rc.left, rc.top + y, rc.right, rc.top + y + 2 };
        if (r.bottom > rc.bottom) r.bottom = rc.bottom;
        SetDCBrushColor(hdc, lerpColor(top, bottom, (float)y / (float)h));
        FillRect(hdc, &r, dcBrush);
    }
}

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
    int mood; // 0: Normal, 1: Eating, 2: Zoomies, 3: In Box, 4: Purring, 5: Catnip Frenzy, 6: Sleeping
    int moodTimer;
    float posX;
    float dirX;
    int napLeft; // seconds of guaranteed sleep left
} ChubbyCat;

ChubbyCat cat = {"Mochi", 88, 70, 85, 6.2f, 0, 0, 215.0f, 1.0f, 0};
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

bool soundMuted = false;
int hoverBtn = -1;
bool hoverMute = false;
bool mouseTracking = false;
int flashBtn = -1;
int flashTimer = 0;
int tickCount = 0;
int alertCooldown = 0;
ULONGLONG lastNeedsTick = 0;

void feedCat(void);
void playCat(void);
void boxCat(void);
void catnipCat(void);
void zoomiesCat(void);
void napCat(void);

typedef struct {
    RECT rc;
    const wchar_t* icon;
    const wchar_t* title;
    const wchar_t* key;
    void (*action)(void);
} ActionBtn;

#define NUM_BTNS 6
ActionBtn buttons[NUM_BTNS] = {
    {{  40, 375, 130, 415 }, L"🐟", L"Treat",   L"[1]", feedCat},
    {{ 140, 375, 230, 415 }, L"🧶", L"Yarn",    L"[2]", playCat},
    {{ 240, 375, 330, 415 }, L"📦", L"Box",     L"[3]", boxCat},
    {{ 340, 375, 430, 415 }, L"🌿", L"Catnip",  L"[4]", catnipCat},
    {{ 440, 375, 530, 415 }, L"💨", L"Zoomies", L"[5]", zoomiesCat},
    {{ 540, 375, 630, 415 }, L"💤", L"Nap",     L"[6]", napCat},
};
RECT btnMute = { 605, 30, 637, 54 };

// Yarn and Zoomies look dimmed when Mochi is too tired (still clickable for the message)
bool btnEnabled(int i) {
    if (i == 1) return cat.energy >= 20;
    if (i == 4) return cat.energy >= 30;
    return true;
}

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
    SetTextColor(hdc, lampOn ? COLOR_TEXT_MAIN : RGB(245, 235, 225));
    SelectObject(hdc, hFontSmall);
    TextOutW(hdc, x, y - 18, label, wcslen(label));

    wchar_t valBuf[16];
    swprintf(valBuf, 16, L"%d%%", value);
    TextOutW(hdc, x + w - 32, y - 18, valBuf, wcslen(valBuf));

    RECT bgRc = { x, y, x + w, y + h };
    drawRoundedBox(hdc, bgRc, lampOn ? COLOR_BAR_BG : RGB(62, 52, 76),
                              lampOn ? COLOR_CARD_BORDER : RGB(80, 66, 94), 6);

    int fillW = (w * value) / 100;
    if (fillW > 0) {
        COLORREF col = (value < 25) ? RGB(232, 84, 84) : fillCol;
        RECT fillRc = { x, y, x + (fillW < 8 ? 8 : fillW), y + h };
        drawRoundedBox(hdc, fillRc, col, col, 6);
    }
}

void drawCozyRoom(HDC hdc, int left, int top, int right, int bottom) {
    int floorY = top + 155;

    COLORREF colWall = lampOn ? COLOR_WALL_DAY : COLOR_WALL_NIGHT;
    COLORREF colFloor = lampOn ? COLOR_FLOOR_DAY : COLOR_FLOOR_NIGHT;
    COLORREF colPlank = lampOn ? COLOR_FLOOR_LINE_DAY : COLOR_FLOOR_LINE_NIGHT;
    COLORREF colRug = lampOn ? COLOR_RUG_DAY : COLOR_RUG_NIGHT;
    COLORREF colRugRing = lampOn ? COLOR_RUG_RING_DAY : COLOR_RUG_RING_NIGHT;

    // 1. Room Wall
    RECT wallRc = { left, top, right, floorY };
    fillVGradient(hdc, wallRc, lampOn ? RGB(255, 250, 243) : RGB(38, 32, 54),
                               lampOn ? RGB(246, 232, 216) : colWall);

    // 2. Window (Sun in Day / Moon & Stars at Night)
    RECT winRc = { left + 35, top + 18, left + 135, top + 115 };
    fillVGradient(hdc, winRc, lampOn ? RGB(150, 208, 248) : RGB(12, 14, 28),
                              lampOn ? RGB(214, 238, 252) : RGB(30, 32, 60));

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
        HBRUSH skyShadow = CreateSolidBrush(lerpColor(RGB(12, 14, 28), RGB(30, 32, 60), 0.22f));
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

    // Curtains and rod
    HBRUSH curB = CreateSolidBrush(lampOn ? RGB(238, 160, 160) : RGB(92, 62, 96));
    SelectObject(hdc, curB);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    POINT curL[4] = { { winRc.left - 16, winRc.top - 6 }, { winRc.left + 12, winRc.top - 6 },
                      { winRc.left + 6, winRc.bottom + 6 }, { winRc.left - 20, winRc.bottom + 6 } };
    POINT curR[4] = { { winRc.right - 12, winRc.top - 6 }, { winRc.right + 16, winRc.top - 6 },
                      { winRc.right + 20, winRc.bottom + 6 }, { winRc.right - 6, winRc.bottom + 6 } };
    Polygon(hdc, curL, 4);
    Polygon(hdc, curR, 4);
    DeleteObject(curB);
    HPEN rodP = CreatePen(PS_SOLID, 3, lampOn ? RGB(176, 130, 96) : RGB(70, 54, 60));
    SelectObject(hdc, rodP);
    MoveToEx(hdc, winRc.left - 24, winRc.top - 8, NULL);
    LineTo(hdc, winRc.right + 24, winRc.top - 8);
    DeleteObject(rodP);

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
    int wag = (int)(sinf(fishSwim * 9.0f) * 3.0f);

    SelectObject(hdc, GetStockObject(NULL_PEN));
    HBRUSH finB = CreateSolidBrush(RGB(255, 150, 80));
    SelectObject(hdc, finB);
    POINT tailPts[4] = {
        { fishX - 6 * fishDir,  fishY },
        { fishX - 14 * fishDir, fishY - 6 + wag },
        { fishX - 10 * fishDir, fishY + wag / 2 },
        { fishX - 14 * fishDir, fishY + 6 + wag }
    };
    Polygon(hdc, tailPts, 4);
    POINT dorsal[3] = { { fishX - 3 * fishDir, fishY - 3 }, { fishX + fishDir, fishY - 8 }, { fishX + 5 * fishDir, fishY - 3 } };
    Polygon(hdc, dorsal, 3);
    DeleteObject(finB);

    HBRUSH fishB = CreateSolidBrush(COLOR_FISH_GOLD);
    SelectObject(hdc, fishB);
    Ellipse(hdc, fishX - 8, fishY - 4, fishX + 8, fishY + 4);
    DeleteObject(fishB);
    HBRUSH bellyB = CreateSolidBrush(RGB(255, 196, 130));
    SelectObject(hdc, bellyB);
    Ellipse(hdc, fishX - 5, fishY + 1, fishX + 5, fishY + 4);
    DeleteObject(bellyB);
    HBRUSH eyeW = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, eyeW);
    Ellipse(hdc, fishX + 4 * fishDir - 2, fishY - 3, fishX + 4 * fishDir + 2, fishY + 1);
    DeleteObject(eyeW);
    HBRUSH eyeP = CreateSolidBrush(RGB(20, 16, 26));
    SelectObject(hdc, eyeP);
    Ellipse(hdc, fishX + 5 * fishDir - 1, fishY - 2, fishX + 5 * fishDir + 1, fishY);
    DeleteObject(eyeP);

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
    bool blinking = fmodf(animTimer, 5.3f) < 0.2f;
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

    if (cat.happiness >= 80 || cat.mood == 4) {
        HBRUSH blushB = CreateSolidBrush(RGB(255, 150, 140));
        SelectObject(hdc, blushB);
        Ellipse(hdc, cx - 52, cy - 8, cx - 36, cy + 2);
        Ellipse(hdc, cx + 36, cy - 8, cx + 52, cy + 2);
        DeleteObject(blushB);
    }

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
    } else if (cat.mood == 4 || cat.mood == 6 || blinking) {
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

void drawActionButton(HDC hdc, const ActionBtn* b, bool hover, bool pressed, bool enabled) {
    COLORREF bg, border, txt;
    if (!enabled) {
        bg     = lampOn ? RGB(240, 234, 228) : RGB(44, 36, 54);
        border = lampOn ? RGB(222, 212, 202) : RGB(70, 58, 82);
        txt    = lampOn ? RGB(176, 160, 150) : RGB(120, 108, 130);
    } else if (hover) {
        bg     = lampOn ? RGB(255, 230, 208) : RGB(78, 62, 92);
        border = lampOn ? RGB(240, 160, 110) : RGB(140, 110, 160);
        txt    = lampOn ? COLOR_TEXT_MAIN : RGB(245, 235, 225);
    } else {
        bg     = lampOn ? COLOR_BTN_BG : RGB(58, 46, 70);
        border = lampOn ? COLOR_BTN_BORDER : RGB(95, 78, 110);
        txt    = lampOn ? COLOR_TEXT_MAIN : RGB(245, 235, 225);
    }

    RECT rc = b->rc;
    if (pressed) {
        OffsetRect(&rc, 0, 2);
    } else {
        COLORREF shade = lampOn ? RGB(238, 216, 196) : RGB(26, 20, 34);
        RECT sh = b->rc;
        OffsetRect(&sh, 0, 2);
        drawRoundedBox(hdc, sh, shade, shade, 8);
    }
    drawRoundedBox(hdc, rc, bg, border, 8);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, txt);
    SelectObject(hdc, hFontBody);
    wchar_t buf[32];
    swprintf(buf, 32, L"%ls %ls", b->icon, b->title);
    RECT tr = rc;
    DrawTextW(hdc, buf, -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, lampOn ? COLOR_TEXT_MUTED : RGB(180, 165, 185));
    RECT hr = { b->rc.left, b->rc.bottom + 4, b->rc.right, b->rc.bottom + 20 };
    DrawTextW(hdc, b->key, -1, &hr, DT_CENTER | DT_SINGLELINE);
}

// Beep() blocks until the tone ends, which froze the animation. Tones now play
// on a short-lived worker thread; a new sound is skipped while one is playing.
typedef struct { int n; int freq[4]; int dur[4]; } SoundSeq;
static volatile LONG soundBusy = 0;

static DWORD WINAPI soundThread(LPVOID p) {
    SoundSeq* s = (SoundSeq*)p;
    for (int i = 0; i < s->n; i++) Beep((DWORD)s->freq[i], (DWORD)s->dur[i]);
    free(s);
    InterlockedExchange(&soundBusy, 0);
    return 0;
}

static void playTones(int n, const int* freq, const int* dur) {
    if (soundMuted || n <= 0) return;
    if (InterlockedCompareExchange(&soundBusy, 1, 0) != 0) return;
    SoundSeq* s = (SoundSeq*)malloc(sizeof(SoundSeq));
    if (!s) { InterlockedExchange(&soundBusy, 0); return; }
    s->n = (n > 4) ? 4 : n;
    for (int i = 0; i < s->n; i++) { s->freq[i] = freq[i]; s->dur[i] = dur[i]; }
    HANDLE h = CreateThread(NULL, 0, soundThread, s, 0, NULL);
    if (h) CloseHandle(h);
    else { free(s); InterlockedExchange(&soundBusy, 0); }
}

void soundBoop()   { int f[] = { 880 };            int d[] = { 80 };           playTones(1, f, d); }
void soundPurr()   { int f[] = { 220, 240 };       int d[] = { 100, 100 };     playTones(2, f, d); }
void soundZoom()   { int f[] = { 600, 800, 1100 }; int d[] = { 50, 50, 70 };   playTones(3, f, d); }
void soundNip()    { int f[] = { 300 };            int d[] = { 120 };          playTones(1, f, d); }
void soundSwitch() { int f[] = { 1200, 900 };      int d[] = { 40, 50 };       playTones(2, f, d); }

void triggerBtn(int i) {
    buttons[i].action();
    flashBtn = i;
    flashTimer = 8;
}

void toggleMute(void) {
    soundMuted = !soundMuted;
    snprintf(statusMessage, sizeof(statusMessage), "%s",
             soundMuted ? "Sound off. Mochi purrs silently." : "Sound on. Mochi's purrs are back!");
    if (!soundMuted) soundBoop();
}

void wakeCat(const char* msg) {
    cat.mood = 0;
    cat.moodTimer = 0;
    cat.napLeft = 0;
    snprintf(statusMessage, sizeof(statusMessage), "%s", msg);
    spawnPopText(cat.posX, 125, L"!", RGB(245, 170, 60));
}

const wchar_t* chonkTier(float w) {
    if (w < 6.5f)  return L"Fluffy Loaf";
    if (w < 7.5f)  return L"Chonky";
    if (w < 9.0f)  return L"Extra Chonk";
    if (w < 11.0f) return L"Absolute Unit";
    return L"Mega Chonk";
}

void saveGame(void);

// Called once per second: needs slowly drop, Mochi naps when exhausted.
void tickNeeds(void) {
    tickCount++;

    if (cat.mood == 6) {
        cat.energy = (cat.energy + 4 > 100) ? 100 : cat.energy + 4;
        if (cat.napLeft > 0) cat.napLeft--;
        spawnPopText(cat.posX + 45, 135, (tickCount & 1) ? L"z" : L"Z", RGB(120, 180, 240));
        if (cat.energy >= 100 && cat.napLeft <= 0) {
            wakeCat("Mochi woke up from her nap, fully recharged!");
        }
    } else {
        if (tickCount % 3 == 0 && cat.fullness > 0) cat.fullness--;
        if (tickCount % 5 == 0 && cat.energy > 0)   cat.energy--;
        if (tickCount % 4 == 0 && cat.fullness < 40) {
            cat.happiness -= (cat.fullness < 15) ? 2 : 1;
            if (cat.happiness < 0) cat.happiness = 0;
        }

        if (cat.mood == 0) {
            if (cat.energy <= 10) {
                cat.mood = 6;
                cat.moodTimer = 0;
                cat.napLeft = 5;
                snprintf(statusMessage, sizeof(statusMessage), "Mochi ran out of energy and dozed off on the rug. Click her to wake her up.");
            } else if (cat.fullness < 25 && alertCooldown <= 0) {
                snprintf(statusMessage, sizeof(statusMessage), "Mochi's tummy is rumbling... maybe a salmon treat?");
                spawnPopText(cat.posX, 125, L"MEOW!", RGB(235, 140, 60));
                soundBoop();
                alertCooldown = 20;
            }
        }
    }

    if (alertCooldown > 0) alertCooldown--;
    if (tickCount % 30 == 0) saveGame();
}

// --- Save / load (stats persist between sessions) -------------------------
#define SAVE_MAGIC 0x4D4F4348u
typedef struct {
    unsigned magic;
    int happiness, fullness, energy;
    float weight;
    int lampOn, muted;
    long long savedAt;
} SaveData;

static bool savePath(wchar_t* out, size_t n) {
    wchar_t base[MAX_PATH];
    DWORD len = GetEnvironmentVariableW(L"LOCALAPPDATA", base, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return false;
    swprintf(out, n, L"%ls\\MochiCat.sav", base);
    return true;
}

void saveGame(void) {
    wchar_t path[MAX_PATH + 32];
    if (!savePath(path, MAX_PATH + 32)) return;
    FILE* f = _wfopen(path, L"wb");
    if (!f) return;
    SaveData d = { SAVE_MAGIC, cat.happiness, cat.fullness, cat.energy, cat.weight,
                   lampOn ? 1 : 0, soundMuted ? 1 : 0, (long long)time(NULL) };
    fwrite(&d, sizeof(d), 1, f);
    fclose(f);
}

static int decayTo(int v, int amount, int floorVal) {
    int r = v - amount;
    if (r < floorVal) r = (v < floorVal) ? v : floorVal;
    return r;
}

void loadGame(void) {
    wchar_t path[MAX_PATH + 32];
    if (!savePath(path, MAX_PATH + 32)) return;
    FILE* f = _wfopen(path, L"rb");
    if (!f) return;
    SaveData d;
    size_t got = fread(&d, sizeof(d), 1, f);
    fclose(f);
    if (got != 1 || d.magic != SAVE_MAGIC) return;
    if (d.happiness < 0 || d.happiness > 100 || d.fullness < 0 || d.fullness > 100 ||
        d.energy < 0 || d.energy > 100 || !(d.weight >= 3.0f && d.weight <= 40.0f)) return;

    cat.happiness = d.happiness;
    cat.fullness = d.fullness;
    cat.energy = d.energy;
    cat.weight = d.weight;
    lampOn = d.lampOn != 0;
    soundMuted = d.muted != 0;

    // Gentle catch-up for time spent away (never drops to zero)
    long long away = (long long)time(NULL) - d.savedAt;
    if (away >= 300) {
        int mins = (int)((away / 60 > 600) ? 600 : away / 60);
        cat.fullness  = decayTo(cat.fullness, mins / 3, 15);
        cat.happiness = decayTo(cat.happiness, mins / 6, 20);
        cat.energy    = (cat.energy + mins / 2 > 100) ? 100 : cat.energy + mins / 2;
        snprintf(statusMessage, sizeof(statusMessage), "Welcome back! Mochi dozed while you were away and missed you.");
    }
}

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
        cat.weight = fmaxf(4.0f, cat.weight - 0.03f);
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
        cat.weight = fmaxf(4.0f, cat.weight - 0.05f);
        cat.happiness = 100;
        snprintf(statusMessage, sizeof(statusMessage), "💨 CAT ZOOMIES! Mochi is galloping across the room!");
        spawnPopText(cat.posX, 115, L"ZOOM!!", RGB(235, 75, 75));
        soundZoom();
    }
}

void napCat() {
    cat.mood = 6;
    cat.moodTimer = 0;
    cat.napLeft = 5;
    cat.fullness = (cat.fullness - 5 < 0) ? 0 : cat.fullness - 5;
    snprintf(statusMessage, sizeof(statusMessage), "Mochi curled into a round loaf and drifted off to sleep. Click her to wake her up.");
    spawnPopText(cat.posX, 125, L"zzz...", RGB(120, 180, 240));
    soundPurr();
}

void clickInteractiveScene(int mx, int my) {
    int cx = (int)cat.posX;
    int cy = 205;

    // Waking a sleeping Mochi
    if (cat.mood == 6 && mx >= cx - 85 && mx <= cx + 85 && my >= cy - 72 && my <= cy + 72) {
        wakeCat("Mochi stretched, yawned, and blinked up at you.");
        return;
    }

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

// True when the cursor is over something clickable (used for the hand cursor)
bool isInteractive(int mx, int my) {
    POINT pt = { mx, my };
    for (int i = 0; i < NUM_BTNS; i++) if (PtInRect(&buttons[i].rc, pt)) return true;
    if (PtInRect(&btnMute, pt)) return true;
    int cx = (int)cat.posX;
    if (mx >= 355 && mx <= 410 && my >= 95 && my <= 220) return true;   // lamp
    if (mx >= 45 && mx <= 115 && my >= 160 && my <= 250) return true;   // aquarium
    if (mx >= 115 && mx <= 155 && my >= 220 && my <= 260) return true;  // yarn
    if (mx >= 320 && mx <= 365 && my >= 210 && my <= 255) return true;  // bowl
    if (mx >= cx - 70 && mx <= cx + 70 && my >= 140 && my <= 270) return true; // Mochi
    return false;
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
        loadGame();
        break;

    case WM_TIMER:
        if (wParam == TIMER_ANIM) {
            animTimer += 0.04f;
            fishSwim += 0.05f;
            bubbleTimer += 0.03f;

            if (flashTimer > 0) flashTimer--;
            {
                ULONGLONG now = GetTickCount64();
                if (now - lastNeedsTick >= 1000) {
                    lastNeedsTick = now;
                    tickNeeds();
                }
            }

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

    case WM_MOUSEMOVE: {
        POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
        hoverBtn = -1;
        for (int i = 0; i < NUM_BTNS; i++) {
            if (PtInRect(&buttons[i].rc, pt)) { hoverBtn = i; break; }
        }
        hoverMute = PtInRect(&btnMute, pt) ? true : false;
        if (!mouseTracking) {
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme);
            mouseTracking = true;
        }
        break;
    }

    case WM_MOUSELEAVE:
        hoverBtn = -1;
        hoverMute = false;
        mouseTracking = false;
        break;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);
            SetCursor(LoadCursor(NULL, isInteractive(pt.x, pt.y) ? IDC_HAND : IDC_ARROW));
            return TRUE;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);

    case WM_KEYDOWN:
        if (wParam >= '1' && wParam <= '6') triggerBtn((int)(wParam - '1'));
        else if (wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD6) triggerBtn((int)(wParam - VK_NUMPAD1));
        else if (wParam == 'L') toggleLamp();
        else if (wParam == 'M') toggleMute();
        break;

    case WM_LBUTTONDOWN: {
        POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
        int hit = -1;
        for (int i = 0; i < NUM_BTNS; i++) {
            if (PtInRect(&buttons[i].rc, pt)) { hit = i; break; }
        }
        if (hit >= 0) triggerBtn(hit);
        else if (PtInRect(&btnMute, pt)) toggleMute();
        else clickInteractiveScene(pt.x, pt.y);
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
        wchar_t weightStr[128];
        swprintf(weightStr, 128, L"%ls  •  %.2f kg  |  Click the lamp for day/night  •  Keys 1-6  •  M = mute",
                 chonkTier(cat.weight), cat.weight);
        TextOutW(hdc, 45, 60, weightStr, wcslen(weightStr));

        int hop = 0;
        if (cat.mood == 1 || cat.mood == 5) hop = -(int)(fabsf(sinf(animTimer * 9.0f)) * 9.0f);
        drawTheChubbyCat(hdc, (int)cat.posX, 205 + hop);

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
        MultiByteToWideChar(CP_UTF8, 0, statusMessage, -1, wMsg, 256);
        RECT textBubble = { bubbleRc.left + 14, bubbleRc.top, bubbleRc.right - 14, bubbleRc.bottom };
        DrawTextW(hdc, wMsg, -1, &textBubble, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Action buttons
        for (int i = 0; i < NUM_BTNS; i++) {
            drawActionButton(hdc, &buttons[i], hoverBtn == i, flashBtn == i && flashTimer > 0, btnEnabled(i));
        }

        // Mute toggle
        drawRoundedBox(hdc, btnMute,
                       hoverMute ? (lampOn ? RGB(255, 230, 208) : RGB(78, 62, 92)) : (lampOn ? COLOR_BTN_BG : RGB(58, 46, 70)),
                       lampOn ? COLOR_BTN_BORDER : RGB(95, 78, 110), 8);
        SetTextColor(hdc, lampOn ? COLOR_TEXT_MAIN : RGB(245, 235, 225));
        SelectObject(hdc, hFontBody);
        RECT muteTr = btnMute;
        DrawTextW(hdc, soundMuted ? L"\U0001F507" : L"\U0001F50A", -1, &muteTr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        BitBlt(hdcWin, 0, 0, winW, winH, hdc, 0, 0, SRCCOPY);
        SelectObject(hdc, oldBmp);
        DeleteObject(hbm);
        DeleteDC(hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        saveGame();
        KillTimer(hwnd, TIMER_ANIM);
        DeleteObject(hFontTitle);
        DeleteObject(hFontBody);
        DeleteObject(hFontSmall);
        DeleteObject(hFontPop);
        freeGdiCache();
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
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;

    RegisterClassW(&wc);

    // Size the window so the *client* area is exactly 670x445
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT wr = { 0, 0, 670, 445 };
    AdjustWindowRect(&wr, style, FALSE);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"Mochi the Chubby Cat",
        style,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
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