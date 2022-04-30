#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

//-----------------------------MAP----------------------------------------------
#define mapX  8      // Largeur de la carte (en tuiles)
#define mapY  8      // Hauteur de la carte (en tuiles)
#define squareSize 64      // Taille des tuiles de la carte

// Taille de la fenêtre
const int L = 1024;
const int H = 512;

int FOV = 60;  // Champ de vision
float quality = 1.0;

// La carte, un chiffre = une tuile, 1 = mur, 0 = vide
int map[] = {
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
    int x, y, squarePosX, squarePosY;
    for (y = 0; y < mapY; y++) {
        for (x = 0; x < mapX; x++) {
            if (map[y * mapX + x] == 1) {
                glColor3f(1, 1, 1);  // Couleur des carrés : blanc
            }
            else {
                glColor3f(0, 0, 0);  // Couleur des carrés : noir
            }

            squarePosX = x * squareSize;
            squarePosY = y * squareSize;
            glBegin(GL_QUADS); // On indique que l'on va créer un quadrilatère$
            // On indique les coordonnées de chaque sommet du quadrilatère
            glVertex2i(0 + squarePosX + 1, 0 + squarePosY + 1);
            glVertex2i(0 + squarePosX + 1, squareSize + squarePosY - 1);
            glVertex2i(squareSize + squarePosX - 1, squareSize + squarePosY - 1);
            glVertex2i(squareSize + squarePosX - 1, 0 + squarePosY + 1);
            glEnd();  // On trace le quadrilatère
        }
    }
}//-----------------------------------------------------------------------------


//------------------------PLAYER------------------------------------------------
float degToRad(int angle) {
    return angle * M_PI / 180.0;  // Conversion des degrés en radians car les fonctions cos() et sin() accepte des angles en radians
}


float FixAng(float angle) {
    // On fait en sorte que les angles 0° et 360° se suivent
    if (angle > 359) {
        angle -= 360;
    }
    if (angle < 0) {
        angle += 360;
    }
    return angle;
}

float playerX,playerY,playerMoveDirectionX,playerMoveDirectionY,playerAngle;

void drawPlayer2D() {
    glColor3f(1, 1, 0);  // Couleur jaune
    glPointSize(8);
    glLineWidth(4);
    glBegin(GL_POINTS);
    glVertex2i(playerX, playerY);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(playerX, playerY);
    glVertex2i(playerX + playerMoveDirectionX * 20, playerY + playerMoveDirectionY * 20);
    glEnd();
}

void Buttons(unsigned char key, int x, int y) {
    // On fait en sorte que les touches zqsd permettent de contrôler le joueur
    if (key == 'q') {
        playerAngle += 5;
        playerAngle = FixAng(playerAngle);
        playerMoveDirectionX = cos(degToRad(playerAngle));
        playerMoveDirectionY = -sin(degToRad(playerAngle));
    }

    if (key == 'd') {
        playerAngle -= 5;
        playerAngle = FixAng(playerAngle);
        playerMoveDirectionX = cos(degToRad(playerAngle));
        playerMoveDirectionY = -sin(degToRad(playerAngle));
    }

    if (key == 'z') {
        playerX += playerMoveDirectionX * 5;
        playerY += playerMoveDirectionY * 5;
    }

    if (key == 's') {
        playerX -= playerMoveDirectionX * 5;
        playerY -= playerMoveDirectionY * 5;
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

    ra = FixAng(playerAngle + FOV/2);                                                              //ray set back 30 degrees
    nbRays = int(FOV * quality);
    //Raycasting
    for (r = 0; r < nbRays; r++) {
        //----------Vertical rays----------
        dof = 0;
        side = 0;
        disV = 100000;
        float Tan = tan(degToRad(ra));

        if (cos(degToRad(ra)) > 0.001) {  //looking left
            rx = (((int)playerX >> 6) << 6) + 64;
            ry = (playerX - rx) * Tan + playerY;
            xo = 64; yo = -xo * Tan;
        }
        else if (cos(degToRad(ra)) < -0.001) {   //looking right
            rx = (((int)playerX >> 6) << 6) - 0.0001;
            ry = (playerX - rx) * Tan + playerY;
            xo = -64;
            yo = -xo * Tan;
        }
        else {    //looking up or down. no hit 
            rx = playerX;
            ry = playerY;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1) {    //hit
                dof = 8;
                disV = cos(degToRad(ra)) * (rx - playerX) - sin(degToRad(ra)) * (ry - playerY);
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
            ry = (((int)playerY >> 6) << 6) - 0.0001;
            rx = (playerY - ry) * Tan + playerX;
            yo = -64; xo = -yo * Tan;
        }
        else if (sin(degToRad(ra)) < -0.001) { //looking down
            ry = (((int)playerY >> 6) << 6) + 64;
            rx = (playerY - ry) * Tan + playerX;
            yo = 64;
            xo = -yo * Tan;
        }
        else {  //looking straight left or right
            rx = playerX;
            ry = playerY;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1) {   //hit
                dof = 8;
                disH = cos(degToRad(ra)) * (rx - playerX) - sin(degToRad(ra)) * (ry - playerY);
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
        glVertex2i(playerX, playerY);
        glVertex2i(rx, ry);
        glEnd();

        //fix fisheye 
        int ca = FixAng(playerAngle - ra);
        disH = disH * cos(degToRad(ca));

        int lineH = (squareSize * 640) / (disH);
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
    playerX = 150;
    playerY = 400;
    playerAngle = 0;
    playerMoveDirectionX = cos(degToRad(playerAngle));
    playerMoveDirectionY = -sin(degToRad(playerAngle));
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
    glutCreateWindow("Raycaster");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(Buttons);
    glutMainLoop();
}

