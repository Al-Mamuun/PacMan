#include <GL/glut.h>
#include <windows.h>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;


struct BeepReq { int freq, ms; };

static DWORD WINAPI beepThread(LPVOID arg) {
    BeepReq* r = (BeepReq*)arg;
    Beep(r->freq, r->ms);
    delete r;
    return 0;
}

static void playBeep(int freq, int ms) {
    BeepReq* r = new BeepReq{freq, ms};
    HANDLE h = CreateThread(NULL, 0, beepThread, r, 0, NULL);
    if (h) CloseHandle(h);
}

static void sndDot()      { playBeep(600,  18); }
static void sndPellet()   { playBeep(950,  55); }
static void sndDie()      { playBeep(200, 300); }
static void sndEatGhost() { playBeep(1300, 90); }
static void sndPerk()     { playBeep(1500, 75); }
static void sndLevelUp()  { playBeep(880, 180); }


#define SCR_W  520
#define SCR_H  620
#define ROWS    20
#define COLS    20
#define CELL    26
#define HUD_H   60
#define MAZE_H  (ROWS*CELL)
#define PI_F    3.14159265f


static int BASE[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,3,2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,2,3,1},
    {1,2,1,1,2,2,1,2,1,1,1,1,2,1,2,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,1,1,1,1,2,1,1,1,2,1,1,1,2,1},
    {1,2,2,4,2,2,2,2,2,1,2,2,2,2,2,2,2,5,2,1},
    {1,1,1,1,1,1,1,2,2,1,2,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,2,2,1},
    {1,2,1,1,1,2,1,1,1,1,1,1,2,1,2,1,1,1,2,1},
    {1,2,2,2,1,2,2,2,2,2,2,1,2,2,2,2,1,2,2,1},
    {1,1,1,2,1,1,1,1,1,6,2,1,1,1,1,2,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,1,2,2,2,2,2,1,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,2,1,1,1,2,1},
    {1,2,2,2,2,1,2,2,2,2,2,1,2,2,2,1,2,2,2,1},
    {1,1,1,1,2,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,1},
    {1,3,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};
static int maze[ROWS][COLS];
static int totalDots = 0;


static bool isWall(int c, int r) {
    if (c < 0 || c >= COLS || r < 0 || r >= ROWS) return true;
    return maze[r][c] == 1;
}

static float cellCx(int c) { return c * CELL + CELL / 2.0f; }
static float cellCy(int r) { return r * CELL + CELL / 2.0f; }
static int   tileX(float x) { return (int)(x / CELL); }
static int   tileY(float y) { return (int)(y / CELL); }


enum GState { MENU, PLAYING, PAUSED, GAMEOVER, WIN, HIGHSCORE, HELP };
static GState gs = MENU;
static int   score, lives, level;
static float elapsed;
static int   lastMs;
static bool  hasResume = false;
static int   menuSel   = 0;

struct HS { int sc, lv; };
static HS  hs[5];
static int hsN = 0;

static float bannerT = 0;
static char  bannerTxt[48];
static float flashT = 0;


static bool  perkInvisible = false;
static float perkInvLeft   = 0;
static bool  perkFreeze    = false;
static float perkFrzLeft   = 0;
static bool  perkSpeed     = false;
static float perkSpdLeft   = 0;


static float pX, pY;
static int   pDX, pDY, nDX, nDY;
static float mouth    = 5;
static float mouthSpd = 3;
static bool  powered  = false;
static float powerLeft = 0;
#define PAC_SPD 110.0f


enum GhostID { BLINKY, PINKY, INKY, CLYDE };

struct Ghost {
    float   x, y, r, g, b;
    int     dx, dy;
    float   tgtX, tgtY;
    float   speed;
    float   frightLeft;
    float   fireCD;
    float   clX, clY, clLife;
    GhostID id;
    bool    fright;
    bool    cloneOn;
};
static Ghost G[4];


struct Fire { float x, y, vx, vy, life; };
struct Part { float x, y, vx, vy, life, r, g, b; };
static vector<Fire> fires;
static vector<Part> parts;




static void drawGhostAt(float gx, float gy,
                         float r, float g, float b,
                         bool fright, float fLeft, float alpha) {
    float rad = 10.0f;
    float dr = r, dg = g, db = b;

    if (fright) {
        float fl = (fLeft < 2) ? (0.5f + 0.5f * sinf(elapsed * 10)) : 0.f;
        dr = 0.1f + fl * 0.4f;
        dg = 0.1f;
        db = 0.85f;
    }
    if (perkFreeze && !fright) {
        dr *= 0.6f;
        dg *= 0.6f;
        db  = db * 0.6f + 0.4f;
    }

    blendOn();
    glColor4f(dr, dg, db, alpha);


    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(gx, gy);
    for (int i = 0; i <= 24; i++) {
        float a = PI_F + PI_F * i / 24.0f;
        glVertex2f(gx + cosf(a) * rad, gy + sinf(a) * rad);
    }
    glEnd();


    fillR(gx - rad, gy, rad * 2, rad);
    for (int i = 0; i < 3; i++)
        fillC(gx - rad + (i*2+1) * rad*2/6.0f, gy + rad, rad / 3.0f);


    if (!fright) {
        glColor4f(1, 1, 1, alpha);
        fillC(gx - 3.5f, gy - 1, 3.5f);
        fillC(gx + 3.5f, gy - 1, 3.5f);
        glColor4f(0.1f, 0.1f, 0.9f, alpha);
        fillC(gx - 3.5f, gy - 1, 1.8f);
        fillC(gx + 3.5f, gy - 1, 1.8f);
    }
    blendOff();
}


static void drawPac() {
    float rot   = atan2f((float)pDY, (float)pDX);
    float m     = mouth * PI_F / 180.0f;
    float alpha = perkInvisible ? 0.35f : 1.0f;

    if (perkInvisible) {
        blendOn();
        glColor4f(0, 1, 1, 0.08f + 0.07f * sinf(elapsed * 10));
        fillC(pX, pY, CELL * 0.9f);
        blendOff();
    }
    if (powered) {
        blendOn();
        glColor4f(1, 1, 0, 0.15f + 0.1f * sinf(elapsed * 8));
        fillC(pX, pY, CELL * 0.7f);
        blendOff();
    }
    if (perkSpeed) {
        blendOn();
        glColor4f(1, 0.8f, 0, 0.12f + 0.08f * sinf(elapsed * 12));
        fillC(pX, pY, CELL * 0.6f);
        blendOff();
    }

    blendOn();
    glColor4f(1, 0.85f, 0, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(pX, pY);
    for (int i = 0; i <= 48; i++) {
        float a = m + (2 * PI_F - 2 * m) * i / 48.0f;
        glVertex2f(pX + cosf(a + rot) * 11, pY + sinf(a + rot) * 11);
    }
    glEnd();

    glColor4f(0, 0, 0, alpha);
    fillC(pX + cosf(rot - 0.8f) * 6, pY + sinf(rot - 0.8f) * 6, 1.8f);
    blendOff();
}




static void drawHUD() {
    float hy = MAZE_H;
    glColor3f(0.05f, 0.05f, 0.05f);
    fillR(0, hy, SCR_W, HUD_H);
    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_LINES);
    glVertex2f(0, hy);
    glVertex2f(SCR_W, hy);
    glEnd();

    char buf[32];

    glColor3f(1, 0.84f, 0);
    drawStrL(8, hy + 20, "SCORE");
    glColor3f(1, 1, 1);
    sprintf(buf, "%d", score);
    drawStrL(8, hy + 44, buf);

    glColor3f(0.5f, 0.8f, 1);
    drawStrL(140, hy + 20, "LEVEL");
    glColor3f(1, 1, 1);
    sprintf(buf, "%d", level);
    drawStrL(140, hy + 44, buf);

    glColor3f(0.6f, 1, 0.6f);
    drawStrL(240, hy + 20, "TIME");
    glColor3f(1, 1, 1);
    sprintf(buf, "%.0fs", elapsed);
    drawStrL(240, hy + 44, buf);

    glColor3f(0.8f, 0.8f, 0.8f);
    drawStr(358, hy + 22, "LIVES");
    for (int i = 0; i < lives; i++) {
        glColor3f(1, 0.85f, 0);
        float lx = 360 + i * 22.0f;
        float ly = hy + 40;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(lx, ly);
        for (int s = 20; s <= 340; s++) {
            float a = s * PI_F / 180;
            glVertex2f(lx + cosf(a) * 8, ly + sinf(a) * 8);
        }
        glEnd();
    }

    if (powered) {
        float frac = powerLeft / 7.0f;
        if (frac > 1) frac = 1;
        glColor3f(0.2f, 0.2f, 0.2f);
        fillR(8, hy + 54, 100, 6);
        glColor3f(1 - frac, frac, 0.1f);
        fillR(8, hy + 54, 100 * frac, 6);
        glColor3f(1, 0.55f, 0.05f);
        drawStr(112, hy + 62, "PWR");
    }

    float px2 = 175;
    if (perkInvisible) {
        glColor3f(0, 1, 1);
        sprintf(buf, "INV %.1fs", perkInvLeft);
        drawStr(px2, hy + 62, buf);
        px2 += 85;
    }
    if (perkFreeze) {
        glColor3f(0.5f, 0.7f, 1);
        sprintf(buf, "FRZ %.1fs", perkFrzLeft);
        drawStr(px2, hy + 62, buf);
        px2 += 85;
    }
    if (perkSpeed) {
        glColor3f(1, 0.9f, 0);
        sprintf(buf, "SPD %.1fs", perkSpdLeft);
        drawStr(px2, hy + 62, buf);
    }
}


static void drawMenu() {
    glColor3f(0, 0, 0);
    fillR(0, 0, SCR_W, SCR_H);

    blendOn();
    glColor4f(1, 0.84f, 0, 0.08f + 0.06f * sinf(elapsed * 2));
    fillC(SCR_W / 2.0f, 90, 80 + 10 * sinf(elapsed * 2));
    blendOff();

    glColor3f(1, 0.84f, 0);
    drawStrXL(SCR_W / 2.0f - 85, 90, "PAC-MAN");
    glColor3f(0.4f, 0.7f, 1);
    drawStr(SCR_W / 2.0f - 140, 112, "CSE 426 - COMPUTER GRAPHICS LAB");

    float gc[4][3] = {{1,0,0}, {1,0.6f,0}, {0,1,1}, {1,0,0.4f}};
    for (int i = 0; i < 4; i++)
        drawGhostAt(SCR_W/2.0f - 60 + i*35, 150,
                    gc[i][0], gc[i][1], gc[i][2], false, 0, 0.85f);

    const char* items[] = {"START GAME", "HIGH SCORES", "HELP", "EXIT"};
    for (int i = 0; i < 4; i++) {
        float bx  = SCR_W / 2.0f - 110;
        float by  = 240 + i * 55;
        bool  sel = (menuSel == i);
        if (sel) {
            glColor3f(1, 0.84f, 0);
            roundBox(bx, by, 220, 40, 10);
            glColor3f(0, 0, 0);
        } else {
            glColor3f(0.2f, 0.2f, 0.2f);
            roundBox(bx, by, 220, 40, 10);
            glColor3f(1, 1, 1);
        }
        drawStrL(bx + 55, by + 25, items[i]);
    }

    glColor3f(0.5f, 0.5f, 0.5f);
    drawStr(SCR_W/2.0f - 130, SCR_H - 32, "Arrow Keys + ENTER  |  Mouse Click");
    glColor3f(0.3f, 0.6f, 1.0f);
    drawStr(SCR_W/2.0f - 130, SCR_H - 12, "(c) 2025  Mamun & Nabila  |  CSE 426");
}


static void drawHighScores() {
    glColor3f(0, 0, 0);
    fillR(0, 0, SCR_W, SCR_H);
    glColor3f(1, 0.84f, 0);
    drawStrXL(SCR_W/2.0f - 90, 70, "HIGH SCORES");
    glColor3f(0.3f, 0.3f, 0.5f);
    fillR(30, 85, SCR_W - 60, 2);

    for (int i = 0; i < hsN; i++) {
        char buf[80];
        if      (i == 0) glColor3f(1, 0.84f, 0);
        else if (i == 1) glColor3f(0.8f, 0.8f, 0.85f);
        else if (i == 2) glColor3f(0.8f, 0.5f, 0.2f);
        else             glColor3f(0.7f, 0.7f, 0.7f);
        sprintf(buf, "%d.  SCORE: %6d     LEVEL: %d", i+1, hs[i].sc, hs[i].lv);
        drawStrL(SCR_W/2.0f - 150, 140 + i*48, buf);
    }
    if (hsN == 0) {
        glColor3f(0.6f, 0.6f, 0.6f);
        drawStrL(SCR_W/2.0f - 80, 220, "No scores yet!");
    }

    glColor3f(0.45f, 0.45f, 0.45f);
    drawStr(SCR_W/2.0f - 100, SCR_H - 20, "Press any key or click to go back");
}





static void drawOverlay(const char* title, float tr, float tg, float tb) {
    blendOn();
    glColor4f(0, 0, 0, 0.76f);
    fillR(0, 0, SCR_W, SCR_H);
    blendOff();

    glColor3f(0.08f, 0.08f, 0.08f);
    roundBox(SCR_W/2.0f - 150, SCR_H/2.0f - 135, 300, 265, 14);

    glLineWidth(2);
    glColor3f(tr, tg, tb);
    glBegin(GL_LINE_LOOP);
    glVertex2f(SCR_W/2.0f - 150, SCR_H/2.0f - 135);
    glVertex2f(SCR_W/2.0f + 150, SCR_H/2.0f - 135);
    glVertex2f(SCR_W/2.0f + 150, SCR_H/2.0f + 130);
    glVertex2f(SCR_W/2.0f - 150, SCR_H/2.0f + 130);
    glEnd();

    glColor3f(tr, tg, tb);
    drawStrXL(SCR_W/2.0f - 95, SCR_H/2.0f - 105, title);

    char buf[64];
    glColor3f(1, 1, 1);
    sprintf(buf, "SCORE : %d", score);
    drawStrL(SCR_W/2.0f - 95, SCR_H/2.0f - 65, buf);
    sprintf(buf, "LEVEL : %d", level);
    drawStrL(SCR_W/2.0f - 95, SCR_H/2.0f - 35, buf);
    sprintf(buf, "TIME  : %.0fs", elapsed);
    drawStrL(SCR_W/2.0f - 95, SCR_H/2.0f + 5, buf);

    glColor3f(1, 0.84f, 0);
    drawStrL(SCR_W/2.0f - 105, SCR_H/2.0f + 55, "ENTER : Play Again");
    drawStrL(SCR_W/2.0f - 75,  SCR_H/2.0f + 85, "ESC : Menu");
}

static void drawWin() {
    blendOn();
    glColor4f(0, 0, 0, 0.76f);
    fillR(0, 0, SCR_W, SCR_H);
    blendOff();

    glColor3f(0.08f, 0.08f, 0.08f);
    roundBox(SCR_W/2.0f - 150, SCR_H/2.0f - 140, 300, 280, 14);

    glLineWidth(2);
    float t2 = elapsed * 3;
    glColor3f(0.5f + 0.5f * sinf(t2),
              0.5f + 0.5f * sinf(t2 + 2),
              0.5f + 0.5f * sinf(t2 + 4));
    glBegin(GL_LINE_LOOP);
    glVertex2f(SCR_W/2.0f - 150, SCR_H/2.0f - 140);
    glVertex2f(SCR_W/2.0f + 150, SCR_H/2.0f - 140);
    glVertex2f(SCR_W/2.0f + 150, SCR_H/2.0f + 140);
    glVertex2f(SCR_W/2.0f - 150, SCR_H/2.0f + 140);
    glEnd();

    if (level < 5) {
        glColor3f(0.2f, 1, 0.3f);
        drawStrXL(SCR_W/2.0f - 90, SCR_H/2.0f - 110, "LEVEL CLEAR!");
    } else {
        glColor3f(1, 0.84f, 0);
        drawStrXL(SCR_W/2.0f - 80, SCR_H/2.0f - 110, "YOU WIN!!!");
    }

    char buf[64];
    glColor3f(1, 1, 1);
    sprintf(buf, "SCORE : %d", score);
    drawStrL(SCR_W/2.0f - 95, SCR_H/2.0f - 70, buf);
    sprintf(buf, "LEVEL : %d", level);
    drawStrL(SCR_W/2.0f - 95, SCR_H/2.0f - 40, buf);
    sprintf(buf, "TIME  : %.0fs", elapsed);
    drawStrL(SCR_W/2.0f - 95, SCR_H/2.0f - 10, buf);

    if (level < 5) {
        glColor3f(0.4f, 1, 0.4f);
        int secs = (int)(bannerT) + 1;
        sprintf(buf, "Next Level in  %d ...", secs);
        drawStrL(SCR_W/2.0f - 105, SCR_H/2.0f + 38, buf);
        glColor3f(0.7f, 0.7f, 0.7f);
        drawStrL(SCR_W/2.0f - 100, SCR_H/2.0f + 75,  "ENTER = Skip wait");
        drawStrL(SCR_W/2.0f - 75,  SCR_H/2.0f + 105, "ESC = Menu");
    } else {
        glColor3f(1, 0.84f, 0);
        drawStrL(SCR_W/2.0f - 110, SCR_H/2.0f + 50, "All 5 levels completed!");
        glColor3f(0.7f, 0.7f, 0.7f);
        drawStrL(SCR_W/2.0f - 105, SCR_H/2.0f + 80,  "ENTER : Play Again");
        drawStrL(SCR_W/2.0f - 75,  SCR_H/2.0f + 110, "ESC : Menu");
    }
}

static void drawPaused() {
    blendOn();
    glColor4f(0, 0, 0, 0.64f);
    fillR(0, 0, SCR_W, SCR_H);
    blendOff();

    glColor3f(0.08f, 0.08f, 0.08f);
    roundBox(SCR_W/2.0f - 135, SCR_H/2.0f - 80, 270, 165, 12);

    glColor3f(1, 0.84f, 0);
    drawStrXL(SCR_W/2.0f - 55, SCR_H/2.0f - 50, "PAUSED");

    glColor3f(0.85f, 0.85f, 0.85f);
    drawStrL(SCR_W/2.0f - 80, SCR_H/2.0f - 5,  "R     : Resume");
    drawStrL(SCR_W/2.0f - 80, SCR_H/2.0f + 25, "P     : Pause / Unpause");
    drawStrL(SCR_W/2.0f - 80, SCR_H/2.0f + 55, "ESC   : Menu");
}


static void ghostPickNext(Ghost& gh) {
    int gc  = tileX(gh.tgtX);
    int gr2 = tileY(gh.tgtY);
    int pc  = tileX(pX);
    int pr  = tileY(pY);
    int tarC = pc;
    int tarR = pr;

    if (gh.fright || perkInvisible) {
        int spread = gh.fright ? 3 : 5;
        int range  = gh.fright ? 7 : 11;
        tarC = gc  + (rand() % range) - spread;
        tarR = gr2 + (rand() % range) - spread;
    } else {
        switch (gh.id) {
        case BLINKY:
            tarC = pc;
            tarR = pr;
            break;
        case PINKY:
            tarC = pc + pDX * 4;
            tarR = pr + pDY * 4;
            break;
        case INKY: {
            int s = (int)(elapsed / 4) % 2;
            tarC = pc + pDX*3 + (s ? 4 : -4);
            tarR = pr + pDY*3 + (s ? 3 : -3);
        } break;
        case CLYDE: {
            int dx2 = gc - pc;
            int dy2 = gr2 - pr;
            if (dx2*dx2 + dy2*dy2 < 64) {
                tarC = 0;
                tarR = ROWS - 1;
            } else {
                tarC = pc;
                tarR = pr;
            }
        } break;
        }
    }

    int odx = -gh.dx;
    int ody = -gh.dy;
    int bdc = 0, bdr = 0, bD = 99999;
    bool found = false;
    int ds[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};

    for (auto& d : ds) {
        if (d[0] == odx && d[1] == ody)    continue;
        if (isWall(gc + d[0], gr2 + d[1])) continue;
        int dist = (gc+d[0]-tarC)*(gc+d[0]-tarC)
                 + (gr2+d[1]-tarR)*(gr2+d[1]-tarR);
        if (!found || dist < bD) {
            bD    = dist;
            bdc   = d[0];
            bdr   = d[1];
            found = true;
        }
    }
    if (!found) {
        bdc = -odx;
        bdr = -ody;
    }

    gh.dx   = bdc;
    gh.dy   = bdr;
    gh.tgtX = cellCx(gc + bdc);
    gh.tgtY = cellCy(gr2 + bdr);
}

static void loseLife() {
    lives--;
    burst(pX, pY, 1, 0.3f, 0, 22);
    flashT = 0.35f;
    sndDie();
    pX  = cellCx(1);
    pY  = cellCy(1);
    pDX = 1;
    pDY = 0;
    nDX = 0;
    nDY = 0;
    perkInvisible = false;
    perkFreeze    = false;
    perkSpeed     = false;

    if (lives <= 0) {
        if (hsN < 5)
            hs[hsN++] = {score, level};
        else if (score > hs[4].sc)
            hs[4] = {score, level};
        sort(hs, hs + hsN, [](const HS& a, const HS& b){ return a.sc > b.sc; });
        gs = GAMEOVER;
    }
}



static void updatePac(float dt) {
    float spd = PAC_SPD * (perkSpeed ? 2.0f : 1.0f);

    if (nDX || nDY) {
        if (!isWall(tileX(pX + nDX * CELL * 0.6f),
                    tileY(pY + nDY * CELL * 0.6f))) {
            pDX = nDX;
            pDY = nDY;
        }
    }

    float nx = pX + pDX * spd * dt;
    float ny = pY + pDY * spd * dt;
    if (!isWall(tileX(nx + pDX * CELL * 0.45f),
                tileY(ny + pDY * CELL * 0.45f))) {
        pX = nx;
        pY = ny;
    }

    if (pX < -CELL / 2.0f)            pX = COLS * CELL + CELL / 2.0f;
    if (pX >  COLS * CELL + CELL/2.0f) pX = -CELL / 2.0f;

    int ec = tileX(pX);
    int er = tileY(pY);
    if (ec >= 0 && ec < COLS && er >= 0 && er < ROWS) {
        int v = maze[er][ec];
        if (v == 2) {
            maze[er][ec] = 0;
            score += 10;
            totalDots--;
            burst(pX, pY, 1, 0.9f, 0.5f, 4);
            sndDot();
        } else if (v == 3) {
            maze[er][ec] = 0;
            score += 50;
            totalDots--;
            powered   = true;
            powerLeft = 7.0f;
            flashT    = 0.28f;
            burst(pX, pY, 1, 0.5f, 0.1f, 18);
            sndPellet();
            for (int i = 0; i < 4; i++) {
                G[i].fright     = true;
                G[i].frightLeft = 7.0f;
            }
        } else if (v >= 4 && v <= 6) {
            maze[er][ec] = 0;
            totalDots--;
            if (v == 4) {
                perkInvisible = true;
                perkInvLeft   = 4.0f;
                burst(pX, pY, 0, 1, 1, 14);
                snprintf(bannerTxt, 48, "INVISIBLE!");
            } else if (v == 5) {
                perkFreeze  = true;
                perkFrzLeft = 4.0f;
                burst(pX, pY, 0.5f, 0.7f, 1, 14);
                snprintf(bannerTxt, 48, "GHOSTS FROZEN!");
            } else {
                perkSpeed   = true;
                perkSpdLeft = 5.0f;
                burst(pX, pY, 1, 0.9f, 0, 14);
                snprintf(bannerTxt, 48, "SPEED BOOST!");
            }
            bannerT = 1.8f;
            sndPerk();
        }
    }

    mouth += mouthSpd * dt * 130;
    if (mouth > 38) mouthSpd = -fabsf(mouthSpd);
    if (mouth <  3) mouthSpd =  fabsf(mouthSpd);

    if (powered) {
        powerLeft -= dt;
        if (powerLeft <= 0) powered = false;
    }
}


static void initLevel(int lv) {
    level     = lv;
    score     = 0;
    lives     = 3;
    elapsed   = 0;
    totalDots = 0;
    memcpy(maze, BASE, sizeof(maze));

    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (maze[r][c] >= 2 && maze[r][c] <= 6) totalDots++;

    pX        = cellCx(1);
    pY        = cellCy(1);
    pDX       = 1;
    pDY       = 0;
    nDX       = 0;
    nDY       = 0;
    mouth     = 5;
    mouthSpd  = 3;
    powered   = false;
    powerLeft = 0;
    perkInvisible = false;
    perkFreeze    = false;
    perkSpeed     = false;
    fires.clear();
    parts.clear();

    float spd = min(68.0f + (lv - 1) * 10.0f, 140.0f);

    auto mk = [&](int tc, int tr, float r, float g, float b, GhostID id) -> Ghost {
        Ghost gh = {};
        gh.x    = cellCx(tc);
        gh.y    = cellCy(tr);
        gh.dx   = 1;
        gh.dy   = 0;
        gh.tgtX = gh.x;
        gh.tgtY = gh.y;
        gh.r    = r;
        gh.g    = g;
        gh.b    = b;
        gh.id   = id;
        gh.speed  = spd;
        gh.fireCD = 0.8f;
        return gh;
    };

    G[0] = mk(18, 18, 1,    0.05f, 0.05f, BLINKY);
    G[1] = mk(10, 10, 1,    0.72f, 0.80f, PINKY);
    G[2] = mk( 5, 15, 0,    1,     1,     INKY);
    G[3] = mk(15,  5, 1,    0.55f, 0.10f, CLYDE);

    for (int i = 0; i < 4; i++) ghostPickNext(G[i]);

    snprintf(bannerTxt, 48, "LEVEL %d", lv);
    bannerT = 2.5f;
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (gs == HIGHSCORE) { drawHighScores(); glutSwapBuffers(); return; }
    if (gs == HELP)      { drawHelp();       glutSwapBuffers(); return; }
    if (gs == MENU)      { drawMenu();       glutSwapBuffers(); return; }

    drawMaze();

    blendOn();
    for (auto& p : parts) {
        float a = p.life / 0.85f;
        if (a > 1) a = 1;
        glColor4f(p.r, p.g, p.b, a);
        fillC(p.x, p.y, 3.5f * a);
    }
    blendOff();

    drawGhosts();
    drawPac();
    drawHUD();

    if (flashT > 0) {
        blendOn();
        glColor4f(1, 0.84f, 0, flashT * 2);
        fillR(0, 0, SCR_W, (float)MAZE_H);
        blendOff();
    }

    if (bannerT > 0 && gs != WIN) {
        float a = bannerT > 1 ? 1 : bannerT;
        blendOn();
        glColor4f(0, 0, 0, 0.74f * a);
        roundBox(SCR_W/2.0f - 130, SCR_H/2.0f - 30, 260, 52, 10);
        blendOff();
        glColor3f(1, 0.84f, 0);
        drawStrXL(SCR_W/2.0f - 95, SCR_H/2.0f + 10, bannerTxt);
    }

    if (gs == PAUSED)   drawPaused();
    if (gs == GAMEOVER) drawOverlay("GAME OVER", 1, 0.2f, 0.2f);
    if (gs == WIN)      drawWin();

    glutSwapBuffers();
}


void update(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    if (pDX == 0 && pDY == 0 && (nDX || nDY)) {
        pDX = nDX;
        pDY = nDY;
    }
    float dt = (now - lastMs) / 1000.0f;
    lastMs = now;
    if (dt > 0.05f) dt = 0.05f;

    if (gs == PLAYING) {
        elapsed += dt;
        updatePac(dt);
        for (int i = 0; i < 4; i++) updateGhost(G[i], dt);

        for (auto& f : fires) {
            f.x    += f.vx * dt;
            f.y    += f.vy * dt;
            f.life -= dt;
            float dx = pX - f.x;
            float dy = pY - f.y;
            if (dx*dx + dy*dy < (CELL*0.55f)*(CELL*0.55f)) {
                f.life = -1;
                loseLife();
                if (gs == GAMEOVER) goto done;
            }
        }
        fires.erase(
            remove_if(fires.begin(), fires.end(), [](const Fire& f) {
                return f.life <= 0 || f.x < 0 || f.x > SCR_W
                                   || f.y < 0 || f.y > (float)MAZE_H;
            }),
            fires.end());

        for (auto& p : parts) {
            p.x    += p.vx * dt;
            p.y    += p.vy * dt;
            p.vy   += 55 * dt;
            p.life -= dt;
        }
        parts.erase(
            remove_if(parts.begin(), parts.end(), [](const Part& p) {
                return p.life <= 0;
            }),
            parts.end());

        if (perkInvisible) { perkInvLeft -= dt; if (perkInvLeft <= 0) perkInvisible = false; }
        if (perkFreeze)    { perkFrzLeft -= dt; if (perkFrzLeft <= 0) perkFreeze    = false; }
        if (perkSpeed)     { perkSpdLeft -= dt; if (perkSpdLeft <= 0) perkSpeed     = false; }
        if (flashT  > 0) flashT  -= dt;
        if (bannerT > 0) bannerT -= dt;

        if (totalDots <= 0) {
            if (hsN < 5)
                hs[hsN++] = {score, level};
            else if (score > hs[4].sc)
                hs[4] = {score, level};
            sort(hs, hs + hsN, [](const HS& a, const HS& b){ return a.sc > b.sc; });
            gs      = WIN;
            bannerT = 3.0f;
            snprintf(bannerTxt, 48,
                level < 5 ? "LEVEL CLEAR!  Next in 3s..." : "ALL LEVELS DONE!  CHAMPION!");
            sndLevelUp();
        }
    }

    if (gs == WIN) {
        bannerT -= dt;
        if (bannerT <= 0) {
            if (level < 5) {
                int saved = score;
                initLevel(level + 1);
                score  = saved;
                gs     = PLAYING;
                lastMs = glutGet(GLUT_ELAPSED_TIME);
            } else {
                gs = GAMEOVER;
            }
        }
    }

done:
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


void keyboard(unsigned char k, int, int) {
    if (gs == HIGHSCORE || gs == HELP) { gs = MENU; return; }

    if (k == 27) {
        if (gs == PLAYING || gs == PAUSED) hasResume = true;
        gs = MENU;
        return;
    }

    if (k == 13) {
        if (gs == MENU) {
            if      (menuSel == 0) { initLevel(1); gs = PLAYING; hasResume = false; lastMs = glutGet(GLUT_ELAPSED_TIME); }
            else if (menuSel == 1) gs = HIGHSCORE;
            else if (menuSel == 2) gs = HELP;
            else                   exit(0);
        } else if (gs == GAMEOVER) {
            initLevel(1);
            gs        = PLAYING;
            hasResume = false;
            lastMs    = glutGet(GLUT_ELAPSED_TIME);
        } else if (gs == WIN) {
            if (level < 5) {
                int saved = score;
                initLevel(level + 1);
                score  = saved;
                gs     = PLAYING;
                lastMs = glutGet(GLUT_ELAPSED_TIME);
            } else {
                initLevel(1);
                gs        = PLAYING;
                hasResume = false;
                lastMs    = glutGet(GLUT_ELAPSED_TIME);
            }
        }
        return;
    }

    if (k == 'p' || k == 'P') {
        if (gs == PLAYING) gs = PAUSED;
        else if (gs == PAUSED) { gs = PLAYING; lastMs = glutGet(GLUT_ELAPSED_TIME); }
        return;
    }
    if (k == 'r' || k == 'R') {
        if (gs == PAUSED) { gs = PLAYING; lastMs = glutGet(GLUT_ELAPSED_TIME); }
        return;
    }

    if (gs == PLAYING) {
        if      (k == 'w' || k == 'W') { nDX =  0; nDY = -1; }
        else if (k == 's' || k == 'S') { nDX =  0; nDY =  1; }
        else if (k == 'a' || k == 'A') { nDX = -1; nDY =  0; }
        else if (k == 'd' || k == 'D') { nDX =  1; nDY =  0; }
    }
}

void specialKey(int k, int, int) {
    if (gs == PLAYING) {
        if (k == GLUT_KEY_UP)    { nDX =  0; nDY = -1; }
        if (k == GLUT_KEY_DOWN)  { nDX =  0; nDY =  1; }
        if (k == GLUT_KEY_LEFT)  { nDX = -1; nDY =  0; }
        if (k == GLUT_KEY_RIGHT) { nDX =  1; nDY =  0; }
    }
    if (gs == MENU) {
        if (k == GLUT_KEY_UP)   menuSel = (menuSel - 1 + 4) % 4;
        if (k == GLUT_KEY_DOWN) menuSel = (menuSel + 1)     % 4;
    }
}


void mouseClick(int btn, int st, int mx, int my) {
    if (btn != GLUT_LEFT_BUTTON || st != GLUT_UP) return;
    if (gs == HIGHSCORE || gs == HELP) { gs = MENU; return; }
    if (gs == MENU) {
        for (int i = 0; i < 4; i++) {
            float bx = SCR_W/2.0f - 110;
            float by = 240.0f + i * 55;
            if (mx > bx && mx < bx + 220 && my > by && my < by + 40) {
                menuSel = i;
                keyboard(13, 0, 0);
                return;
            }
        }
    }
}


void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, SCR_W, SCR_H, 0);
    glMatrixMode(GL_MODELVIEW);
}


int main(int argc, char** argv) {
    srand((unsigned)time(0));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(SCR_W, SCR_H);
    glutCreateWindow("Pac-Man - CSE 426 Computer Graphics Lab");

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, SCR_W, SCR_H, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0, 0, 0, 1);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutMouseFunc(mouseClick);
    glutTimerFunc(16, update, 0);

    lastMs = glutGet(GLUT_ELAPSED_TIME);
    glutMainLoop();
    return 0;
}
