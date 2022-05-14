# Raycaster

Ce projet est une implémentation minimaliste d'une technique de rendu 3D de jeu vidéo appelée Raycasting, originellement introduite par le jeu Wolfenstein 3D.
Le joueur évolue dans un monde en 2D, divisé en blocs de taille égale. Pour faire une impression de 3D, le moteur graphique tire des rayon virtuels depuis le joueur qui entrent en collision avec un mur. À ce moment, on affiche une barre verticale à l'écran dont la taille est proportionnelle à la longueur du rayon et la position relative à l'angle du rayon par rapport à celui du joueur.
Pour plus d'informations, notamment sur  les concepts mathématiques impliqués, nous vous invitons à vous référez à cette vidéo : [Wolfenstein 3D's map renderer](https://www.youtube.com/watch?v=eOCQfxRQ2pY).

Ce projet a été réalisé par en deux semaines, pour un TP de NSI.
