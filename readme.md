# Système Solaire OpenGL

Ce projet est une simulation simple du système solaire utilisant OpenGL, GLFW et ImGui.

## Prérequis

### Installation de MSYS2
1. Téléchargez et installez MSYS2 depuis https://www.msys2.org/
2. Ouvrez MSYS2 MinGW64 et mettez à jour le système :
```bash
pacman -Syu
```

### Installation des dépendances
Installez les paquets nécessaires :
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-stb
```

## Compilation

Le projet utilise un Makefile pour la compilation. Dans le dossier du projet, exécutez simplement :
```bash
make
```

Pour nettoyer les fichiers compilés :
```bash
make clean
```

## Exécution
```bash
./BasicShader.exe
```

## Contrôles
- WASD : Déplacement de la caméra
- Souris : Rotation de la caméra
- O : Verrouiller/déverrouiller le curseur
- ESC : Quitter

## Fonctionnalités
- Système solaire avec planètes en orbite
- Éclairage dynamique depuis le soleil
- Interface ImGui pour le debug et les paramètres
- Chargement de modèles 3D
- Skybox

## Structure du projet
- `src/core/` : Composants principaux
  - `BasicShader.cpp` : Point d'entrée et logique principale
  - `Mat4.cpp/.h` : Opérations matricielles
  - `ResourceManager.cpp/.h` : Gestion des ressources
  - `SceneManager.cpp/.h` : Gestion de la scène

- `src/rendering/` : Composants de rendu
  - `GLShader.cpp/.h` : Gestion des shaders
  - `Mesh.cpp/.h` : Gestion des objets 3D
  - `Planet.cpp/.h` : Logique des planètes
  - `Skybox.cpp/.h` : Gestion de la skybox

- `src/ui/` : Interface utilisateur
  - `UI.cpp/.h` : Interface ImGui
  - `imgui/` : Bibliothèque ImGui

- `src/utils/` : Utilitaires
  - `CameraController.cpp/.h` : Contrôle de la caméra
  - `tiny_obj_loader.cpp/.h` : Chargement de modèles 3D

- `resources/` : Ressources du projet
  - `models/` : Modèles 3D
  - `textures/` : Textures des planètes et skybox
  - `shaders/` : Shaders GLSL

## Dépannage
Si vous rencontrez des erreurs :
1. Vérifiez que tous les paquets sont installés
2. Assurez-vous que les fichiers de texture sont présents dans le dossier resources/textures
3. Vérifiez que tous les shaders sont présents dans le dossier resources/shaders

## Remarques
- Les textures des planètes doivent être dans le dossier resources/textures
- Les modèles 3D doivent être dans le dossier resources/models
- Les shaders doivent être dans le dossier resources/shaders
