#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>
using namespace std;

#define MAPX  16      // Largeur de la carte (en tuiles)
#define MAPY  16      // Hauteur de la carte (en tuiles)
#define TILESIZE 64      // Taille des tuiles de la carte

const int L = 1024;     // Largeur de la fenêtre
const int H = 512;  // Hauteur de la fenêtre
const float minimapTileSizeRatioX = MAPX / 8;
const float minimapTileSizeRatioY = MAPY / 8;
int FOV = 60;  // Champ de vision
float quality = 10.0;

int map[] = {          // Carte
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,0,0,1,0,0,0,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,1,
    1,0,0,0,1,0,0,0,0,0,0,1,1,0,0,1,
    1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,
    1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,
    1,0,1,0,0,0,0,0,0,0,1,1,1,0,0,1,
    1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1,
    1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};


void drawMap2D() {
    /*
    Affichage de la minimap
    */

    int x, y, squarePosX, squarePosY, minimapTileX, minimapTileSizeY;
    minimapTileX = L / 2 / MAPX;    // Largeur y d'une tuile sur la minimap
    minimapTileSizeY = H / MAPY;    // Longueur x d'une tuile sur la minimap

    for (y = 0; y < MAPY; y++) {
        for (x = 0; x < MAPX; x++) {
            if (map[y * MAPX + x] == 1) {
                glColor3f(1, 1, 1);  // Couleur des carrés : blanc
            }
            else {
                glColor3f(0, 0, 0);  // Couleur des carrés : noir
            }

            squarePosX = x * minimapTileX;
            squarePosY = y * minimapTileSizeY;
            glBegin(GL_QUADS); // On indique que l'on va créer un quadrilatère
            // On indique les coordonnées de chaque sommet du quadrilatère
            glVertex2i(0 + squarePosX + 1, 0 + squarePosY + 1);
            glVertex2i(0 + squarePosX + 1, minimapTileSizeY + squarePosY - 1);
            glVertex2i(minimapTileX + squarePosX - 1, minimapTileSizeY + squarePosY - 1);
            glVertex2i(minimapTileX + squarePosX - 1, 0 + squarePosY + 1);
            glEnd();  // On trace le quadrilatère
        }
    }
}


float degToRad(float angle) {
    /*
    Conversion degrés en radians car les fonctions cos() et sin() acceptent des angles en radians
    */
    return angle * M_PI / 180.0;
}


float FixAng(float angle) {
    /*
    Remise à zéro de l'angle quand on atteint les 360°
    */

    if (angle > 359.0) {
        angle -= 360.0;
    }
    if (angle < 0.0) {
        angle += 360.0;
    }
    return angle;
}

float playerX, playerY, playerMoveDirectionX, playerMoveDirectionY, playerAngle;

void drawPlayer2D() {
    /*
    Affichage du joueur sur la minimap
    */

    glColor3f(1,1,0);  // Couleur du joueur (jaune)
    glPointSize(8);
    glLineWidth(4);
    glBegin(GL_POINTS);
    glVertex2i(playerX / minimapTileSizeRatioX, playerY / minimapTileSizeRatioY);
    glEnd();

    glBegin(GL_LINES);
    glVertex2i(playerX / minimapTileSizeRatioX, playerY / minimapTileSizeRatioY);
    glVertex2i(playerX / minimapTileSizeRatioX + playerMoveDirectionX * 20, playerY / minimapTileSizeRatioY + playerMoveDirectionY * 20);
    glEnd();
}

void movePlayer(float x, float y) {
    // On enregistre la future position du joueur si il n'y avait pas de mur
    float nextX = playerX + x;
    float nextY = playerY + y;

    // On calcule la position du bloc le plus proche après déplacement dans map[]
    int tileX = trunc(nextX / TILESIZE);
    int tileY = trunc(nextY / TILESIZE);

    int tileId = tileY * MAPX + tileX;

    // On regarde si le bloc est un mur
    if (map[tileId] == 1) {
        float collisionX = 0;
        float collisionY = 0;

        // On détermine la distance absolue au milieu du bloc sur les axes x et y
        if (nextX - (TILESIZE * (tileX + 0.5)) < TILESIZE / 2) {
            collisionX = TILESIZE / 2 - abs((TILESIZE * (tileX + 0.5)) - nextX);
        }
        if (nextY - (TILESIZE * (tileY + 0.5)) < TILESIZE / 2) {
            collisionY = TILESIZE / 2 - abs((TILESIZE * (tileY + 0.5)) - nextY);
        }

        // On détermine si on est à droite/gauche ou en dessous/au dessus du bloc
        if (collisionX < collisionY) {
            // On détermine si on est à droite ou à gauche du bloc
            if (nextX < TILESIZE * (tileX + 0.5)) {
                nextX = TILESIZE * tileX - 1;
            }
            else {
                nextX = TILESIZE * (tileX + 1) + 1;
            }
        }
        else {
            // On détermine si on est au dessus ou en dessous du bloc
            if (nextY < TILESIZE * (tileY + 0.5)) {
                nextY = TILESIZE * tileY - 1;
            }
            else {
                nextY = TILESIZE * (tileY + 1) + 1;
            }
        }

    int nextTileX = trunc(nextX / TILESIZE);
    int nextTileY = trunc(nextY / TILESIZE);

    if (map[nextTileY * MAPX + nextTileX] == 1) {
        if (map[nextTileY * MAPX +  tileX] == 1) {
            nextY -= y;
        }
        if (map[nextTileY * MAPX +  tileX] == 1)  {
            nextX -= x;
        }
    }

    }
    // On effectue les déplacements pour de vrai
    playerX = nextX;
    playerY = nextY;
}

void Buttons(unsigned char key, int x, int y) {
    /*
    Gestion des inputs du joueur
    */

    if (key == 'q') {   // On tourne la vision du joueur vers la gauche
        playerAngle += 5;
        playerAngle = FixAng(playerAngle);
        playerMoveDirectionX = cos(degToRad(playerAngle));
        playerMoveDirectionY = -sin(degToRad(playerAngle));
    }

    if (key == 'd') {   // On tourne la vision du joueur vers la droite
        playerAngle -= 5;
        playerAngle = FixAng(playerAngle);
        playerMoveDirectionX = cos(degToRad(playerAngle));
        playerMoveDirectionY = -sin(degToRad(playerAngle));
    }

    if (key == 'z') {   // On avance le joueur
        movePlayer(playerMoveDirectionX * 5, playerMoveDirectionY * 5);
    }

    if (key == 's') {   // On recule le joueur
        movePlayer(-playerMoveDirectionX * 5, -playerMoveDirectionY * 5);
    }
    glutPostRedisplay();    // On actualise l'affichage
}
//-----------------------------------------------------------------------------


//---------------------Affichage des rayon et des murs-------------------------
float distance(float ax, float ay, float bx, float by, float ang) {
    return cos(degToRad(ang)) * (bx - ax) - sin(degToRad(ang)) * (by - ay);
}

void drawRays() {
    /*
    Affichage des rayons sur la minimap et la "3d"
    */

    int mp, rayNb, wall, side, nbRays;
    float verticalRayX, verticalRayY, rayX, rayY, rayAngle, xOffset, yOffset, mx, my, disV, disH, lineOff, Tan, lineWidth;

    rayAngle = FixAng(playerAngle + FOV / 2);     // On trace le premier rayon en commençant à gauche et à un angle dépendant du champs de vision
    nbRays = FOV * quality;    // Nombre de rayons à tracer en fonction du champs de vision et du niveau de détail voulu
    lineWidth = 512.0 / nbRays;  // Epaisseur d'une ligne pour l'affichage "3d" dépendant du nombre de lignes à tracer

    // Rayons
    for (rayNb = 0; rayNb < nbRays; rayNb++) {
        //----------Rayons verticaux----------
        wall = 0;
        side = 0;
        disV = 100000;
        Tan = tan(degToRad(rayAngle));
        if (cos(degToRad(rayAngle)) > 0.001) {  // On regarde à gauche
            rayX = (((int)playerX >> 6) << 6) + 64;
            rayY = (playerX - rayX) * Tan + playerY;
            xOffset = 64;
            yOffset = -xOffset * Tan;
        }
        else if (cos(degToRad(rayAngle)) < -0.001) {   // On regarde à droite
            rayX = (((int)playerX >> 6) << 6) - 0.0001;
            rayY = (playerX - rayX) * Tan + playerY;
            xOffset = -64;
            yOffset = -xOffset * Tan;
        }
        else {    // On regarde pile en haut ou en bas, on ne toucheras donc jamais un mur vertical
            rayX = playerX;
            rayY = playerY;
            wall = MAPY;
        }

        while (wall < MAPY) {   // On teste tous les murs verticaux pour savoir si le rayon en touche un
            mx = (int)(rayX) >> 6;
            my = (int)(rayY) >> 6;
            mp = my * MAPX + mx;
            if (mp > 0 && mp < MAPX * MAPY && map[mp] == 1) {    // Touché !
                disV = cos(degToRad(rayAngle)) * (rayX - playerX) - sin(degToRad(rayAngle)) * (rayY - playerY);
                break;
            }
            else {    // Pas de colision, on va voir à la prochaine intersection verticale
                rayX += xOffset;
                rayY += yOffset;
                wall += 1;
            }
        }
        verticalRayX = rayX;
        verticalRayY = rayY;

        //----------Rayons horizontaux----------
        wall = 0;
        disH = 100000;
        Tan = 1.0 / Tan;
        if (sin(degToRad(rayAngle)) > 0.001) {  // On regarde en haut
            rayY = (((int)playerY >> 6) << 6) - 0.0001;
            rayX = (playerY - rayY) * Tan + playerX;
            yOffset = -64;
            xOffset = -yOffset * Tan;
        }
        else if (sin(degToRad(rayAngle)) < -0.001) { // On regarde en bas
            rayY = (((int)playerY >> 6) << 6) + 64;
            rayX = (playerY - rayY) * Tan + playerX;
            yOffset = 64;
            xOffset = -yOffset * Tan;
        }
        else {  // On regarde pile à droite ou à gauche, on ne toucheras donc jamais un mur horizontal
            rayX = playerX;
            rayY = playerY;
            wall = MAPX;
        }

        while (wall < MAPX) {   // On teste tous les murs horizontaux pour savoir si le rayon en touche un
            mx = (int)(rayX) >> 6;
            my = (int)(rayY) >> 6;
            mp = my * MAPX + mx;
            if (mp > 0 && mp < MAPX * MAPY && map[mp] == 1) {   // Touché !
                disH = cos(degToRad(rayAngle)) * (rayX - playerX) - sin(degToRad(rayAngle)) * (rayY - playerY);
                break;
            }
            else {  // Pas de colision, on va voir à la prochaine intersection horizontale
                rayX += xOffset;
                rayY += yOffset;
                wall += 1;
            }
        }

        glColor3f(0.8, 0, 0);
        if (disV < disH) {  // On a touché un mur vertical en premier
            rayX = verticalRayX;
            rayY = verticalRayY;
            disH = disV;
            glColor3f(0.6, 0, 0);
        }

        // On affiche les rayons en 2D sur la minimap
        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2f(playerX/ minimapTileSizeRatioX, playerY/ minimapTileSizeRatioY);
        glVertex2f(rayX/ minimapTileSizeRatioX, rayY/ minimapTileSizeRatioY);
        glEnd();
        //cout << rayX << " - " << rayY << '\n';

        // On règle le problème du "fisheye": les murs en face de nous nous apparaissent
        // complètement distordus car sur les cotés les rayons sont plus long c'est pourquoi en veut tout réaplanir
        int ca = FixAng(playerAngle - rayAngle);
        disH = disH * cos(degToRad(ca));

        // On prépare l'affichage des lignes verticales représentant les rayons pour le joueur
        int lineH = (TILESIZE * 320) / (disH);
        if (lineH > H) {  // On indique une limite de taille pour les lignes
            lineH = H;
        }
        lineOff = H / 2 - lineH / 2; // On centre les lignes sur l'axe vertical

        // On affiche les lignes verticales
        glLineWidth(1);
        glBegin(GL_QUADS); // Quadrilatère
        glVertex2f(rayNb * lineWidth + 512, lineOff);  // 1er rayon en bas à gauche
        glVertex2f((rayNb + 1) * lineWidth + 512, lineOff);    // 2ème rayon en bas à droite
        glVertex2f((rayNb + 1) * lineWidth + 512, lineH + lineOff);    // 3ème en haut à droite
        glVertex2f(rayNb * lineWidth + 512, lineH + lineOff);  // 4ème rayon en haut à gauche
        glEnd();

        rayAngle = FixAng(rayAngle - 1 / quality);    // On change l'angle pour le prochain rayon
    }
}


void init() {
    /*
    Fonction exécutée au démarrage mettant en place l'environnement pour le joueur
    */

    glClearColor(0.3, 0.3, 0.3, 0); // On met la couleur du fond en gris
    gluOrtho2D(0, L, H, 0); // On définit une surface pour afficher dessus
    // Position et angle initial du joueur
    playerX = 150;
    playerY = 400;
    playerAngle = 0;
    // Directions pour la minimap
    playerMoveDirectionX = cos(degToRad(playerAngle));
    playerMoveDirectionY = -sin(degToRad(playerAngle));
}

void display() {
    /*
    Fonction exécutée à chaque rafraîchissement d'image
    */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // On efface complètement l'écran en laissant la couleur de fond 
    drawMap2D();    // On affiche la minimap
    drawPlayer2D(); // On affiche le joueur sur la minimap
    drawRays();   // On affiche la vision "3d"
    glutSwapBuffers();  // On échange les buffers pour afficher sur l'écran ce que l'on vient de render
}

int main(int argc, char* argv[]) {
    /*
    Fonction principale se répétant à l'infini
    */

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(L, H);  // On initialise une fenêtre de L largeur et H hauteur
    glutCreateWindow("Raycaster");  // On crée une fenêtre avec un titre
    init();
    glutDisplayFunc(display);   // On définit la fonction à appeler quand il faut rafraîchir l'écran
    glutKeyboardFunc(Buttons);  // On définit la fonction à appeler quand on appuie sur le clavier
    glutMainLoop();     // On laisse GLUT gérer la boucle principale
}

