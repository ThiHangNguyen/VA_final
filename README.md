🧩 AR Maze – Jeu en Réalité Augmentée sur Feuille A4

Ce projet implémente un jeu de labyrinthe en réalité augmentée, basé sur la détection et le tracking d’une feuille A4.
Le rendu combine OpenCV pour la vision et OpenGL pour l’affichage 3D temps réel.

Le système supporte plusieurs sources vidéo :

vidéo locale (par défaut),

webcam,

téléphone via caméra IP.

La commande pour voir le doc à la racine: 
xdg-open html/index.html (Ubuntu / Linux)
ou
google-chrome html/index.html

🛠️ Compilation du projet

Depuis la racine du projet :

cmake --build . -j

En cas de problème de compilation

Si le projet ne compile pas correctement :

rm -rf build
mkdir build
cd build
cmake ..
cmake --build . -j


Cela permet de repartir sur une configuration propre.

▶️ Lancement du programme
1️⃣ Mode par défaut — Vidéo locale
./AR_A4_Video


Utilise la vidéo fournie par défaut

Calibration caméra :

data/camera.yaml

2️⃣ Mode Webcam
./AR_A4_Video --webcam


⚠️ Important :
Il faut utiliser un fichier de calibration adapté à la webcam.

Exemple :

data/camera_webcam.yaml


Si nécessaire, modifier directement dans le code (par exemple dans src/input.cpp) :

cfg.calibPath = "../data/camera_webcam.yaml";

3️⃣ Mode Téléphone (Caméra IP +port )
./AR_A4_Video --phone http://192.168.1.110:4747/video


Ce mode utilise l’application DroidCam.

À adapter :

192.168.1.110 → adresse IP du téléphone

4747 → port utilisé par DroidCam

Calibration associée :

data/camera_ip11.yaml


Ou directement dans le code :

cfg.calibPath = "../data/camera_ip11.yaml";

🎮 Commandes clavier
Touche	Action
Espace	Pause / Reprendre le jeu
V	Basculer entre environnement réel et virtuel
Échap (ESC)	Quitter le jeu et revenir au menu
🧠 Remarques importantes

Le système est robuste dans des conditions de lumière spécifiques, mais pas parfait dans tous les environnements

Une bonne calibration caméra est essentielle pour la stabilité

En cas de problème :

vérifier la source vidéo,

vérifier le fichier .yaml,

nettoyer et recompiler le projet.

📱 Outils et bibliothèques utilisées

OpenCV – vision par ordinateur (détection A4, pose, tracking)

OpenGL / GLFW / GLEW – rendu 3D temps réel

DroidCam – caméra IP via smartphone

Doxygen – génération de la documentation du code

👩‍💻 Auteurs

Thi Hang NGUYEN

Bichoy DAOUD

✅ Note finale

Le projet est conçu pour un cadre académique (vision, AR, graphisme temps réel) et privilégie :

la clarté du pipeline,

la robustesse du tracking,

la séparation vision / rendu / logique jeu.