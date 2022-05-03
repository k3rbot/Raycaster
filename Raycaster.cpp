#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

//-----------------------------CARTE----------------------------------------------
#define mapX  8      // Largeur de la carte (en tuiles)
#define mapY  8      // Hauteur de la carte (en tuiles)
#define tileSize 64      // Taille des tuiles de la carte

const int L = 1024;
const int H = 512;
int FOV = 60;  // Champ de vision
float quality = 1.0;

int map[] = {          // Carte
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

            squarePosX = x * tileSize;
            squarePosY = y * tileSize;
            glBegin(GL_QUADS); // On indique que l'on va créer un quadrilatère
            // On indique les coordonnées de chaque sommet du quadrilatère
            glVertex2i(0 + squarePosX + 1, 0 + squarePosY + 1);
            glVertex2i(0 + squarePosX + 1, tileSize + squarePosY - 1);
            glVertex2i(tileSize + squarePosX - 1, tileSize + squarePosY - 1);
            glVertex2i(tileSize + squarePosX - 1, 0 + squarePosY + 1);
            glEnd();  // On trace le quadrilatère
        }
    }
}//-----------------------------------------------------------------------------


//------------------------JOUEUR------------------------------------------------
float degToRad(int angle) {
    return angle * M_PI / 180.0;  // Conversion degrés en radians car les fonctions cos() et sin() acceptent des angles en radians
}


float FixAng(float angle) {
    // On remet à zéro l'angle quand on atteint les 360°
    if (angle > 359) {
        angle -= 360;
    }
    if (angle < 0) {
        angle += 360;
    }
    return angle;
}

float playerX, playerY, playerMoveDirectionX, playerMoveDirectionY, playerAngle;

void drawPlayer2D() {
    glColor3f(1, 1, 0);  // Couleur du joueur (jaune)
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

void movePlayer(float x, float y) {
    // On enregistre la future position du joueur si il n'y avait pas de mur
    float nextX = playerX + x;
    float nextY = playerY + y;

    // On calcule la position du bloc le plus proche après déplacement dans map[]
    int tileX = trunc(nextX / tileSize);
    int tileY = trunc(nextY / tileSize);

    int tileId = tileY * mapX + tileX;

    // On regarde si le bloc est un mur
    if (map[tileId] == 1) {
        float collisionX = 0;
        float collisionY = 0;

        // On détermine la distance absolue au milieu du bloc sur les axes x et y
        if (nextX - (tileSize * (tileX + 0.5)) < tileSize / 2) {
            collisionX = tileSize / 2 - abs((tileSize * (tileX + 0.5)) - nextX);
        }
        if (nextY - (tileSize * (tileY + 0.5)) < tileSize / 2) {
            collisionY = tileSize / 2 - abs((tileSize * (tileY + 0.5)) - nextY);
        }

        // On détermine si on est à droite/gauche ou en dessous/au dessus du bloc
        if (collisionX < collisionY) {
            // On détermine si on est à droite ou à gauche du bloc
            if (nextX < tileSize * (tileX + 0.5)) {
                nextX = tileSize * tileX - 1;
            }
            else {
                nextX = tileSize * (tileX + 1) + 1;
            }
        }
        else {
            // On détermine si on est au dessus ou en dessous du bloc
            if (nextY < tileSize * (tileY + 0.5)) {
                nextY = tileSize * tileY - 1;
            }
            else {
                nextY = tileSize * (tileY + 1) + 1;
            }
        }

    int nextTileX = trunc(nextX / tileSize);
    int nextTileY = trunc(nextY / tileSize);

    if (map[nextTileY * mapX + nextTileX] == 1) {
        if (map[nextTileY * mapX +  tileX] == 1) {
            nextY -= y;
        }
        if (map[nextTileY * mapX +  tileX] == 1)  {
            nextX -= x;
        }
    }

    }
    // On effectue les déplacements pour de vrai
    playerX = nextX;
    playerY = nextY;
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
        movePlayer(playerMoveDirectionX * 5, playerMoveDirectionY * 5);
    }

    if (key == 's') {
        movePlayer(-playerMoveDirectionX * 5, -playerMoveDirectionY * 5);
    }
    glutPostRedisplay();    // On actualise l'affichage
}
//-----------------------------------------------------------------------------


//---------------------Affichage des rayon et des murs-------------------------
float distance(float ax, float ay, float bx, float by, float ang) {
    return cos(degToRad(ang)) * (bx - ax) - sin(degToRad(ang)) * (by - ay);
}

void drawRays2D() {
    int r, mx, my, mp, dof, side, nbRays, line_width;
    float vx, vy, rx, ry, ra, xo, yo, disV, disH, lineOff;

    ra = FixAng(playerAngle + FOV / 2);     // On trace le premier rayon en commençant à gauche et à un angle dépendant du champs de vision
    nbRays = int(FOV * quality);    // Nombre de rayons à tracer en fonction du champs de vision et du niveau de détail voulu
    line_width = 511 / nbRays;  // Epaisseur d'une ligne pour l'affichage "3d" dépendant du nombre de lignes à tracer

    // Rayons
    for (r = 0; r < nbRays; r++) {
        //----------Rayons verticaux----------
        dof = 0;
        side = 0;
        disV = 100000;
        float Tan = tan(degToRad(ra));

        if (cos(degToRad(ra)) > 0.001) {  // On regarde à gauche
            rx = (((int)playerX >> 6) << 6) + 64;
            ry = (playerX - rx) * Tan + playerY;
            xo = 64; yo = -xo * Tan;
        }
        else if (cos(degToRad(ra)) < -0.001) {   // On regarde à droite
            rx = (((int)playerX >> 6) << 6) - 0.0001;
            ry = (playerX - rx) * Tan + playerY;
            xo = -64;
            yo = -xo * Tan;
        }
        else {    // On regarde pile en haut ou en bas, on ne toucheras donc jamais un mur vertical
            rx = playerX;
            ry = playerY;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1) {    // Touché !
                dof = 8;
                disV = cos(degToRad(ra)) * (rx - playerX) - sin(degToRad(ra)) * (ry - playerY);
            }
            else {    // Pas de colision, on va voir à la prochaine intersection verticale
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }
        vx = rx;
        vy = ry;

        //----------Rayons horizontaux----------
        dof = 0;
        disH = 100000;
        Tan = 1.0 / Tan;
        if (sin(degToRad(ra)) > 0.001) {  // On regarde en haut
            ry = (((int)playerY >> 6) << 6) - 0.0001;
            rx = (playerY - ry) * Tan + playerX;
            yo = -64; xo = -yo * Tan;
        }
        else if (sin(degToRad(ra)) < -0.001) { // On regarde en bas
            ry = (((int)playerY >> 6) << 6) + 64;
            rx = (playerY - ry) * Tan + playerX;
            yo = 64;
            xo = -yo * Tan;
        }
        else {  // On regarde pile à droite ou à gauche, on ne toucheras donc jamais un mur horizontal
            rx = playerX;
            ry = playerY;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * mapX + mx;
            if (mp > 0 && mp < mapX * mapY && map[mp] == 1) {   // Touché !
                dof = 8;
                disH = cos(degToRad(ra)) * (rx - playerX) - sin(degToRad(ra)) * (ry - playerY);
            }
            else {  // Pas de colision, on va voir à la prochaine intersection horizontale
                rx += xo;
                ry += yo;
                dof += 1;
            }
        }

        glColor3f(0, 0.8, 0);
        if (disV < disH) {  // On a touché un mur vertical en premier
            rx = vx;
            ry = vy;
            disH = disV;
            glColor3f(0, 0.6, 0);
        }

        // On affiche les rayons en 2D sur la minimap
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2i(playerX, playerY);
        glVertex2i(rx, ry);
        glEnd();

        // On règle le problème du "fisheye": les murs en face de nous nous apparaissent
        // complètement distordus car sur les cotés les rayons sont plus long c'est pourquoi en veut tout réaplanir
        int ca = FixAng(playerAngle - ra);
        disH = disH * cos(degToRad(ca));

        // On prépare l'affichage des lignes verticales représentant les rayons pour le joueur
        int lineH = (tileSize * 640) / (disH);
        if (lineH > 640) {  // On indique une limite de taille pour les lignes
            lineH = 640;
        }
        lineOff = 256 - lineH / 2; // On centre les lignes sur l'axe vertical

        // On affiche les lignes verticales
        glLineWidth(1);
        glBegin(GL_QUADS); // Quadrilatère
        glVertex2f(r * line_width + 512, lineOff);  // 1er rayon en bas à gauche
        glVertex2f((r + 1) * line_width + 513, lineOff);    // 2ème rayon en bas à droite
        glVertex2f((r + 1) * line_width + 513, lineH + lineOff);    // 3ème en haut à droite
        glVertex2f(r * line_width + 512, lineH + lineOff);  // 4ème rayon en haut à gauche
        glEnd();

        ra = FixAng(ra - 1 / quality);    // On change l'angle pour le prochain rayon
    }
}//-----------------------------------------------------------------------------


void init() {
    glClearColor(0.3, 0.3, 0.3, 0); // On met la couleur du fond en gris
    gluOrtho2D(0, L, H, 0); // On initialise une fenêtre de L largeur et H hauteur
    // Position et angle initial du joueur
    playerX = 150;
    playerY = 400;
    playerAngle = 0;
    // Directions pour la minimap
    playerMoveDirectionX = cos(degToRad(playerAngle));
    playerMoveDirectionY = -sin(degToRad(playerAngle));
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // On efface complètement l'écran en laissant la couleur de fond 
    drawMap2D();    // On affiche la minimap
    drawPlayer2D(); // On affiche le joueur sur la minimap
    drawRays2D();   // On affiche la vision "3d"
    glutSwapBuffers();  // On échange les buffers pour afficher sur l'écran ce que l'on vient de render
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

