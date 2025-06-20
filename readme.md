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

# Basic 3D Engine

Un moteur de rendu 3D développé en C++ avec OpenGL, featuring un système solaire interactif et plusieurs techniques de rendu avancées.

**⭐ Ce projet répond intégralement aux critères demandés pour le rendu académique ⭐**

## 📹 Vidéo de Démonstration

**Regardez la démonstration complète du projet ici :**

[![Démonstration du projet](https://img.youtube.com/vi/wlvpWvKUzZ0/maxresdefault.jpg)](https://youtu.be/wlvpWvKUzZ0)

🎥 **[Cliquez ici pour voir la vidéo explicative](https://youtu.be/wlvpWvKUzZ0)**

Cette vidéo présente toutes les fonctionnalités du moteur, incluant :
- Navigation dans le système solaire
- Changement de shaders en temps réel
- Chargement de modèles 3D
- Environment mapping et réflexions
- Gestion des scènes multiples

## 📋 Conformité aux Critères Académiques

### ✅ **Partie 1 : Affichage**

#### **Plusieurs objets avec shaders différents :**
- ✅ **Color Shader** : Objets avec couleurs simples (démo scene - sphere rouge)
- ✅ **Basic Shader** : Objets avec textures et éclairage Phong/Blinn-Phong (planètes, soleil)
- ✅ **Environment Mapping Shader** : Réflexions avec cubemaps (démo scene - sphere métallique)

#### **Gestion sRGB :**
- ✅ Textures chargées avec format `GL_SRGB8_ALPHA8`
- ✅ Pipeline de rendu respectant l'espace colorimétrique sRGB

#### **Chargement OBJ avec TinyOBJLoader :**
- ✅ Integration complète de TinyOBJLoader
- ✅ Chargement automatique des matériaux MTL
- ✅ Support des couleurs ambiantes, diffuses, spéculaires
- ✅ Gestion des textures associées aux matériaux
- ✅ Triangularisation automatique

#### **Éclairage Phong/Blinn-Phong :**
- ✅ Équations d'éclairage Phong et Blinn-Phong implémentées
- ✅ Choix du modèle d'illumination par objet (Lambert, Phong, Blinn-Phong)
- ✅ Éclairage émissif multiple
- ✅ Calcul correct des composantes ambiante, diffuse et spéculaire

### ✅ **Partie 2 : Navigation**

#### **Positionnement des objets :**
- ✅ Objets placés à des positions différentes
- ✅ Transformations complètes : translation, rotation, scale
- ✅ Système solaire avec orbites animées
- ✅ Scène de démonstration avec mouvements de va-et-vient

#### **Uniform Buffer Objects (UBO) :**
- ✅ UBO pour projection + vue (ProjectionView)
- ✅ UBO pour les transformations d'objets (Transform)
- ✅ Optimisation des performances GPU

#### **Caméra 3D :**
- ✅ Navigation libre dans l'espace 3D (WASD + souris)
- ✅ Rotation de caméra avec souris
- ✅ Zoom avec molette
- ✅ Contrôles verticaux (Espace/Shift)

### ✅ **Partie 3 : Options**

#### **Classe Mat4 personnalisée :**
- ✅ Classe Mat4 complète en C++
- ✅ Multiplication de matrices
- ✅ Transformations (translate, rotate, scale)
- ✅ Calcul de WorldMatrix via UBO

#### **Interface graphique ImGui :**
- ✅ Interface complète avec ImGui
- ✅ Contrôles de scène en temps réel
- ✅ Modification des paramètres de matériaux
- ✅ Sélection de shaders par objet
- ✅ Chargement de modèles via interface
- ✅ Gestion des skybox et cubemaps

#### **Fonctionnalités bonus :**
- ✅ Système de scènes multiples
- ✅ Environment mapping avec cubemaps personnalisés
- ✅ Skybox dynamique
- ✅ Mode wireframe pour debugging
- ✅ Système de ressources optimisé

## 🚀 Fonctionnalités Techniques

### Rendu 3D Avancé
- **OpenGL 3.3+** : Pipeline moderne avec shaders GLSL
- **Shaders multiples** : Basic (Phong/Blinn-Phong), Color, Environment Mapping
- **Environment Mapping** : Réflexions réalistes avec support de cubemaps personnalisés
- **Skybox dynamique** : Chargement de cubemaps depuis des dossiers
- **Éclairage émissif** : Support de multiples sources de lumière

### Système Solaire Interactif
- **Échelles réalistes** : Tailles relatives correctes des planètes
- **Orbites animées** : Mouvements planétaires en temps réel
- **Textures planétaires** : Textures haute qualité pour chaque planète
- **Contrôles interactifs** : Modification des paramètres orbitaux via l'interface

### Gestion de Scènes
- **Scènes multiples** : Système solaire, scène de démonstration, scènes personnalisées
- **Chargement de modèles** : Support des fichiers OBJ avec textures
- **Interface utilisateur** : Interface ImGui complète pour tous les réglages

### Techniques de Rendu
- **Uniform Buffer Objects** : Optimisation des performances
- **Texture Management** : Chargement automatique de textures avec les modèles
- **Wireframe Mode** : Mode filaire pour le debugging
- **Camera Controller** : Navigation 3D fluide

## 🛠️ Installation et Compilation

### Prérequis
- Windows 10/11
- Visual Studio 2019/2022 ou MinGW-w64
- OpenGL 3.3+

### Dépendances Incluses
- **GLFW** : Gestion des fenêtres et entrées
- **GLEW** : Extensions OpenGL
- **ImGui** : Interface utilisateur
- **stb_image** : Chargement d'images
- **tiny_obj_loader** : Chargement de modèles OBJ (critère académique)

### Compilation
```bash
# Avec MinGW (MSYS2)
cd c:\msys64\home\polom\Basic
mkdir build && cd build
cmake ..
make

# Ou utiliser le Makefile direct
make
```

## 🎮 Utilisation

### Navigation (Critère académique)
- **Souris** : Rotation de la caméra
- **WASD** : Déplacement
- **Molette** : Zoom
- **Espace/Shift** : Monter/Descendre

### Interface Utilisateur (ImGui - Critère académique)

#### Scene Manager
- Changement de scènes en temps réel
- Création de nouvelles scènes vides
- Navigation entre scènes

#### Solar System Objects
- Contrôle de la position et taille du soleil
- Paramètres orbitaux des planètes (rayon, vitesse, taille)
- Objets personnalisés chargés

#### Shader Settings (Critère académique)
- **Basic Shader** : Éclairage Phong/Blinn-Phong complet
- **Color Shader** : Rendu simple avec couleurs unies
- **EnvMap Shader** : Environment mapping avec réflexions

#### Scene Objects
- Chargement de modèles 3D (.obj) avec TinyOBJLoader
- Changement de skybox via sélection de dossier
- Gestion des objets personnalisés

### Chargement de Contenu (Critère académique)

#### Modèles 3D (TinyOBJLoader)
1. Cliquer sur "Load 3D Model"
2. Sélectionner un fichier .obj
3. Le modèle apparaît avec ses textures automatiquement chargées
4. Les matériaux MTL sont traités automatiquement

#### Skybox Personnalisé
1. Préparer un dossier avec 6 images : `right.png`, `left.png`, `top.png`, `bottom.png`, `front.png`, `back.png`
2. Cliquer sur "Change Skybox"
3. Sélectionner le dossier contenant les images

#### Cubemaps pour Environment Mapping
- Chaque objet peut avoir son propre cubemap
- Boutons "Create Procedural CubeMap" et "Load CubeMap Directory"
- Support des réflexions personnalisées par objet

## 📁 Structure du Projet
