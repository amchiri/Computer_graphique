#include "../include/SceneManager.h"
#include <iostream>
#include <algorithm>
#include <cstdio>  // Pour sprintf
#include "../include/CameraController.h" // Ajouté pour CameraController
#include <UBOManager.h>
#include <filesystem> // Pour vérifier l'existence des fichiers

// Définir MAX_LIGHTS en haut du fichier
#define MAX_LIGHTS 10

// ==================== Scene Implementation ====================

std::string Scene::GetShaderPath(const std::string& filename) {
    // Obtenir le chemin de l'exécutable
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path executablePath = std::filesystem::path(exePath).parent_path();
    std::cout << "Executable path: " << executablePath << std::endl;

    // Chemins possibles pour les shaders
    std::vector<std::filesystem::path> searchPaths = {
        executablePath / "assets" / "shaders",
        executablePath / ".." / "assets" / "shaders",
        executablePath / ".." / ".." / "assets" / "shaders",
        std::filesystem::current_path() / "assets" / "shaders",
        "assets/shaders"  // Chemin relatif de base
    };

    for (const auto& basePath : searchPaths) {
        std::filesystem::path shaderPath = basePath / filename;
        std::cout << "Trying path: " << shaderPath << std::endl;
        if (std::filesystem::exists(shaderPath)) {
            std::cout << "Found shader at: " << shaderPath << std::endl;
            return shaderPath.string();
        }
    }

    std::cerr << "Could not find shader file: " << filename << " in any search path" << std::endl;
    return filename;
}

bool Scene::InitializeCubeMap() {
    std::cout << "Initializing cubemap..." << std::endl;
    
    // Créer un cubemap procédural par défaut
    if (m_CubeMap.CreateProcedural()) {
        std::cout << "Procedural cubemap created successfully!" << std::endl;
        return true;
    }
    
    std::cerr << "Failed to create cubemap!" << std::endl;
    return false;
}

bool Scene::InitializeShaders() {
    std::cout << "Loading Basic shader..." << std::endl;
    
    std::string baseDir = std::filesystem::current_path().string();
    std::cout << "Current working directory before shader loading: " << baseDir << std::endl;
    
    // Sauvegarder le répertoire de travail actuel
    char previousDir[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, previousDir);
    
    // Essayer de revenir au répertoire de l'exécutable
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path executableDir = std::filesystem::path(exePath).parent_path();
    SetCurrentDirectoryA(executableDir.string().c_str());
    
    std::string basicVS = GetShaderPath("Basic.vs");
    std::string basicFS = GetShaderPath("Basic.fs");
    std::string colorVS = GetShaderPath("Color.vs");
    std::string colorFS = GetShaderPath("Color.fs");
    std::string envMapVS = GetShaderPath("EnvMap.vs");
    std::string envMapFS = GetShaderPath("EnvMap.fs");
    
    if (!std::filesystem::exists(basicVS) || !std::filesystem::exists(basicFS)) {
        std::cerr << "Basic shader files not found at: " << basicVS << std::endl;
        return false;
    }

    if (!m_basicShader.LoadVertexShader(basicVS.c_str())) {
        std::cerr << "Failed to load Basic vertex shader" << std::endl;
        return false;
    }
    if (!m_basicShader.LoadFragmentShader(basicFS.c_str())) {
        std::cerr << "Failed to load Basic fragment shader" << std::endl;
        return false;
    }
    if (!m_basicShader.Create()) {
        std::cerr << "Failed to create Basic shader program" << std::endl;
        return false;
    }

    std::cout << "Loading Color shader..." << std::endl;
    
    if (!std::filesystem::exists(colorVS) ||
        !std::filesystem::exists(colorFS)) {
        std::cerr << "Color shader files not found!" << std::endl;
        return false;
    }

    if (!m_colorShader.LoadVertexShader(colorVS.c_str())) {
        std::cerr << "Failed to load Color vertex shader" << std::endl;
        return false;
    }
    if (!m_colorShader.LoadFragmentShader(colorFS.c_str())) {
        std::cerr << "Failed to load Color fragment shader" << std::endl;
        return false;
    }
    if (!m_colorShader.Create()) {
        std::cerr << "Failed to create Color shader program" << std::endl;
        return false;
    }

    std::cout << "Loading EnvMap shader..." << std::endl;
    
    if (!std::filesystem::exists(envMapVS) ||
        !std::filesystem::exists(envMapFS)) {
        std::cerr << "EnvMap shader files not found!" << std::endl;
        return false;
    }

    if (!m_envMapShader.LoadVertexShader(envMapVS.c_str())) {
        std::cerr << "Failed to load EnvMap vertex shader" << std::endl;
        return false;
    }
    if (!m_envMapShader.LoadFragmentShader(envMapFS.c_str())) {
        std::cerr << "Failed to load EnvMap fragment shader" << std::endl;
        return false;
    }
    if (!m_envMapShader.Create()) {
        std::cerr << "Failed to create EnvMap shader program" << std::endl;
        return false;
    }

    // Restaurer le répertoire de travail précédent
    SetCurrentDirectoryA(previousDir);
    
    // Initialiser le cubemap après les shaders
    if (!InitializeCubeMap()) {
        std::cerr << "Failed to initialize cubemap" << std::endl;
        // On continue quand même, ce n'est pas critique
    }
    
    std::cout << "All shaders loaded successfully" << std::endl;
    return true;
}

void Scene::CleanupShaders() {
    m_basicShader.Destroy();
    m_colorShader.Destroy();
    m_envMapShader.Destroy();
}

// ==================== SolarSystemScene Implementation ====================

SolarSystemScene::~SolarSystemScene() {
    Cleanup();
}

bool SolarSystemScene::Initialize() {
    if (!InitializeShaders()) {
        return false;
    }
    
    try {
        createSun();
        createPlanets();
        loadPlanetTextures();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing Solar System Scene: " << e.what() << std::endl;
        return false;
    }
}

void SolarSystemScene::Update(float deltaTime) {
    updatePlanets(deltaTime);
}

void SolarSystemScene::Render(const Mat4& projection, const Mat4& view) {
    // Obtenir la position de la caméra pour les calculs de spécularité
    extern CameraController* g_Camera;
    const float* cameraPos = g_Camera->GetPosition();

    for (Mesh* obj : m_objects) {
        GLShader* shader = nullptr;
        if (obj == m_sun) {
            shader = &GetBasicShader();
        } else {
            shader = &GetBasicShader();  // Par défaut
        }
        
        GLuint program = shader->GetProgram();
        glUseProgram(program);

        // Matrices communes à tous les shaders
        GLint loc_proj = glGetUniformLocation(program, "u_projection");
        if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projection.data());
        
        GLint loc_view = glGetUniformLocation(program, "u_view");
        if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_FALSE, view.data());

        // Matrice de transformation de l'objet
        float modelMatrix[16];
        obj->calculateModelMatrix(modelMatrix);
        GLint loc_transform = glGetUniformLocation(program, "u_transform");
        if (loc_transform >= 0) glUniformMatrix4fv(loc_transform, 1, GL_FALSE, modelMatrix);

        // Configuration spécifique selon le type de shader
        if (shader == &GetBasicShader()) {
            setupBasicShader(program, obj, m_lightColor, m_lightIntensity, cameraPos);
        }
        else if (shader == &GetColorShader()) {
            setupColorShader(program, obj);
        }
        else if (shader == &GetEnvMapShader()) {
            setupEnvMapShader(program, obj, cameraPos);
        }

        obj->draw(*shader);
    }
}

void SolarSystemScene::setupBasicShader(GLuint program, Mesh* obj, float* light_color, float light_intensity, const float* cameraPos) {
    // Configuration de l'éclairage principal (le soleil)
    if (m_sun) {
        const float* sunPos = m_sun->getPosition();
        GLint loc_lightDir = glGetUniformLocation(program, "u_light.direction");
        if (loc_lightDir >= 0) glUniform3f(loc_lightDir, sunPos[0], sunPos[1], sunPos[2]);
    }
    
    float lightDiffuse[3] = {
        light_color[0] * light_intensity * 2.0f,
        light_color[1] * light_intensity * 2.0f,
        light_color[2] * light_intensity * 2.0f
    };

    GLint loc_lightDiffuse = glGetUniformLocation(program, "u_light.diffuseColor");
    GLint loc_lightSpecular = glGetUniformLocation(program, "u_light.specularColor");
    GLint loc_intensity = glGetUniformLocation(program, "u_intensity");
    
    if (loc_intensity >= 0) glUniform1f(loc_intensity, light_intensity);
    if (loc_lightDiffuse >= 0) glUniform3fv(loc_lightDiffuse, 1, lightDiffuse);
    if (loc_lightSpecular >= 0) glUniform3fv(loc_lightSpecular, 1, lightDiffuse);

    // Position de la caméra pour les calculs de spécularité
    GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
    if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, cameraPos);

    // Configuration des lumières émissives multiples
    std::vector<Mesh*> emissiveLights;
    if (m_sun && m_sun->getMaterial().isEmissive) {
        emissiveLights.push_back(m_sun);
    }
    
    // Ajouter d'autres objets émissifs de la scène
    for (Mesh* meshObj : m_objects) {
        if (meshObj != m_sun && meshObj->getMaterial().isEmissive) {
            emissiveLights.push_back(meshObj);
        }
    }

    int numLights = std::min((int)emissiveLights.size(), (int)MAX_LIGHTS);
    GLint loc_numLights = glGetUniformLocation(program, "u_numEmissiveLights");
    if (loc_numLights >= 0) glUniform1i(loc_numLights, numLights);
    
    for (size_t i = 0; i < emissiveLights.size() && i < MAX_LIGHTS; i++) {
        const auto& light = emissiveLights[i];
        const auto& mat = light->getMaterial();
        const float* pos = light->getPosition();

        char buffer[64];
        sprintf(buffer, "u_emissiveLights[%zu].position", i);
        GLint loc_pos = glGetUniformLocation(program, buffer);
        if (loc_pos >= 0) glUniform3fv(loc_pos, 1, pos);
        
        sprintf(buffer, "u_emissiveLights[%zu].color", i);
        GLint loc_color = glGetUniformLocation(program, buffer);
        if (loc_color >= 0) glUniform3fv(loc_color, 1, mat.lightColor);
        
        sprintf(buffer, "u_emissiveLights[%zu].intensity", i);
        GLint loc_lightIntensity = glGetUniformLocation(program, buffer);
        if (loc_lightIntensity >= 0) glUniform1f(loc_lightIntensity, mat.emissiveIntensity);
    }

    // Configuration du matériau de l'objet
    const Material& mat = obj->getMaterial();
    
    GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
    GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
    GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
    GLint loc_isEmissive = glGetUniformLocation(program, "u_material.isEmissive");
    GLint loc_emissiveIntensity = glGetUniformLocation(program, "u_material.emissiveIntensity");
    GLint loc_lightColor = glGetUniformLocation(program, "u_material.lightColor");
    
    if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, mat.diffuse);
    if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, mat.specular);
    if (loc_matShininess >= 0) glUniform1f(loc_matShininess, mat.shininess);
    if (loc_isEmissive >= 0) glUniform1i(loc_isEmissive, mat.isEmissive ? 1 : 0);
    if (loc_emissiveIntensity >= 0) glUniform1f(loc_emissiveIntensity, mat.emissiveIntensity);
    if (loc_lightColor >= 0) glUniform3fv(loc_lightColor, 1, mat.lightColor);
    
    // Gérer l'affichage de la texture pour le shader Basic - CORRECTION COMPLÈTE
    GLint loc_useTexture = glGetUniformLocation(program, "u_useTexture");
    GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
    
    bool hasTexture = (obj->getMaterial().diffuseMap != 0);
    if (loc_hasTexture >= 0) glUniform1i(loc_hasTexture, hasTexture);
    if (loc_useTexture >= 0) glUniform1i(loc_useTexture, mat.useTextureInBasicShader);
    
    // Lier la texture seulement si on veut l'utiliser ET qu'elle existe
    if (hasTexture && mat.useTextureInBasicShader) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->getMaterial().diffuseMap);
        GLint loc_texture = glGetUniformLocation(program, "u_texture");
        if (loc_texture >= 0) glUniform1i(loc_texture, 0);
    } else {
        // IMPORTANT: Débinder la texture si on ne l'utilise pas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void SolarSystemScene::setupColorShader(GLuint program, Mesh* obj) {
    const Material& mat = obj->getMaterial();
    GLint loc_color = glGetUniformLocation(program, "u_color");
    if (loc_color >= 0) {
        glUniform3fv(loc_color, 1, mat.diffuse);
    }
    
    // Gérer l'affichage de la texture pour le shader Color - CORRECTION
    GLint loc_useTexture = glGetUniformLocation(program, "u_useTexture");
    GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
    
    bool hasTexture = (obj->getMaterial().diffuseMap != 0);
    if (loc_hasTexture >= 0) glUniform1i(loc_hasTexture, hasTexture);
    if (loc_useTexture >= 0) glUniform1i(loc_useTexture, mat.useTextureInColorShader);
    
    // Lier la texture seulement si on veut l'utiliser ET qu'elle existe
    if (hasTexture && mat.useTextureInColorShader) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->getMaterial().diffuseMap);
        GLint loc_texture = glGetUniformLocation(program, "u_texture");
        if (loc_texture >= 0) glUniform1i(loc_texture, 0);
    } else {
        // IMPORTANT: Débinder la texture si on ne l'utilise pas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void SolarSystemScene::setupEnvMapShader(GLuint program, Mesh* obj, const float* cameraPos) {
    const Material& mat = obj->getMaterial();
    
    // Position de la caméra pour les réflections
    GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
    if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, cameraPos);
    
    // Nouveau uniform pour ignorer le matériau
    GLint loc_ignoreMaterial = glGetUniformLocation(program, "u_ignoreObjectMaterial");
    if (loc_ignoreMaterial >= 0) glUniform1i(loc_ignoreMaterial, mat.ignoreObjectMaterialInEnvMap);
    
    // Si on ignore le matériau, pas besoin de configurer les autres paramètres
    if (!mat.ignoreObjectMaterialInEnvMap) {
        // Paramètres du matériau pour l'environment mapping
        GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
        GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
        GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
        GLint loc_specularStrength = glGetUniformLocation(program, "u_material.specularStrength");
        
        if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, mat.diffuse);
        if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, mat.specular);
        if (loc_matShininess >= 0) glUniform1f(loc_matShininess, mat.shininess);
        if (loc_specularStrength >= 0) glUniform1f(loc_specularStrength, mat.specularStrength);
        
        // Gérer l'affichage de la texture pour le shader EnvMap
        GLint loc_useTexture = glGetUniformLocation(program, "u_useTexture");
        GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
        
        bool hasTexture = (obj->getMaterial().diffuseMap != 0);
        if (loc_hasTexture >= 0) glUniform1i(loc_hasTexture, hasTexture);
        if (loc_useTexture >= 0) glUniform1i(loc_useTexture, mat.useTextureInEnvMapShader);
        
        // Lier la texture seulement si on veut l'utiliser ET qu'elle existe
        if (hasTexture && mat.useTextureInEnvMapShader) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, obj->getMaterial().diffuseMap);
            GLint loc_texture = glGetUniformLocation(program, "u_texture");
            if (loc_texture >= 0) glUniform1i(loc_texture, 1);
        } else {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    } else {
        // Si on ignore le matériau, débinder toutes les textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        GLint loc_useTexture = glGetUniformLocation(program, "u_useTexture");
        GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
        if (loc_hasTexture >= 0) glUniform1i(loc_hasTexture, 0);
        if (loc_useTexture >= 0) glUniform1i(loc_useTexture, 0);
    }
    
    // Associer le cubemap au shader (unité de texture 0) - toujours nécessaire
    if (m_CubeMap.IsLoaded()) {
        m_CubeMap.Bind(0);
        GLint loc_envmap = glGetUniformLocation(program, "u_envmap");
        if (loc_envmap >= 0) glUniform1i(loc_envmap, 0);
    }
}

void SolarSystemScene::Cleanup() {
    // Nettoyage des objets qui ne sont pas des planètes
    std::vector<Mesh*> meshesToDelete;
    for(Mesh* obj : m_objects) {
        bool isPlanetMesh = false;
        for(const auto& planet : m_planets) {
            if(planet.GetMesh() == obj) {
                isPlanetMesh = true;
                break;
            }
        }
        
        if(!isPlanetMesh && obj != m_sun) {
            meshesToDelete.push_back(obj);
        }
    }

    for(Mesh* obj : meshesToDelete) {
        delete obj;
    }

    delete m_sun;
    m_sun = nullptr;
    m_objects.clear();
    m_planets.clear();
}

void SolarSystemScene::createSun() {
    m_sun = new Mesh();
    m_sun->createSphere(1.0f, 32, 32);

    const float sunScale = 8.0f;
    m_sun->setScale(sunScale, sunScale, sunScale);
    m_sun->setPosition(0.0f, 0.0f, 0.0f);

    Material sunMaterial;
    sunMaterial.diffuse[0] = sunMaterial.diffuse[1] = sunMaterial.diffuse[2] = 1.0f;
    sunMaterial.specular[0] = sunMaterial.specular[1] = sunMaterial.specular[2] = 1.0f;
    sunMaterial.shininess = 32.0f;
    sunMaterial.isEmissive = true;
    sunMaterial.emissiveIntensity = 2.0f;
    sunMaterial.lightColor[0] = sunMaterial.lightColor[1] = sunMaterial.lightColor[2] = 1.0f;
    m_sun->setMaterial(sunMaterial);
    m_sun->loadTexture("assets/textures/sun.png");

    // Le soleil commence avec le shader basique par défaut
    // Il peut être changé via l'interface utilisateur
    m_sun->setCurrentShader(nullptr); // Sera assigné lors du premier rendu

    m_objects.push_back(m_sun);
}

void SolarSystemScene::createPlanets() {
    // Paramètres: rayon orbital, vitesse orbitale, taille
    float planetData[][3] = {
        {15.0f, 0.8f, 0.8f},    // Mercure
        {20.0f, 0.6f, 1.2f},    // Vénus
        {28.0f, 0.4f, 1.5f},    // Terre
        {35.0f, 0.3f, 1.2f},    // Mars
        {50.0f, 0.15f, 4.0f},   // Jupiter
    };

    for(int i = 0; i < 7; i++) {
        m_planets.emplace_back();
        m_planets.back().Initialize(
            planetData[i][0],    // rayon orbital
            planetData[i][1],    // vitesse de rotation
            planetData[i][2]     // taille
        );
        
        // Chaque planète peut avoir son propre shader
        Mesh* planetMesh = m_planets.back().GetMesh();
        planetMesh->setCurrentShader(nullptr); // Sera assigné lors du premier rendu
        
        m_objects.push_back(planetMesh);
    }
}

void SolarSystemScene::loadPlanetTextures() {
    const char* textures[] = {
        "assets/textures/mercury.png",
        "assets/textures/venus.png",
        "assets/textures/earth.png",
        "assets/textures/mars.png",
        "assets/textures/jupiter.png",
        "assets/textures/saturn.png",
        "assets/textures/uranus.png"
    };

    for(size_t i = 0; i < m_planets.size(); i++) {
        Planet& planet = m_planets[i];
        Material planetMaterial;
        planetMaterial.diffuse[0] = planetMaterial.diffuse[1] = planetMaterial.diffuse[2] = 1.0f;
        planetMaterial.specular[0] = planetMaterial.specular[1] = planetMaterial.specular[2] = 0.5f;
        planetMaterial.shininess = 32.0f;
        planetMaterial.isEmissive = false;
        planetMaterial.emissiveIntensity = 0.0f;
        planetMaterial.lightColor[0] = planetMaterial.lightColor[1] = planetMaterial.lightColor[2] = 1.0f;
        planet.SetMaterial(planetMaterial);
        
        if (!planet.LoadTexture(textures[i])) {
            std::cerr << "Error: Could not load texture for planet " << i << std::endl;
        }
    }
}

void SolarSystemScene::updatePlanets(float deltaTime) {
    for(auto& planet : m_planets) {
        planet.Update(deltaTime);
    }
}

// ==================== DemoScene Implementation ====================

DemoScene::~DemoScene() {
    Cleanup();
}

bool DemoScene::Initialize() {
    // Ajouter l'initialisation des shaders !
    if (!InitializeShaders()) {
        return false;
    }

    try {
        createDemoObjects();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing Demo Scene: " << e.what() << std::endl;
        return false;
    }
}

void DemoScene::Update(float deltaTime) {
    m_rotationTime += deltaTime;
    
    // Faire des mouvements de va-et-vient pour les objets de démonstration
    if (m_objects.size() >= 3) {
        float time = m_rotationTime;
        
        // Premier objet : mouvement horizontal (axe X)
        float posX = 5.0f * sin(time * 1.0f);  // Amplitude de 5 unités, fréquence normale
        m_objects[0]->setPosition(-8 + posX, 0, -10);
        
        // Deuxième objet : mouvement vertical (axe Y)
        float posY = 3.0f * sin(time * 1.5f);  // Amplitude de 3 unités, fréquence plus rapide
        m_objects[1]->setPosition(0, posY, -10);
        
        // Troisième objet : mouvement en profondeur (axe Z)
        float posZ = 4.0f * sin(time * 0.8f);  // Amplitude de 4 unités, fréquence plus lente
        m_objects[2]->setPosition(8, 0, -10 + posZ);
    }
}

void DemoScene::Render(const Mat4& projection, const Mat4& view) {
    // Variables d'éclairage
    static float light_color[3] = { 1.0f, 1.0f, 1.0f };
    static float light_intensity = 1.0f;
    static float lightPos[3] = { 0.0f, 10.0f, 0.0f };

    extern CameraController* g_Camera;
    const float* cameraPos = g_Camera->GetPosition();

    for (Mesh* obj : m_objects) {
        GLShader* currentShader = obj->getCurrentShader();
        
        // Si l'objet n'a pas de shader assigné, utiliser le shader approprié par défaut
        if (!currentShader) {
            // Assigner différents shaders aux objets de demo pour montrer la variété
            size_t objIndex = std::find(m_objects.begin(), m_objects.end(), obj) - m_objects.begin();
            switch (objIndex % 3) {
                case 0: currentShader = &GetColorShader(); break;
                case 1: currentShader = &GetBasicShader(); break;
                case 2: currentShader = &GetEnvMapShader(); break;
                default: currentShader = &GetBasicShader(); break;
            }
            obj->setCurrentShader(currentShader);
        }

        GLuint program = currentShader->GetProgram();
        glUseProgram(program);

        // Matrices communes
        GLint loc_proj = glGetUniformLocation(program, "u_projection");
        if (loc_proj >= 0) glUniformMatrix4fv(loc_proj, 1, GL_FALSE, projection.data());
        
        GLint loc_view = glGetUniformLocation(program, "u_view");
        if (loc_view >= 0) glUniformMatrix4fv(loc_view, 1, GL_FALSE, view.data());

        // Matrice de transformation de l'objet
        float modelMatrix[16];
        obj->calculateModelMatrix(modelMatrix);
        GLint loc_transform = glGetUniformLocation(program, "u_transform");
        if (loc_transform >= 0) glUniformMatrix4fv(loc_transform, 1, GL_FALSE, modelMatrix);

        // Configuration spécifique selon le shader
        if (currentShader == &GetColorShader()) {
            setupColorShaderDemo(program, obj);
        }
        else if (currentShader == &GetBasicShader()) {
            setupBasicShaderDemo(program, obj, light_color, light_intensity, lightPos, cameraPos);
        }
        else if (currentShader == &GetEnvMapShader()) {
            setupEnvMapShaderDemo(program, obj, cameraPos);
        }

        obj->draw(*currentShader);
    }
}

void DemoScene::setupColorShaderDemo(GLuint program, Mesh* obj) {
    const Material& mat = obj->getMaterial();
    GLint loc_color = glGetUniformLocation(program, "u_color");
    if (loc_color >= 0) {
        glUniform3fv(loc_color, 1, mat.diffuse);
    }
    
    // Gérer l'affichage de la texture pour le shader Color - CORRECTION
    GLint loc_useTexture = glGetUniformLocation(program, "u_useTexture");
    GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
    
    bool hasTexture = (obj->getMaterial().diffuseMap != 0);
    if (loc_hasTexture >= 0) glUniform1i(loc_hasTexture, hasTexture);
    if (loc_useTexture >= 0) glUniform1i(loc_useTexture, mat.useTextureInColorShader);
    
    // Lier la texture seulement si on veut l'utiliser ET qu'elle existe
    if (hasTexture && mat.useTextureInColorShader) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->getMaterial().diffuseMap);
        GLint loc_texture = glGetUniformLocation(program, "u_texture");
        if (loc_texture >= 0) glUniform1i(loc_texture, 0);
    } else {
        // IMPORTANT: Débinder la texture si on ne l'utilise pas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void DemoScene::setupBasicShaderDemo(GLuint program, Mesh* obj, float* light_color, float light_intensity, float* lightPos, const float* cameraPos) {
    // Éclairage
    GLint loc_lightDir = glGetUniformLocation(program, "u_light.direction");
    if (loc_lightDir >= 0) glUniform3f(loc_lightDir, lightPos[0], lightPos[1], lightPos[2]);
    
    float lightDiffuse[3] = {
        light_color[0] * light_intensity,
        light_color[1] * light_intensity,
        light_color[2] * light_intensity
    };

    GLint loc_lightDiffuse = glGetUniformLocation(program, "u_light.diffuseColor");
    GLint loc_lightSpecular = glGetUniformLocation(program, "u_light.specularColor");
    GLint loc_intensity = glGetUniformLocation(program, "u_intensity");
    GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
    
    if (loc_intensity >= 0) glUniform1f(loc_intensity, light_intensity);
    if (loc_lightDiffuse >= 0) glUniform3fv(loc_lightDiffuse, 1, lightDiffuse);
    if (loc_lightSpecular >= 0) glUniform3fv(loc_lightSpecular, 1, lightDiffuse);
    if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, cameraPos);

    // Matériau
    const Material& mat = obj->getMaterial();
    GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
    GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
    GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
    GLint loc_isEmissive = glGetUniformLocation(program, "u_material.isEmissive");
    
    if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, mat.diffuse);
    if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, mat.specular);
    if (loc_matShininess >= 0) glUniform1f(loc_matShininess, mat.shininess);
    if (loc_isEmissive >= 0) glUniform1i(loc_isEmissive, mat.isEmissive ? 1 : 0);
    
    // Gérer l'affichage de la texture pour le shader Basic - CORRECTION
    GLint loc_useTexture = glGetUniformLocation(program, "u_useTexture");
    GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
    
    bool hasTexture = (obj->getMaterial().diffuseMap != 0);
    if (loc_hasTexture >= 0) glUniform1i(loc_hasTexture, hasTexture);
    if (loc_useTexture >= 0) glUniform1i(loc_useTexture, mat.useTextureInBasicShader);
    
    // Lier la texture seulement si on veut l'utiliser ET qu'elle existe
    if (hasTexture && mat.useTextureInBasicShader) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->getMaterial().diffuseMap);
        GLint loc_texture = glGetUniformLocation(program, "u_texture");
        if (loc_texture >= 0) glUniform1i(loc_texture, 0);
    } else {
        // IMPORTANT: Débinder la texture si on ne l'utilise pas
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void DemoScene::setupEnvMapShaderDemo(GLuint program, Mesh* obj, const float* cameraPos) {
    const Material& mat = obj->getMaterial();
    
    GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
    if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, cameraPos);
    
    // Nouveau uniform pour ignorer le matériau
    GLint loc_ignoreMaterial = glGetUniformLocation(program, "u_ignoreObjectMaterial");
    if (loc_ignoreMaterial >= 0) glUniform1i(loc_ignoreMaterial, mat.ignoreObjectMaterialInEnvMap);
    
    // Si on ignore le matériau, pas besoin de configurer les autres paramètres
    if (!mat.ignoreObjectMaterialInEnvMap) {
        GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
        GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
        GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
        GLint loc_specularStrength = glGetUniformLocation(program, "u_material.specularStrength");
        
        if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, mat.diffuse);
        if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, mat.specular);
        if (loc_matShininess >= 0) glUniform1f(loc_matShininess, mat.shininess);
        if (loc_specularStrength >= 0) glUniform1f(loc_specularStrength, mat.specularStrength);
        
        // Gérer l'affichage de la texture pour le shader EnvMap
        GLint loc_useTexture = glGetUniformLocation(program, "u_useTexture");
        GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
        
        bool hasTexture = (obj->getMaterial().diffuseMap != 0);
        if (loc_hasTexture >= 0) glUniform1i(loc_hasTexture, hasTexture);
        if (loc_useTexture >= 0) glUniform1i(loc_useTexture, mat.useTextureInEnvMapShader);
        
        if (hasTexture && mat.useTextureInEnvMapShader) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, obj->getMaterial().diffuseMap);
            GLint loc_texture = glGetUniformLocation(program, "u_texture");
            if (loc_texture >= 0) glUniform1i(loc_texture, 1);
        } else {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    } else {
        // Si on ignore le matériau, débinder toutes les textures
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        GLint loc_useTexture = glGetUniformLocation(program, "u_useTexture");
        GLint loc_hasTexture = glGetUniformLocation(program, "u_hasTexture");
        if (loc_hasTexture >= 0) glUniform1i(loc_hasTexture, 0);
        if (loc_useTexture >= 0) glUniform1i(loc_useTexture, 0);
    }
    
    // Associer le cubemap au shader
    if (m_CubeMap.IsLoaded()) {
        m_CubeMap.Bind(0);
        GLint loc_envmap = glGetUniformLocation(program, "u_envmap");
        if (loc_envmap >= 0) glUniform1i(loc_envmap, 0);
    }
}

void DemoScene::Cleanup() {
    for (Mesh* obj : m_objects) {
        delete obj;
    }
    m_objects.clear();
}

void DemoScene::createDemoObjects() {
    // Cube couleur (rouge) - utilisera le shader de couleur
    Mesh* colorCube = new Mesh();
    colorCube->createSphere(1.0f, 32, 32);
    Material matColor;
    matColor.diffuse[0] = 0.8f; matColor.diffuse[1] = 0.2f; matColor.diffuse[2] = 0.2f;
    matColor.specular[0] = matColor.specular[1] = matColor.specular[2] = 0.0f;
    matColor.shininess = 1.0f;
    matColor.isEmissive = false;
    colorCube->setMaterial(matColor);
    colorCube->setPosition(-8, 0, -10); // Position initiale
    colorCube->setCurrentShader(nullptr); // Sera assigné lors du rendu
    m_objects.push_back(colorCube);

    // Cube texturé - utilisera le shader basique
    Mesh* texCube = new Mesh();
    texCube->createSphere(1.0f, 32, 32);
    texCube->loadTexture("assets/textures/earth.png");
    Material matTex;
    matTex.diffuse[0] = matTex.diffuse[1] = matTex.diffuse[2] = 1.0f;
    matTex.specular[0] = matTex.specular[1] = matTex.specular[2] = 0.5f;
    matTex.shininess = 16.0f;
    matTex.isEmissive = false;
    texCube->setMaterial(matTex);
    texCube->setPosition(0, 0, -10);  // Position initiale
    texCube->setCurrentShader(nullptr);
    m_objects.push_back(texCube);

    // Cube environment mapping - utilisera le shader d'environment mapping
    Mesh* envCube = new Mesh();
    envCube->createSphere(1.0f, 32, 32);
    Material matEnv;
    matEnv.diffuse[0] = matEnv.diffuse[1] = matEnv.diffuse[2] = 1.0f;
    matEnv.specular[0] = matEnv.specular[1] = matEnv.specular[2] = 1.0f;
    matEnv.shininess = 64.0f;
    matEnv.isEmissive = false;
    envCube->setMaterial(matEnv);
    envCube->setPosition(8, 0, -10);  // Position initiale
    envCube->setCurrentShader(nullptr);
    m_objects.push_back(envCube);
}

// ==================== EmptyScene Implementation ====================

EmptyScene::EmptyScene(const std::string& name) : Scene(name) {
}

bool EmptyScene::Initialize() {
    std::cout << "=== EmptyScene::Initialize called ===" << std::endl;
    
    try {
        std::cout << "Initializing shaders..." << std::endl;
        if (!InitializeShaders()) {
            std::cerr << "Failed to initialize shaders for EmptyScene" << std::endl;
            return false;
        }
        std::cout << "EmptyScene shaders initialized successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during shader initialization: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Unknown exception during shader initialization!" << std::endl;
        return false;
    }
    
    return true;
}

void EmptyScene::Update(float deltaTime) {
    for (Mesh* obj : m_objects) {
        if (obj) {
            // Ajouter ici la logique de mise à jour si nécessaire
        }
    }
}

void EmptyScene::Render(const Mat4& projection, const Mat4& view) {
    for (Mesh* obj : m_objects) {
        if (obj) {
            if (!obj->getCurrentShader()) {
                obj->setCurrentShader(&m_basicShader);
            }
            GLShader* shader = obj->getCurrentShader();
            if (shader) {
                obj->draw(*shader);
            }
        }
    }
}

void EmptyScene::Cleanup() {
    std::cout << "=== EmptyScene::Cleanup called ===" << std::endl;
    std::cout << "Number of objects to clean: " << m_objects.size() << std::endl;
    
    for (Mesh* obj : m_objects) {
        if (obj) {
            std::cout << "Deleting object at " << obj << std::endl;
            delete obj;
        }
    }
    m_objects.clear();
    std::cout << "Objects cleared" << std::endl;
    
    CleanupShaders();
    std::cout << "Shaders cleaned up" << std::endl;
}

// ==================== SceneManager Implementation ====================

SceneManager& SceneManager::GetInstance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::AddScene(std::unique_ptr<Scene> scene) {
    std::cout << "=== AddScene called ===" << std::endl;
    std::string name = scene->GetName();
    std::cout << "Adding scene: " << name << std::endl;
    m_scenes[name] = std::move(scene);
    m_sceneOrder.push_back(name);
    std::cout << "Scene added successfully" << std::endl;
}

bool SceneManager::SetActiveScene(const std::string& sceneName) {
    std::cout << "=== SetActiveScene called ===" << std::endl;
    std::cout << "Attempting to set scene: " << sceneName << std::endl;
    
    if (g_UI) {
        std::cout << "Resetting UI..." << std::endl;
        g_UI->SetSceneObjects({}, nullptr, {});
    }

    auto it = m_scenes.find(sceneName);
    if (it == m_scenes.end()) {
        std::cerr << "Scene '" << sceneName << "' not found" << std::endl;
        return false;
    }

    if (!it->second) {
        std::cerr << "Scene pointer is null!" << std::endl;
        return false;
    }

    std::cout << "Setting active scene..." << std::endl;
    m_activeScene = it->second.get();
    m_activeSceneName = sceneName;

    std::cout << "Updating UI with new scene objects..." << std::endl;
    if (g_UI && m_activeScene) {
        const auto& objects = m_activeScene->GetObjects();
        std::cout << "Number of objects in scene: " << objects.size() << std::endl;
        g_UI->SetSceneObjects(
            objects,
            m_activeScene->GetSun(),
            m_activeScene->GetPlanets()
        );
    }

    std::cout << "Scene change completed successfully" << std::endl;
    return true;
}

bool SceneManager::Initialize() {
    // Vérifier et configurer le répertoire de base
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::filesystem::path basePath = std::filesystem::path(exePath).parent_path();
    std::cout << "Application base path: " << basePath << std::endl;
    
    // Vérifier l'existence des dossiers essentiels
    if (!std::filesystem::exists(basePath / "assets")) {
        std::cerr << "Warning: 'assets' directory not found at: " << basePath << std::endl;
    }
    if (!std::filesystem::exists(basePath / "assets" / "shaders")) {
        std::cerr << "Warning: 'shaders' directory not found at: " << (basePath / "assets") << std::endl;
    }

    // Initialiser toutes les scènes
    for (auto& pair : m_scenes) {
        if (!pair.second->Initialize()) {
            std::cerr << "Failed to initialize scene: " << pair.first << std::endl;
            return false;
        }
    }

    // Définir la première scène comme active
    if (!m_sceneOrder.empty()) {
        SetActiveScene(m_sceneOrder[0]);
    }

    return true;
}

void SceneManager::Update(float deltaTime) {
    if (m_activeScene) {
        m_activeScene->Update(deltaTime);
    }
}

void SceneManager::Render(const Mat4& projection, const Mat4& view) {
    if (m_activeScene) {
        m_activeScene->Render(projection, view);
    }
}

void SceneManager::Cleanup() {
    m_activeScene = nullptr;
    m_activeSceneName.clear();
    m_scenes.clear();
    m_sceneOrder.clear();
}

std::vector<std::string> SceneManager::GetSceneNames() const {
    return m_sceneOrder;
}

void SceneManager::NextScene() {
    if (m_sceneOrder.empty()) return;

    auto it = std::find(m_sceneOrder.begin(), m_sceneOrder.end(), m_activeSceneName);
    if (it != m_sceneOrder.end()) {
        ++it;
        if (it == m_sceneOrder.end()) {
            it = m_sceneOrder.begin();
        }
        SetActiveScene(*it);
    }
}

void SceneManager::PreviousScene() {
    if (m_sceneOrder.empty()) return;

    auto it = std::find(m_sceneOrder.begin(), m_sceneOrder.end(), m_activeSceneName);
    if (it != m_sceneOrder.end()) {
        if (it == m_sceneOrder.begin()) {
            it = m_sceneOrder.end();
        }
        --it;
        SetActiveScene(*it);
    }
}

void SceneManager::RemoveScene(const std::string& name) {
    if (name == m_activeSceneName) {
        NextScene();
    }
    
    m_scenes.erase(name);
    auto it = std::find(m_sceneOrder.begin(), m_sceneOrder.end(), name);
    if (it != m_sceneOrder.end()) {
        m_sceneOrder.erase(it);
    }
}
