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