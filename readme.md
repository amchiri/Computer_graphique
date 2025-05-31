# Projet Académique OpenGL - Système Solaire et Scènes 3D

## Contexte du Projet
Ce projet a été réalisé dans le cadre d'un cours académique sur la programmation graphique avec OpenGL. Réalisé en groupe de 4, il vise à démontrer la maîtrise des concepts fondamentaux de la programmation 3D et des shaders.

### Objectifs Accomplis
1. **Affichage 3D Avancé**
   - Multiple shaders implémentés (Basic, Color, EnvMap)
   - Gestion du sRGB
   - Support des fichiers OBJ avec TinyOBJLoader
   - Gestion complète des matériaux (ambient, diffuse, specular)
   - Multiples modèles d'illumination :
     - Lambert
     - Phong
     - Blinn-Phong

2. **Navigation et Gestion de Scène**
   - Système de caméra libre avec contrôles WASD
   - Utilisation d'UBO pour :
     - Matrices de projection et vue
     - Transformations des objets
   - Multiples scènes :
     - Système solaire interactif
     - Scène de démonstration
     - Création de scènes personnalisées

3. **Fonctionnalités Avancées**
   - Implémentation d'une classe Mat4 personnalisée
   - Interface graphique complète avec ImGui
   - Gestion des transformations (position, rotation, échelle)
   - Chargement dynamique de modèles 3D
   - Skybox

4. **Améliorations Techniques**
   - Gestion de la mémoire avec smart pointers
   - Architecture orientée objet
   - Système de gestion de ressources
   - Support du wireframe
   - Gestion des lumières multiples

## Installation et Utilisation

### 1. Installation de l'environnement de développement

#### Installation de MSYS2
1. Télécharger MSYS2 depuis https://www.msys2.org
2. Installer MSYS2 et lancer MSYS2 MinGW64
3. Mettre à jour le système :
```bash
pacman -Syu
```

#### Installation des dépendances de développement
```bash
# Outils de développement
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-gdb
pacman -S mingw-w64-x86_64-cmake

# Bibliothèques graphiques
pacman -S mingw-w64-x86_64-glew
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-mesa

# Outils additionnels
pacman -S mingw-w64-x86_64-stb
```

#### Configuration de l'environnement
1. Ajouter au PATH système : `C:\msys64\mingw64\bin`
2. Redémarrer le terminal

### 2. Structure du projet
```
Basic/
  ├── src/          # Fichiers sources C++
  ├── include/      # Headers
  ├── imgui/        # ImGui library
  ├── assets/       # Ressources
  │   ├── shaders/  # Shaders GLSL
  │   └── textures/ # Textures
  └── build/        # Fichiers compilés
```

### 3. Compilation
```bash
# Créer les dossiers nécessaires
mkdir -p build assets/shaders assets/textures

# Compiler le projet
make

# Nettoyer
make clean
```

### 4. Développement
- Les shaders sont dans `assets/shaders/`
- Les textures sont dans `assets/textures/`
- Les fichiers sources dans `src/`
- Les headers dans `include/`

## Section Utilisateur

### 1. Installation
1. Télécharger la dernière version
2. Extraire l'archive
3. Installer les dépendances requises :
```bash
# Ouvrir MSYS2 MinGW64 et exécuter :
pacman -S mingw-w64-x86_64-glfw
pacman -S mingw-w64-x86_64-glew
```

### 2. Utilisation
1. Double-cliquer sur `BasicShader.exe`
2. Interface utilisateur :
   - Scene Manager : Gestion des scènes
   - Debug Info : Paramètres et informations
   - Light Settings : Contrôle de l'éclairage

### 3. Contrôles
- WASD/ZQSD : Déplacement caméra
- Souris : Rotation caméra
- O : Verrouiller/Déverrouiller la souris
- 1, 2 : Changer de scène
- N, P : Navigation entre les scènes

### 4. Fonctionnalités
- Système solaire interactif
- Plusieurs shaders (Basic, Color, EnvMap)
- Gestion de scènes
- Chargement de modèles 3D
- Éclairage dynamique

### 5. Dépannage
- Vérifier que GLFW et GLEW sont installés
- Vérifier que les shaders sont dans `assets/shaders/`
- Vérifier que les textures sont dans `assets/textures/`
