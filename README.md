Projet AR 2025 – Détection d’une feuille A4 + Réalité augmentée (OpenCV + OpenGL)

Résumé :
-------
Ce projet permet de détecter une feuille A4 dans une vidéo ou depuis la webcam,
d’estimer sa pose avec solvePnP (OpenCV), puis d’afficher un cube en réalité augmentée
sur cette feuille, en utilisant OpenGL (GLFW, GLEW, GLM).

Structure :
----------
AR2025/
├── src/               --> Code source principal
│   ├── detect/        --> Détection (coins, contours, A4)
│   ├── gl/            --> Fonctions d'affichage OpenGL (cube, axes, shaders)
│   ├── ar/            --> Estimation de pose (solvePnP), transformations
├── data/              --> Vidéos de test + fichiers camera.yaml
├── include/           --> Headers publics (optionnel)
├── build/             --> Dossier build (vide au début)
├── CMakeLists.txt     --> Fichier de configuration cmake
└── README.txt         --> Ce fichier

Compilation :
-------------
Depuis le dossier racine :

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j

Le binaire sera généré dans : build/AR_A4_Video

Exécution :
-----------

1) Lancer avec une vidéo + calibration

    ./build/AR_A4_Video --video ../data/video_test.mp4 ../data/camera_ip11.yaml

2) Lancer avec la webcam (détection en live)

    ./build/AR_A4_Video --webcam

3) Lancer en mode par défaut (vidéo du professeur déjà codée en dur)

    ./build/AR_A4_Video

Fonctionnalités :
-----------------

- Détection dynamique d’une feuille A4 :
    * seuillage adaptatif basé sur la moyenne locale
    * approximation quadrilatère avec validation géométrique

- Estimation de pose avec solvePnP (OpenCV)
- Affichage 3D d’un cube et d’axes (OpenGL 3.3)
- Support webcam ou vidéo
- Génération possible de calibration YAML depuis damier ou A4

Calibration :
-------------
Si besoin de calibrer une nouvelle caméra :
Utiliser le programme s pour obtenir un fichier camera.yaml


Recordvideo:
-------------

enregistrer la vidéo  par webcam

Doxygen :
---------

Pour générer la documentation :

    doxygen Doxyfile

La doc HTML sera disponible dans doc/html/index.html
la cmd pour l'ouvrir : xdg-open doc/html/index.html


Commandes utiles :
------------------

Compiler :

    cmake --build build -j

Lancer avec vidéo :

    ./build/AR_A4_Video --video ../data/video_test.mp4 ../data/camera_ip11.yaml

Lancer avec webcam :

    ./build/AR_A4_Video --webcam

Lancer par défaut :

    ./build/AR_A4_Video

lancer avec la caméra du téléphone, exemple 
    ./AR_A4_Video --phone http://147.94.216.154:4747/video

Auteur :
--------

Projet AR 2025 – Étudiante en informatique, Polytech Marseille: Bichoy DAOUD & Thi Hang NGUYEN
Encadré par S. Mavromatis
Réalisé en C++ avec OpenCV 4.5+, GLFW, GLEW, GLM
