# Système Solaire OpenGL

## Section Développeur

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
- ZQSD/WASD : Déplacement caméra
- Souris : Rotation caméra
- Espace : Monter
- Ctrl : Descendre
- Échap : Menu

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
