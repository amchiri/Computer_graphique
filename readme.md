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

Naviguez vers le dossier du projet et compilez :
```bash
cd /c/msys64/home/polom/Basic
g++ BasicShader.cpp GLShader.cpp MatrixUtils.cpp Mesh.cpp Mat4.cpp tiny_obj_loader.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_impl_glfw.cpp \
    imgui/imgui_impl_opengl3.cpp \
    -I. -Iimgui -lglew32 -lglfw3 -lopengl32 -lglu32 -lcomdlg32 -o BasicShader.exe
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

## Structure des fichiers
- `BasicShader.cpp` : Point d'entrée et logique principale
- `Mesh.cpp/.h` : Gestion des objets 3D
- `Mat4.cpp/.h` : Opérations matricielles
- `GLShader.cpp/.h` : Gestion des shaders
- `*.vs/*.fs` : Shaders GLSL

## Dépannage
Si vous rencontrez des erreurs :
1. Vérifiez que tous les paquets sont installés
2. Assurez-vous que les fichiers de texture sont présents
3. Vérifiez les chemins d'accès dans le code

## Remarques
- Les textures des planètes doivent être dans le dossier du projet
- Le fichier `space.png` est nécessaire pour la skybox


g++ BasicShader.cpp GLShader.cpp Mesh.cpp Mat4.cpp tiny_obj_loader.cpp imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp imgui/imgui_tables.cpp imgui/imgui_impl_glfw.cpp imgui/imgui_impl_opengl3.cpp -I. -Iimgui -lglew32 -lglfw3 -lopengl32 -lglu32 -lcomdlg32 -o BasicShader.exe