#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

// Les deux fichiers sont de la forme const int [] = {RGB,RGB,RGB};
#include "resources/wallTexture.pnm"    // Tableau contenant la texture d'un mur
#include "resources/doorTexture.pnm"    // Tableau contenant la texture d'une porte
#include "resources/keyTexture.pnm"     // Tableau contenant la texture d'une clé
#include "resources/end.pnm"

using namespace std;

#define MAPX  16                        // Largeur de la carte (en tuiles)
#define MAPY  16                        // Hauteur de la carte (en tuiles)
#define TILESIZE 64                     // Taille des tuiles de la carte
#define TEXTUREWIDTH 32                 // Largeur des textures (carrés)
#define ONEDEG 0.0174532925199432957    // Un radian

const int L = 1090;                                                     // Largeur de la fenêtre
const int H = 1024;                                                     // Hauteur de la fenêtre

const int FOV = 60;                                                     // Champ de vision en degré
const int quality = 10;
int depth[FOV * quality];                                               // Taille de chaque rayon
const int nbRays = FOV * quality;                                       // Nombre de rayons par degré
const float lineWidth = (float)L / nbRays;                              // Epaisseur d'une ligne pour l'affichage "3d" dépendant du nombre de lignes à tracer

float frame, frame2, fps;                                               // Permet le calcul des images par seconde et la vitesse de déplacement du joueur

const float focalLength = 1 / tan(FOV / 2 * M_PI / 180) * 180 / M_PI;    // https://jsantell.com/3d-projection/

int gameState = 1;                                                      // Différents états du jeu : 1 = en train de jouer, 2 : écran de fin


// Carte du monde (0 = vide, 1 = mur, 2 = porte)
const int map[] = {
    1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,2,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,
    1,0,0,0,0,1,1,1,1,1,1,1,0,0,0,1,
    1,0,0,0,0,1,0,0,1,0,0,0,0,0,0,1,
    1,0,1,0,0,1,0,0,1,0,1,0,0,0,0,1,
    1,0,1,0,0,1,0,0,0,0,1,0,0,0,0,1,
    1,0,1,0,0,1,1,1,1,1,1,1,0,0,0,1,
    1,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

// Tableau de pointeurs contenant les adresses des tableaux contenant les textures du jeu
const int* texturesArray[2] = {
        wallTexture,
        doorTexture
};


typedef struct {
    char text[30];
    int duration;
} Messages;
Messages Message;

// Caractéristiques du joueur
typedef struct {
    float x, y, directionX, directionY, angle;
} Players;
Players Player;

// Structure conservant les infos des touches préssées
typedef struct {
    int z, q, s, d, e;
} Buttonkeys;
Buttonkeys Keys;

class Sprite {
public:
    bool visible;
    int map;
    int x, y, z;

    void draw() {
        if (visible) {
            if (Player.x < x + 100 && Player.x > x - 100 && Player.y < y + 100 && Player.y > y - 100) {
                if (Message.duration == 0) {
                    strcpy_s(Message.text, "Press e to pick up the key");
                    Message.duration = 20;
                }
            }

            float spriteXtoPlayer = x - Player.x;   // Distance du sprite relative à celle du joueur
            float spriteYtoPlayer = y - Player.y;

            // On fait tourner le sprite autour de son centre pour toujours le voir
            float cosAngle = cos(Player.angle);
            float sinAngle = sin(Player.angle);
            float tempX = spriteYtoPlayer * cosAngle + spriteXtoPlayer * sinAngle;
            float zDepth = spriteXtoPlayer * cosAngle - spriteYtoPlayer * sinAngle;
            spriteXtoPlayer = tempX;
            spriteYtoPlayer = zDepth;

            // transformation des coordonées sur la map en coordonnées sur l'écran (matrice de projection 3d: https://jsantell.com/3d-projection/)
            spriteXtoPlayer = (spriteXtoPlayer * focalLength / spriteYtoPlayer) + (L / 8 / 2.0);
            spriteYtoPlayer = (z * focalLength / spriteYtoPlayer) + (H / 8 / 2.0);

            glPointSize(1);
            if (spriteXtoPlayer > 0 && spriteXtoPlayer * 8 < L) {
                int scale = TEXTUREWIDTH * 320 / zDepth;
                float textureX = 0;
                float textureY = 0;
                float textureX_step = (float)TEXTUREWIDTH / scale;
                float textureY_step = textureX_step;
                for (int textureX_screen = -scale / 2; textureX_screen < scale / 2; textureX_screen++) {
                    textureY = 0;
                    if (zDepth - 10 < depth[int((spriteXtoPlayer * 8 + textureX_screen) / lineWidth)]) {
                        for (int textureY_screen = 0; textureY_screen < scale; textureY_screen++) {
                            int pixel = ((int)textureY * TEXTUREWIDTH + (int)textureX) * 3;
                            int red = keyTexture[pixel];
                            int green = keyTexture[pixel + 1];
                            int blue = keyTexture[pixel + 2];
                            if (green != 255 || red != 0 || blue != 0) {
                                glColor3ub(red, green, blue);
                                glBegin(GL_POINTS);
                                glVertex2f(spriteXtoPlayer * 8 + textureX_screen, spriteYtoPlayer * 8 + textureY_screen);
                                glEnd();
                            }
                            textureY += textureY_step;
                        }
                    }
                    textureX += textureX_step;
                }
            }
        }
    }
};
Sprite KeySprite;


/*
Mise en application du cercle trigonométrique
*/
double overflowAngle(double angle) {

    if (angle > 2 * M_PI) {
        angle -= 2 * M_PI;
    }
    else if (angle < 0) {
        angle += 2 * M_PI;
    }
    return angle;
}

void displayText() {
    if (Message.duration > 0) {
        glColor3ub(255, 255, 255);
        glRasterPos2i(10, 30);
        int len = strlen(Message.text);
        for (int i = 0; i < len; i++) {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, Message.text[i]);
        }
        Message.duration--;
    }
}

/*
Cette fonction permet de conctrôler le joueur lorsque ce dernier appuie sur une touche
*/
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
    else if (key == 'e') {
        Keys.e = 1;
    }

    glutPostRedisplay();
}

/*
Cette fonction permet de conctrôler le joueur lorsque ce dernier relâche sur une touche
*/
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
    else if (key == 'e') {
        Keys.e = 0;
    }

    glutPostRedisplay();
}


/*
Cette fonction calcule la position des murs vis-à-vis du joueur et affiche les murs à l'écran en conséquence.
Pour ce faire, elle envoie des rayons virtuels, qui lorsque ils touchent un mur permettent de déterminer sa distance relative au joueur.
*/
void drawWalls() {
    int rayNb, wall, side;
    unsigned int tileX, tileY, tileId;
    float verticalRayX, verticalRayY, rayX, rayY, xOffset, yOffset, disV, disH, lineOffset, Tan;
    double rayAngle;

    rayAngle = overflowAngle(Player.angle + (FOV / 2 * ONEDEG));    // On trace le premier rayon en commençant à gauche et à un angle dépendant du champs de vision

    // Rayons
    for (rayNb = 0; rayNb < nbRays; rayNb++) {
        // Envoi du rayon croisant les lignes verticales
        wall = 0;
        side = 0;
        disV = 100000;
        Tan = tan(rayAngle);

        // On regarde à droite
        if (cos(rayAngle) > 0.001) {
            // On fait des bitshifts pour pour arrondir la position du joueur à la ligne verticale sur sa gauche,
            // puis on ajoute 64 pour obtenir l'abscisse de la ligne verticale à sa droite.
            rayX = (((int)Player.x >> 6) << 6) + 64;
            rayY = (Player.x - rayX) * Tan + Player.y;
            xOffset = 64;
            yOffset = -xOffset * Tan;
        }
        // On regarde à gauche
        else if (cos(rayAngle) < -0.001) {
            // On fait des bitshifts pour pour arrondir la position du joueur à la ligne verticale sur sa gauche,
            // puis on retire une toute petit valeur pour tomber sur le bloc de gauche.
            rayX = (((int)Player.x >> 6) << 6) - 0.0001;
            rayY = (Player.x - rayX) * Tan + Player.y;
            xOffset = -64;
            yOffset = -xOffset * Tan;
        }
        // On regarde à la verticale donc le rayon ne peut pas toucher une ligne verticale
        else {
            rayX = Player.x;
            rayY = Player.y;
            wall = MAPY;
        }

        // On teste tous les murs verticaux pour savoir si le rayon en touche un
        while (wall < MAPY) {
            tileX = (unsigned int)rayX >> 6;
            tileY = (unsigned int)rayY >> 6;
            tileId = tileY * MAPX + tileX;  // Position du mur dans la carte
            // Touché !
            if (tileId > 0 && tileId < MAPX * MAPY && map[tileId] > 0) {
                // Calcule de la distance entre le joueur et le point de collision du rayon
                disV = cos(rayAngle) * (rayX - Player.x) - sin(rayAngle) * (rayY - Player.y);
                break;
            }
            // Pas de colision, on va voir à la prochaine intersection verticale
            else {
                rayX += xOffset;
                rayY += yOffset;
                wall += 1;
            }
        }
        // On sauvegarde les variables pour plus tard
        verticalRayX = rayX;
        verticalRayY = rayY;

        // Envoi du rayon croisant les lignes horizontales
        // Les étapes sont les même que pour le rayon vertical
        wall = 0;
        disH = 100000;
        Tan = 1.0 / Tan;
        if (sin(rayAngle) > 0.001) {
            rayY = (((int)Player.y >> 6) << 6) - 0.0001;
            rayX = (Player.y - rayY) * Tan + Player.x;
            yOffset = -64;
            xOffset = -yOffset * Tan;
        }
        else if (sin(rayAngle) < -0.001) {
            rayY = (((int)Player.y >> 6) << 6) + 64;
            rayX = (Player.y - rayY) * Tan + Player.x;
            yOffset = 64;
            xOffset = -yOffset * Tan;
        }
        else {
            rayX = Player.x;
            rayY = Player.y;
            wall = MAPX;
        }

        while (wall < MAPX) {
            tileX = (unsigned int)rayX >> 6;
            tileY = (unsigned int)rayY >> 6;
            tileId = tileY * MAPX + tileX;  // Position du mur dans la carte
            if (tileId > 0 && tileId < MAPX * MAPY && map[tileId] > 0) {
                disH = cos(rayAngle) * (rayX - Player.x) - sin(rayAngle) * (rayY - Player.y);
                break;
            }
            else {
                rayX += xOffset;
                rayY += yOffset;
                wall += 1;
            }
        }


        float shading = 1;
        glColor3f(0.8, 0, 0);
        // On décide si il faut prendre compte du rayon vertical ou horizontal (on prend le plus court) pour afficher le mur
        if (disV < disH) {
            // Puisque les valeurs du rayons horizontal sont en mémoire, on ne les changes que si le rayon vertical est plus avantageux
            rayX = verticalRayX;
            rayY = verticalRayY;
            disH = disV;
            glColor3f(0.6, 0, 0);
            shading = 0.75;
            tileX = (unsigned int)rayX >> 6;
            tileY = (unsigned int)rayY >> 6;
            tileId = tileY * MAPX + tileX;  // Position du mur dans la carte
        }

        // On règle le problème du "fisheye": les murs en face de nous nous apparaissent
        // complètement distordus car sur les cotés les rayons sont plus long
        double ca = overflowAngle(Player.angle - rayAngle);
        disH = disH * cos(ca);

        depth[rayNb] = disH;    // On sauvegarde la distance de chaque rayon pour vérifier si le sprite est derrière un mur ou non

        // On prépare l'affichage des lignes verticales représentant les rayons pour le joueur
        int lineH = (TILESIZE * H) / disH;
        float textureY_step = (float)TEXTUREWIDTH / lineH;
        float textureOffset = 0;
        // On indique une limite de taille pour les lignes
        if (lineH > H) {
            // On veut centrer la texture sur le mur qu'on regarde quand on est très proche
            textureOffset = (lineH - H) / 2.0;
            lineH = H;
        }
        // On centre les lignes sur l'axe vertical
        lineOffset = (H - lineH) / 2.0;

        // On affiche les textures qui forment les murs "3d"
        float textureY = textureY_step * textureOffset;     // On adapte la taille de la texture à la taille du mur
        float textureX;
        // On affiche un mur vertical
        if (shading == 1) {
            textureX = (int)(rayX / 2.0) % TEXTUREWIDTH;    // On récupère la bonne "bande" verticale de la texture à cet endroit du mur
            // On regarde vers le bas, on doit donc inverser les textures
            if (rayAngle > M_PI) {
                textureX = TEXTUREWIDTH - textureX - 1;
            }
        }
        // On affiche un mur horizontal
        else {
            textureX = (int)(rayY / 2.0) % TEXTUREWIDTH;
            // On regarde à droite, on doit donc inverser les textures
            if (rayAngle > M_PI / 2 && rayAngle < (3.0 / 2.0 * M_PI)) {
                textureX = TEXTUREWIDTH - textureX - 1;
            }
        }


        // Parfois le le mur choisi a une valeur abérrante, ou tombe sur un espace vide, donc on prend la texture de base des murs
        int texture;
        if (tileId > 0 && tileId < (MAPX * MAPY)) {
            if (map[tileId] == 0) {
                texture = 0;
            }
            else {
                texture = map[tileId] - 1;
            }
        }
        else {
            texture = 0;
        }


        glPointSize(lineWidth);
        glBegin(GL_POINTS);

        for (int linePixelY = 0; linePixelY < lineH; linePixelY++) {
            int pixel = ((int)textureY * TEXTUREWIDTH + (int)textureX) * 3;
            int red = *(texturesArray[texture] + pixel) * shading;
            int green = *(texturesArray[texture] + (pixel + 1)) * shading;
            int blue = *(texturesArray[texture] + (pixel + 2)) * shading;
            glColor3ub(red, green, blue);
            glVertex2f(rayNb * lineWidth, linePixelY + lineOffset);
            textureY += textureY_step;
        };

        glEnd();

        rayAngle = overflowAngle(rayAngle - (ONEDEG / quality));    // On change l'angle pour le prochain rayon
    }
}


/*
Fonction exécutée au démarrage mettant en place l'environnement pour le joueur
*/
void init() {
    glClearColor(0.2, 0.2, 0.2, 0);
    // Position et angle initial du joueur
    Player.x = 150;
    Player.y = 400;
    Player.angle = M_PI;
    // Valeurs d'un déplacement
    Player.directionX = cos(Player.angle);
    Player.directionY = -sin(Player.angle);

    KeySprite.visible = 1;
    KeySprite.x = 1.5 * TILESIZE;
    KeySprite.y = 3 * TILESIZE;
    KeySprite.z = 40;
}


/*
Fonction exécutée à chaque rafraîchissemen
*/
void display() {


    // Récuperation du nombre d'images par seconde
    frame2 = glutGet(GLUT_ELAPSED_TIME);
    fps = frame2 - frame;
    frame = glutGet(GLUT_ELAPSED_TIME);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // On efface complètement l'écran en laissant la couleur de fond


    if (gameState == 1) {
        // Gestion des inputs du joueur
        // On tourne la vision du joueur vers la gauche
        if (Keys.q == 1) {
            Player.angle += 0.0015 * fps;   // La vitesse de mouvement dépends de la fréquence d'image
            Player.angle = overflowAngle(Player.angle);
            Player.directionX = cos(Player.angle);
            Player.directionY = -sin(Player.angle);
        }
        // On tourne la vision du joueur vers la droite
        if (Keys.d == 1) {
            Player.angle -= 0.0015 * fps;   // La vitesse de mouvement dépends de la fréquence d'image
            Player.angle = overflowAngle(Player.angle);
            Player.directionX = cos(Player.angle);
            Player.directionY = -sin(Player.angle);
        }

        int xOffset, yOffset;    // Marge contre les murs

        // Si on regarde vers la gauche
        if (Player.directionX < 0) {
            xOffset = -20;
        }
        else {
            xOffset = 20;
        }
        // Si on regarde vers le haut
        if (Player.directionY < 0) {
            yOffset = -20;
        }
        else {
            yOffset = 20;
        }


        // On veut savoir si le block situé devant nous est un mur on enregistre donc notre position dans la map avec le décalage
        int mapPlayerPosX = Player.x / TILESIZE;
        int mapPlayerPosX_add_xOffset = (Player.x + xOffset) / TILESIZE;
        int mapPlayerPosX_sub_xOffset = (Player.x - xOffset) / TILESIZE;
        int mapPlayerPosY = Player.y / TILESIZE;
        int mapPlayerPosY_add_yOffset = (Player.y + yOffset) / TILESIZE;
        int mapPlayerPosY_sub_yOffset = (Player.y - yOffset) / TILESIZE;


        // On avance le joueur
        if (Keys.z == 1) {
            // Si la position du joueur dans la map avec le décalage ne correspond pas à un mur on peut avancer
            if (map[mapPlayerPosY * MAPX + mapPlayerPosX_add_xOffset] == 0) {
                Player.x += Player.directionX * 0.15 * fps;
            }
            if (map[mapPlayerPosY_add_yOffset * MAPX + mapPlayerPosX] == 0) {
                Player.y += Player.directionY * 0.15 * fps;
            }
        }
        // On recule le joueur
        if (Keys.s == 1) {
            // Si la position du joueur dans la map avec le décalage ne correspond pas à un mur on peut reculer
            if (map[mapPlayerPosY * MAPX + mapPlayerPosX_sub_xOffset] == 0) {
                Player.x -= Player.directionX * 0.15 * fps;
            }
            if (map[mapPlayerPosY_sub_yOffset * MAPX + mapPlayerPosX] == 0) {
                Player.y -= Player.directionY * 0.15 * fps;
            }
        }

        if (Keys.e == 1) {
            if (Player.x < KeySprite.x + 100 && Player.x > KeySprite.x - 100 && Player.y < KeySprite.y + 100 && Player.y > KeySprite.y - 100 && KeySprite.visible) {
                KeySprite.visible = false;
                strcpy_s(Message.text, "You picked up the key");
                Message.duration = 250;
            }
            else if (!KeySprite.visible && (map[int((Player.y + 5 * yOffset) / TILESIZE) * MAPX + mapPlayerPosX] == 2 || map[mapPlayerPosY * MAPX + int((Player.x + 5 * xOffset) / TILESIZE)] == 2)) {
                gameState = 2;
            }
        }
        if (!KeySprite.visible && (map[int((Player.y + 100 * Player.directionY) / TILESIZE) * MAPX + int((Player.x + 100 * Player.directionX) / TILESIZE)] == 2)) {
            strcpy_s(Message.text, "Press e to open the door");
            Message.duration = 20;
        }

        // Affichage du ciel
        glColor3f(0, 0.5, 0.5);
        glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(L, 0);
        glVertex2i(L, H / 2);
        glVertex2i(0, H / 2);
        glEnd();

        drawWalls();   // On affiche la vision "3d"
        KeySprite.draw();
        displayText();
    }
    else if (gameState == 2) {
        glClearColor(0, 0, 0, 0);
        glPointSize(1);
        for (int y = 0; y < 1024; y++) {
            for (int x = 0; x < 1024; x++) {
                int pixel = (y * 1024 + x) * 3;
                int red = endScreen[pixel];
                int green = endScreen[pixel + 1];
                int blue = endScreen[pixel + 2];
                glColor3ub(red, green, blue);
                glBegin(GL_POINTS);
                glVertex2i(x + 33, y);
                glEnd();
            }
        }
    }

    glutPostRedisplay();
    glutSwapBuffers();  // On échange les buffers pour afficher sur l'écran ce que l'on vient de render
}

/*
On garde les proportion parfaites de notre jeu
*/
void resize(int l, int h) {
    glutReshapeWindow(L, H);
};

/*
Point d'entrée du programme
*/
int main(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(L, H);                       // On initialise une fenêtre de L largeur et H hauteur
    glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH) / 2 - L / 2, glutGet(GLUT_SCREEN_HEIGHT) / 2 - H / 2);
    glutCreateWindow("Raycaster");                  // On crée une fenêtre avec un titre
    gluOrtho2D(0, L, H, 0);                         // On définit une surface pour afficher dessus
    init();
    glutDisplayFunc(display);                       // On définit la fonction à appeler quand il faut rafraîchir l'écran
    glutKeyboardFunc(buttonDown);                   // On indique la fonction à appeler quand on appuie sur le clavier
    glutKeyboardUpFunc(buttonUp);                   // Et celle quand on relache un bouton
    glutReshapeFunc(resize);
    glutMainLoop();                                 // On laisse GLUT gérer la boucle principale
}

