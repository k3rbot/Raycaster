#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <gl/glut.h>
#include <math.h>
#include <iostream>

// Les deux fichiers sont de la forme const int [] = {RGB,RGB,RGB};
#include "resources/wallTexture.pnm"    // Tableau contenant la texture d'un mur
#include "resources/doorTexture.pnm"    // Tableau contenant la texture d'une porte

using namespace std;

#define MAPX  16                        // Largeur de la carte (en tuiles)
#define MAPY  16                        // Hauteur de la carte (en tuiles)
#define TILESIZE 64                     // Taille des tuiles de la carte
#define TEXTUREWIDTH 32                 // Largeur des textures (carrés)
#define ONEDEG 0.0174532925199432957    // Un radian

const int L = 1090;                                             // Largeur de la fenêtre
const int H = 1024;                                             // Hauteur de la fenêtre
int FOV = 60;                                                   // Champ de vision en degré
int quality = 10;                                               // Nombre de rayons par degré
float frame, frame2, fps;                                       // Permet le calcul des images par seconde et la vitesse de déplacement du joueur
const float focalLength = 1 / (tan((FOV / 2) * M_PI / 180));    // https://jsantell.com/3d-projection/


// Caractéristiques du joueur
typedef struct {
    float x, y, directionX, directionY, angle;
} Players;
Players Player;


// Structure conservant les infos des touches pressées
typedef struct {
    int z, q, s, d;
} Buttonkeys;
Buttonkeys Keys;


class Sprite {
public:
    int state;
    int map;
    int x, y, z;

    void draw() {
        float sx = x - Player.x;
        float sy = y - Player.y;
        float sz = z;

        float CS = cos(Player.angle);
        float SN = sin(Player.angle);
        float a = sy * CS + sx * SN;
        float b = sx * CS - sy * SN;
        sx = a;
        sy = b;

        sx = (sx * 109.4 / sy) + (L / 8 / 2.0);
        sy = (sz * 109.4 / sy) + (H / 8 / 2.0);

        glPointSize(8);
        glColor3f(1, 1, 0);
        glBegin(GL_POINTS);
        glVertex2f(sx * 8, sy * 8);
        glEnd();
    }
};
Sprite KeySprite;


// Carte du monde (0 = vide, 1 = mur, 2 = porte)
const int map[] = {
    1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,0,0,0,1,0,1,0,0,0,1,0,0,0,1,
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

    glutPostRedisplay();
}


/*
Cette fonction calcule la position des murs vis-à-vis du joueur et affiche les murs à l'écran en conséquence.
Pour ce faire, elle envoie des rayons virtuels, qui lorsque ils touchent un mur permettent de déterminer sa distance relative au joueur.
*/
void drawWalls() {
    int rayNb, wall, side, nbRays;
    unsigned int tileX, tileY, tileId;
    float verticalRayX, verticalRayY, rayX, rayY, xOffset, yOffset, disV, disH, lineOffset, Tan, lineWidth;
    double rayAngle;

    rayAngle = overflowAngle(Player.angle + (FOV / 2 * ONEDEG));    // On trace le premier rayon en commençant à gauche et à un angle dépendant du champs de vision
    nbRays = FOV * quality;                                         // Nombre de rayons à tracer en fonction du champs de vision et du niveau de détail voulu
    lineWidth = (float)L / nbRays;                                  // Epaisseur d'une ligne pour l'affichage "3d" dépendant du nombre de lignes à tracer

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
            // On accède à l'adresse contenant la texture désirée pour l'afficher.
            // Pour ce faire on accède à l'adresse de la première valeur du tableau contenant la texture grâce à notre tableau de pointeurs.
            // Ensuite on ajoute le nombre correspondant à l'indice du pixel de la texture dans le tableau pour atteindre son adresse et on utilise l'opérateur * pour accéder à la variable par sa mémoire.
            // Le compileur s'occupe de multiplier l'indice par la taille d'un élément, ici 4 octets pour un tableau int[]
            int red = *(texturesArray[texture] + (((int)textureY * TEXTUREWIDTH + (int)textureX) * 3)) * shading;
            int green = *(texturesArray[texture] + (((int)textureY * TEXTUREWIDTH + (int)textureX) * 3 + 1)) * shading;
            int blue = *(texturesArray[texture] + (((int)textureY * TEXTUREWIDTH + (int)textureX) * 3 + 2)) * shading;
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
    gluOrtho2D(0, L, H, 0); // On définit une surface pour afficher dessus
    // Position et angle initial du joueur
    Player.x = 150;
    Player.y = 400;
    Player.angle = M_PI;
    // Valeurs d'un déplacement
    Player.directionX = cos(Player.angle);
    Player.directionY = -sin(Player.angle);

    KeySprite.state = 1;
    KeySprite.x = 1.5 * TILESIZE;
    KeySprite.y = 3 * TILESIZE;
    KeySprite.z = 20;
}


/*
Fonction exécutée à chaque rafraîchissemen
*/
void display() {
    

    // Récuperation du nombre d'images par seconde
    frame2 = glutGet(GLUT_ELAPSED_TIME);
    fps = frame2 - frame;
    frame = glutGet(GLUT_ELAPSED_TIME);

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

    // Si on recule
    if (Player.directionX < 0) {
        xOffset = -20;
    }
    else {
        xOffset = 20;
    }
    // Si on recule
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // On efface complètement l'écran en laissant la couleur de fond

    // Affichage du ciel et du sol (on fait deux rectangles)
    glColor3f(0, 0.5, 0.5);
    glBegin(GL_QUADS);
    glVertex2i(0, 0);
    glVertex2i(L, 0);
    glVertex2i(L, H / 2);
    glVertex2i(0, H / 2);
    glEnd();
    glColor3f(0.2, 0.2, 0.2);
    glBegin(GL_QUADS);
    glVertex2i(0, H / 2);
    glVertex2i(L, H / 2);
    glVertex2i(L, H);
    glVertex2i(0, H);

    glEnd();
    drawWalls();            // On affiche la vision "3d"
    glutPostRedisplay();
    KeySprite.draw();
    glutSwapBuffers();      // On envoie l'image au processeur graphique
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
    glutCreateWindow("Raycaster");                  // On crée une fenêtre avec un titre
    init();
    glutDisplayFunc(display);                       // On définit la fonction à appeler quand il faut rafraîchir l'écran
    glutKeyboardFunc(buttonDown);                   // On indique la fonction à appeler quand on appuie sur le clavier
    glutKeyboardUpFunc(buttonUp);                   // Et celle quand on relache un bouton
    glutReshapeFunc(resize);
    glutMainLoop();                                 // On laisse GLUT gérer la boucle principale
}

