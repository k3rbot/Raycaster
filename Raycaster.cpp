#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>
using namespace std;

#define MAPX  16      // Largeur de la carte (en tuiles)
#define MAPY  16      // Hauteur de la carte (en tuiles)
#define TILESIZE 64      // Taille des tuiles de la carte
#define TEXTUREWIDTH 32     // Largeur des textures (carrés)
#define ONEDEG 0.0174532925199432957 // Un degré en radians (pour éviter des calculs inutiles)

const int L = 1024;     // Largeur de la fenêtre
const int H = 512;  // Hauteur de la fenêtre
const float minimapTileSizeRatioX = MAPX / 8;   // Ratio pour avoir la minimap toujours de la même taille pour déterminer la taille des tuiles à afficher
const float minimapTileSizeRatioY = MAPY / 8;
int FOV = 60 ;  // Champ de vision
float quality = 10.0;

typedef struct {    // Gestion des entrées du joueur
    int z, q, s, d;
} Buttonkeys;
Buttonkeys Keys;

const int map[] = {          // Carte
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

const int wallTexture[] = {
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,

    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,

    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 1, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0,    1, 1, 1, 1, 1, 1, 1, 1,    0, 0, 0, 0, 0, 0, 0, 0
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


double overflowAngle(double angle) {
    /*
    Remise à zéro de l'angle quand on atteint les 2π radians
    */

    if (angle > 2 * M_PI) {
        angle -= 2 * M_PI;
    }
    else if (angle < 0) {
        angle += 2 * M_PI;
    }
    return angle;
}

float playerX, playerY, playerDirectionX, playerDirectionY, playerAngle;

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
    glVertex2i(playerX / minimapTileSizeRatioX + playerDirectionX * 20, playerY / minimapTileSizeRatioY + playerDirectionY * 20);
    glEnd();
}


void buttonDown(unsigned char key, int x, int y) {
    if (key == 'z') {
        Keys.z = 1;
    }
    else if (key == 'q') {
        Keys.q = 1;
    }
    else if (key == 's') {
        Keys.s = 1;
    }
    else if (key == 'd') {
        Keys.d = 1;
    }

    glutPostRedisplay();
}


void buttonUp(unsigned char key, int x, int y) {
    if (key == 'z') {
        Keys.z = 0;
    }
    else if (key == 'q') {
        Keys.q = 0;
    }
    else if (key == 's') {
        Keys.s = 0;
    }
    else if (key == 'd') {
        Keys.d = 0;
    }

    glutPostRedisplay();
}


float distance(float ax, float ay, float bx, float by, float ang) {
    /*
    Calcul de la distance entre deux points
    */
    return cos(ang) * (bx - ax) - sin(ang) * (by - ay);
}

void drawRays() {
    /*
    Affichage des rayons sur la minimap et la "3d"
    */

    int mp, rayNb, wall, side, nbRays;
    float verticalRayX, verticalRayY, rayX, rayY, xOffset, yOffset, mx, my, disV, disH, lineOffset, Tan, lineWidth;
    double rayAngle;

    rayAngle = overflowAngle(playerAngle + (FOV / 2 * ONEDEG));     // On trace le premier rayon en commençant à gauche et à un angle dépendant du champs de vision
    nbRays = FOV * quality;    // Nombre de rayons à tracer en fonction du champs de vision et du niveau de détail voulu
    lineWidth = 512.0 / nbRays;  // Epaisseur d'une ligne pour l'affichage "3d" dépendant du nombre de lignes à tracer

    // Rayons
    for (rayNb = 0; rayNb < nbRays; rayNb++) {
        //----------Rayons verticaux----------
        wall = 0;
        side = 0;
        disV = 100000;
        Tan = tan(rayAngle);
        if (cos(rayAngle) > 0.001) {  // On regarde à gauche
            rayX = (((int)playerX >> 6) << 6) + 64;
            rayY = (playerX - rayX) * Tan + playerY;
            xOffset = 64;
            yOffset = -xOffset * Tan;
        }
        else if (cos(rayAngle) < -0.001) {   // On regarde à droite
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
                disV = cos(rayAngle) * (rayX - playerX) - sin(rayAngle) * (rayY - playerY);
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
        if (sin(rayAngle) > 0.001) {  // On regarde en haut
            rayY = (((int)playerY >> 6) << 6) - 0.0001;
            rayX = (playerY - rayY) * Tan + playerX;
            yOffset = -64;
            xOffset = -yOffset * Tan;
        }
        else if (sin(rayAngle) < -0.001) { // On regarde en bas
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
                disH = cos(rayAngle) * (rayX - playerX) - sin(rayAngle) * (rayY - playerY);
                break;
            }
            else {  // Pas de colision, on va voir à la prochaine intersection horizontale
                rayX += xOffset;
                rayY += yOffset;
                wall += 1;
            }
        }

        float shading = 1;
        glColor3f(0.8, 0, 0);
        if (disV < disH) {  // On a touché un mur vertical en premier
            rayX = verticalRayX;
            rayY = verticalRayY;
            disH = disV;
            glColor3f(0.6, 0, 0);
            shading = 0.8;
        }

        // On affiche les rayons en 2D sur la minimap
        glLineWidth(1);
        glBegin(GL_LINES);
        glVertex2f(playerX/ minimapTileSizeRatioX, playerY/ minimapTileSizeRatioY);
        glVertex2f(rayX/ minimapTileSizeRatioX, rayY/ minimapTileSizeRatioY);
        glEnd();
        //cout << rayX << " - " << rayY << '\n';

        // On règle le problème du "fisheye": les murs en face de nous nous apparaissent
        // complètement distordus car sur les cotés les rayons sont plus long
        double ca = overflowAngle(playerAngle - rayAngle);
        disH = disH * cos(ca);

        // On prépare l'affichage des lignes verticales représentant les rayons pour le joueur
        int lineH = (TILESIZE * H) / disH;
        float textureY_step = (float)TEXTUREWIDTH / lineH;
        float textureOffset = 0;
        if (lineH > H) {  // On indique une limite de taille pour les lignes
            textureOffset = (lineH - H) / 2.0;    // On veut centrer la texture sur le mur qu'on regarde quand on est très proche
            lineH = H;
        }
        lineOffset = (H - lineH) / 2.0; // On centre les lignes sur l'axe vertical

        // On affiche les textures qui forment les murs "3d"
        float textureY = textureY_step * textureOffset;     // On adapte la taille de la texture à la taille du mur
        float textureX;
        if (shading == 1) {  // On affiche un mur vertical
            textureX = (int)(rayX / 2.0) % TEXTUREWIDTH;  // On récupère la bonne "bande" verticale de la texture à cet endroit du mur
            if (rayAngle > M_PI) {  // On regarde vers le bas, on doit donc inverser les textures
                textureX = TEXTUREWIDTH - textureX - 1;
            }
        }
        else {  // On affiche un mur horizontal
            textureX = (int)(rayY / 2.0) % TEXTUREWIDTH;
            if (rayAngle > M_PI / 2 && rayAngle < (3.0 / 2.0 * M_PI)) {   // On regarde à droite, on doit donc inverser les textures
                textureX = TEXTUREWIDTH - textureX - 1;
            }
        }
        glPointSize(lineWidth);
        glBegin(GL_POINTS);
        for (int linePixelY = 0; linePixelY < lineH; linePixelY++) {
            float color = wallTexture[(int)textureY * TEXTUREWIDTH + (int)textureX] * shading;
            glColor3f(color, color, color);
            glVertex2f(rayNb * lineWidth + L / 2, linePixelY + lineOffset);
            textureY += textureY_step;        
        };
        glEnd();

        rayAngle = overflowAngle(rayAngle - (ONEDEG / quality));    // On change l'angle pour le prochain rayon
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
    playerAngle = M_PI;
    // Directions pour la minimap
    playerDirectionX = cos(playerAngle);
    playerDirectionY = -sin(playerAngle);
}

float frame, frame2, fps;

void display() {
    /*
    Fonction exécutée à chaque rafraîchissement d'image
    */
    
    // Récuperation du nombre d'images par seconde
    frame2 = glutGet(GLUT_ELAPSED_TIME);
    fps = frame2 - frame;
    frame = glutGet(GLUT_ELAPSED_TIME);
    
    // Gestion des inputs du joueur
    if (Keys.q == 1) {   // On tourne la vision du joueur vers la gauche
        playerAngle += 0.003 * fps;  // La vitesse de mouvement dépends maintenant de la fréquence d'image
        playerAngle = overflowAngle(playerAngle);
        playerDirectionX = cos(playerAngle);
        playerDirectionY = -sin(playerAngle);
    }
    if (Keys.d == 1) {   // On tourne la vision du joueur vers la droite
        playerAngle -= 0.003 * fps;
        playerAngle = overflowAngle(playerAngle);
        playerDirectionX = cos(playerAngle);
        playerDirectionY = -sin(playerAngle);
    }

    int xOffset = 0;    // Marge contre le mur
    int yOffset = 0;

    if (playerDirectionX < 0) { // Si on est en bas de la map
        xOffset = -20;
    }
    else {
        xOffset = 20;
    }
    if (playerDirectionY < 0) {
        yOffset = -20;
    }
    else {
        yOffset = 20;
    }

    // On veut savoir si le block situé devant nous est un mur on enregistre donc notre position
    // dans la map avec le décalage
    int gridPlayerPosX = playerX / 64.0;
    int gridPlayerPosX_add_xOffset = (playerX + xOffset) / 64.0;
    int gridPlayerPosX_sub_xOffset = (playerX - xOffset) / 64.0;
    int gridPlayerPosY = playerY / 64.0;
    int gridPlayerPosY_add_yOffset = (playerY + yOffset) / 64.0;
    int gridPlayerPosY_sub_yOffset = (playerY - yOffset) / 64.0;

    if (Keys.z == 1) {   // On avance le joueur
        // Si la position du joueur dans la map avec le décalage ne correspond pas à un mur on peut avancer
        if (map[gridPlayerPosY * MAPX + gridPlayerPosX_add_xOffset] == 0) {
            playerX += playerDirectionX * 0.15 * fps;
        }
        if (map[gridPlayerPosY_add_yOffset * MAPX + gridPlayerPosX] == 0) {
            playerY += playerDirectionY * 0.15 * fps;
        }
    }
    if (Keys.s == 1) {   // On recule le joueur
        if (map[gridPlayerPosY * MAPX + gridPlayerPosX_sub_xOffset] == 0) {
            playerX -= playerDirectionX * 0.15 * fps;
        }
        if (map[gridPlayerPosY_sub_yOffset * MAPX + gridPlayerPosX] == 0) {
            playerY -= playerDirectionY * 0.15 * fps;
        }
    }
    glutPostRedisplay();

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
    glutKeyboardFunc(buttonDown);  // On indique la fonction à appeler quand on appuie sur le clavier
    glutKeyboardUpFunc(buttonUp);   // Et celle quand on relache un bouton
    glutMainLoop();     // On laisse GLUT gérer la boucle principale
}

