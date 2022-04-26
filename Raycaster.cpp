#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>

//-----------------------------MAP----------------------------------------------
#define mapX  8      //map width
#define mapY  8      //map height
#define mapS 64      //map cube size

const int L = 1024;
const int H = 512;
int FOV = 60;
float quality = 1.0;

int map[] = {          //the map array. Edit to change level but keep the outer walls
    1,1,1,1,1,1,1,1,
    1,0,1,0,0,0,0,1,
    1,0,1,0,0,0,0,1,
    1,0,1,0,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,
};


void drawMap2D() {
    int x, y, xo, yo;
    for (y = 0; y < mapY; y++) {
        for (x = 0; x < mapX; x++) {
            if (map[y * mapX + x] == 1) {
                glColor3f(1, 1, 1);
            }
            else {
                glColor3f(0, 0, 0);
            }
            xo = x * mapS;
            yo = y * mapS;
            glBegin(GL_QUADS);
            glVertex2i(0 + xo + 1, 0 + yo + 1);
            glVertex2i(0 + xo + 1, mapS + yo - 1);
            glVertex2i(mapS + xo - 1, mapS + yo - 1);
            glVertex2i(mapS + xo - 1, 0 + yo + 1);
            glEnd();
        }
    }
}//-----------------------------------------------------------------------------


//------------------------PLAYER------------------------------------------------
float degToRad(int a) {
    return a * M_PI / 180.0;
}


float FixAng(float a) {
    if (a > 359) {
        a -= 360;
    }
    if (a < 0) {
        a += 360;
    }
    return a;
}

float px,py,pdx,pdy,pa;

void drawPlayer2D() {
    glColor3f(1, 1, 0);
    glPointSize(8);
    glLineWidth(4);
    glBegin(GL_POINTS);
    glVertex2i(px, py);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(px, py);
    glVertex2i(px + pdx * 20, py + pdy * 20);
    glEnd();
}

void Buttons(unsigned char key, int x, int y) {
    if (key == 'q') {
        pa += 5;
        pa = FixAng(pa);
        pdx = cos(degToRad(pa));
        pdy = -sin(degToRad(pa));
    }

    if (key == 'd') {
        pa -= 5;
        pa = FixAng(pa);
        pdx = cos(degToRad(pa));
        pdy = -sin(degToRad(pa));
    }

    if (key == 'z') {
        px += pdx * 5;
        py += pdy * 5;
    }

    if (key == 's') {
        px -= pdx * 5;
        py -= pdy * 5;
    }
    glutPostRedisplay();
}
//-----------------------------------------------------------------------------


//---------------------------Draw Rays and Walls--------------------------------
float distance(float ax, float ay, float bx, float by, float ang) {
    return cos(degToRad(ang)) * (bx - ax) - sin(degToRad(ang)) * (by - ay);
}

void drawRays2D() {
    int r, mx, my, mp, dof, side, nbRays;
    float vx, vy, rx, ry, ra, xo, yo, disV, disH, lineOff;

    ra = FixAng(pa + FOV/2);                                                              //ray set back 30 degrees
    nbRays = int(FOV * quality);
    //Raycasting
    for (r = 0; r < nbRays; r++) {
        //----------Vertical rays----------
        dof = 0;
        side = 0;
        disV = 100000;
        float Tan = tan(degToRad(ra));

        if (cos(degToRad(ra)) > 0.001) {  //looking left
            rx = (((int)px >> 6) << 6) + 64;
            ry = (px - rx) * Tan + py;
            xo = 64; yo = -xo * Tan;
        }
        else if (cos(degToRad(ra)) < -0.001) {   //looking right
            rx = (((int)px >> 6) << 6) - 0.0001;
            ry = (px - rx) * Tan + py;
            xo = -64;
            yo = -xo * Tan;
        }
        else {    //looking up or down. no hit 
            rx = px;
            ry = py;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1) {    //hit
                dof = 8;
                disV = cos(degToRad(ra)) * (rx - px) - sin(degToRad(ra)) * (ry - py);
            }
            else {    //check next horizontal
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }
        vx = rx;
        vy = ry;

        //----------Horizontal rays----------
        dof = 0;
        disH = 100000;
        Tan = 1.0 / Tan;
        if (sin(degToRad(ra)) > 0.001) {  //looking up 
            ry = (((int)py >> 6) << 6) - 0.0001;
            rx = (py - ry) * Tan + px;
            yo = -64; xo = -yo * Tan;
        }
        else if (sin(degToRad(ra)) < -0.001) { //looking down
            ry = (((int)py >> 6) << 6) + 64;
            rx = (py - ry) * Tan + px;
            yo = 64;
            xo = -yo * Tan;
        }
        else {  //looking straight left or right
            rx = px;
            ry = py;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1) {   //hit
                dof = 8;
                disH = cos(degToRad(ra)) * (rx - px) - sin(degToRad(ra)) * (ry - py);
            }
            else {  //check next horizontal
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }

        glColor3f(0, 0.8, 0);
        if (disV < disH) {  //horizontal hit first
            rx = vx;
            ry = vy;
            disH = disV;
            glColor3f(0, 0.6, 0);
        }

        //draw 2D ray
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2i(px, py);
        glVertex2i(rx, ry);
        glEnd();

        //fix fisheye 
        int ca = FixAng(pa - ra);
        disH = disH * cos(degToRad(ca));

        int lineH = (mapS * 640) / (disH);
        if (lineH > 640) {  //line height and limit
            lineH = 640;
        }
        lineOff = 250 - lineH / 2; //line offset

        //draw vertical wall
        glLineWidth(1);
        glBegin(GL_QUADS);
        glVertex2f(r * 511 / nbRays + 512, lineOff);
        glVertex2f(r * 511 / nbRays + 513 + 511 / nbRays, lineOff);
        glVertex2f(r * 511 / nbRays + 513 + 511 / nbRays, lineH + lineOff);
        glVertex2f(r * 511 / nbRays + 512, lineH + lineOff);
        glEnd();

        ra = FixAng(ra - 1 / quality);    //go to next ray
    }
}//-----------------------------------------------------------------------------


void init() {
    glClearColor(0.3, 0.3, 0.3, 0);
    gluOrtho2D(0, L, H, 0);
    px = 150;
    py = 400;
    pa = 90;
    pdx = cos(degToRad(pa));
    pdy = -sin(degToRad(pa));
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawMap2D();
    drawPlayer2D();
    drawRays2D();
    glutSwapBuffers();
}

int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024, 510);
    glutCreateWindow("YouTube-3DSage");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(Buttons);
    glutMainLoop();
}

