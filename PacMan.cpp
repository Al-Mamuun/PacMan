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

#define SCR_W   520
#define SCR_H   620
#define ROWS    20
#define COLS    20
#define CELL    26
#define HUD_H   60
#define MAZE_H  (ROWS*CELL)
#define PI_F    3.14159265f



static int BASE[ROWS][COLS]={
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
static int totalDots=0;
static float elapsed=0;

static bool isWall(int col,int row){
    if(col<0||col>=COLS||row<0||row>=ROWS) return true;
    return maze[row][col]==1;
}
static float cellCx(int col){ return col*CELL+CELL/2.0f; }
static float cellCy(int row){ return row*CELL+CELL/2.0f; }
static int tileX(float px){ return (int)(px/CELL); }
static int tileY(float py){ return (int)(py/CELL); }

static void drawPerkItem(int v,float x,float y){
    float cx2=x+CELL/2.0f, cy2=y+CELL/2.0f;
    float pulse=0.8f+0.2f*sinf(elapsed*4);
    blendOn();
    if(v==4){ c4(0,1,1,0.25f*pulse); fillC(cx2,cy2,10*pulse); c4(0,1,1,1); fillC(cx2,cy2,5*pulse); c4(1,1,1,0.9f); drawStr(cx2-3,cy2+4,"I"); }
    else if(v==5){ c4(0.4f,0.4f,1,0.25f*pulse); fillC(cx2,cy2,10*pulse); c4(0.5f,0.7f,1,1); fillC(cx2,cy2,5*pulse); c4(1,1,1,0.9f); drawStr(cx2-3,cy2+4,"F"); }
    else if(v==6){ c4(1,1,0,0.25f*pulse); fillC(cx2,cy2,10*pulse); c4(1,0.8f,0,1); fillC(cx2,cy2,5*pulse); c4(0,0,0,0.9f); drawStr(cx2-3,cy2+4,"S"); }
    blendOff();
}

static void drawMaze(){
    for(int r=0;r<ROWS;r++) for(int c=0;c<COLS;c++){
        float x=c*CELL, y=r*CELL;
        int v=maze[r][c];
        if(v==1){
            blendOn(); c4(0,0.2f,1,0.15f); fillR(x-1,y-1,CELL+2,CELL+2); blendOff();
            c3(0,0,0.6f); fillR(x,y,CELL,CELL);
            c3(0,0.4f,1); fillR(x+3,y+3,CELL-6,CELL-6);
        }
        else if(v==2){ c3(1,0.95f,0.75f); fillC(x+CELL/2.0f,y+CELL/2.0f,2.5f); }
        else if(v==3){
            float p=0.8f+0.2f*sinf(elapsed*5);
            blendOn(); c4(1,0.55f,0.05f,0.3f*p); fillC(x+CELL/2.0f,y+CELL/2.0f,9*p); blendOff();
            c3(1,0.55f,0.05f); fillC(x+CELL/2.0f,y+CELL/2.0f,6*p);
        }
        else if(v==4||v==5||v==6){ drawPerkItem(v,x,y); }
    }
}


static void c3(float r,float g,float b){ glColor3f(r,g,b); }
static void c4(float r,float g,float b,float a){ glColor4f(r,g,b,a); }
static void blendOn(){ glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); }
static void blendOff(){ glDisable(GL_BLEND); }

static void fillR(float x,float y,float w,float h){
    glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y);
    glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}
static void fillC(float x,float y,float rad,int seg=32){
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x,y);
    for(int i=0;i<=seg;i++){
        float a=2*PI_F*i/seg;
        glVertex2f(x+cosf(a)*rad,y+sinf(a)*rad);
    }
    glEnd();
}
static void drawStr(float x,float y,const char*s,void*f=GLUT_BITMAP_HELVETICA_12){
    glRasterPos2f(x,y);
    for(;*s;s++) glutBitmapCharacter(f,*s);
}
static void drawStrL(float x,float y,const char*s){ drawStr(x,y,s,GLUT_BITMAP_HELVETICA_18); }
static void drawStrXL(float x,float y,const char*s){ drawStr(x,y,s,GLUT_BITMAP_TIMES_ROMAN_24); }
static void roundBox(float x,float y,float w,float h,float rad){
    fillR(x+rad,y,w-2*rad,h); fillR(x,y+rad,w,h-2*rad);
    fillC(x+rad,y+rad,rad); fillC(x+w-rad,y+rad,rad);
    fillC(x+rad,y+h-rad,rad); fillC(x+w-rad,y+h-rad,rad);
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glutSwapBuffers();
}

void reshape(int w,int h){
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,SCR_W,SCR_H,0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc,char**argv){
    srand((unsigned)time(0));
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(SCR_W,SCR_H);
    glutCreateWindow("Pac-Man - CSE 426 Computer Graphics Lab");
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0,SCR_W,SCR_H,0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClearColor(0,0,0,1);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();
    return 0;
}