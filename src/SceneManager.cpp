#include "../include/SceneManager.h"
#include <iostream>
#include <algorithm>
#include <cstdio>  // Pour sprintf
#include "../include/CameraController.h" // Ajouté pour CameraController

// Définir MAX_LIGHTS en haut du fichier
#define MAX_LIGHTS 10

// ==================== Scene Implementation ====================

bool Scene::InitializeShaders() {
    // Charger les shaders de base pour la scène
    if (!m_basicShader.LoadVertexShader("Basic.vs") ||
        !m_basicShader.LoadFragmentShader("Basic.fs") ||
        !m_basicShader.Create()) {
        return false;
    }

    if (!m_colorShader.LoadVertexShader("Color.vs") ||
        !m_colorShader.LoadFragmentShader("Color.fs") ||
        !m_colorShader.Create()) {
        return false;
    }

    if (!m_envMapShader.LoadVertexShader("EnvMap.vs") ||
        !m_envMapShader.LoadFragmentShader("EnvMap.fs") ||
        !m_envMapShader.Create()) {
        return false;
    }

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
}

void SolarSystemScene::setupColorShader(GLuint program, Mesh* obj) {
    const Material& mat = obj->getMaterial();
    GLint loc_color = glGetUniformLocation(program, "u_color");
    if (loc_color >= 0) {
        glUniform3fv(loc_color, 1, mat.diffuse);
    }
}

void SolarSystemScene::setupEnvMapShader(GLuint program, Mesh* obj, const float* cameraPos) {
    const Material& mat = obj->getMaterial();
    
    // Position de la caméra pour les réflections
    GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
    if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, cameraPos);
    
    // Paramètres du matériau pour l'environment mapping
    GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
    GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
    GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
    
    if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, mat.diffuse);
    if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, mat.specular);
    if (loc_matShininess >= 0) glUniform1f(loc_matShininess, mat.shininess);
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
    m_sun->loadTexture("models/sun.png");

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
        {65.0f, 0.12f, 3.5f},   // Saturne
        {80.0f, 0.08f, 2.5f},   // Uranus
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
        "models/mercury.png",
        "models/venus.png",
        "models/earth.png",
        "models/mars.png",
        "models/jupiter.png",
        "models/saturn.png",
        "models/uranus.png"
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
    
    // Faire tourner les objets de démonstration
    if (m_objects.size() >= 3) {
        // Rotation autour de l'axe Y
        float rotation = m_rotationTime * 0.5f;
        m_objects[0]->setRotation(0, rotation, 0);
        m_objects[1]->setRotation(0, rotation * 1.5f, 0);
        m_objects[2]->setRotation(0, rotation * 2.0f, 0);
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
}

void DemoScene::setupEnvMapShaderDemo(GLuint program, Mesh* obj, const float* cameraPos) {
    const Material& mat = obj->getMaterial();
    
    GLint loc_viewPos = glGetUniformLocation(program, "u_viewPos");
    if (loc_viewPos >= 0) glUniform3fv(loc_viewPos, 1, cameraPos);
    
    GLint loc_matDiffuse = glGetUniformLocation(program, "u_material.diffuseColor");
    GLint loc_matSpecular = glGetUniformLocation(program, "u_material.specularColor");
    GLint loc_matShininess = glGetUniformLocation(program, "u_material.shininess");
    
    if (loc_matDiffuse >= 0) glUniform3fv(loc_matDiffuse, 1, mat.diffuse);
    if (loc_matSpecular >= 0) glUniform3fv(loc_matSpecular, 1, mat.specular);
    if (loc_matShininess >= 0) glUniform1f(loc_matShininess, mat.shininess);
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
    colorCube->setPosition(-8, 0, -10); // Ajout d'un Z négatif pour être devant la caméra
    colorCube->setCurrentShader(nullptr); // Sera assigné lors du rendu
    m_objects.push_back(colorCube);

    // Cube texturé - utilisera le shader basique
    Mesh* texCube = new Mesh();
    texCube->createSphere(1.0f, 32, 32);
    texCube->loadTexture("models/earth.png");
    Material matTex;
    matTex.diffuse[0] = matTex.diffuse[1] = matTex.diffuse[2] = 1.0f;
    matTex.specular[0] = matTex.specular[1] = matTex.specular[2] = 0.5f;
    matTex.shininess = 16.0f;
    matTex.isEmissive = false;
    texCube->setMaterial(matTex);
    texCube->setPosition(0, 0, -10);  // Ajout d'un Z négatif pour être devant la caméra
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
    envCube->setPosition(8, 0, -10);  // Ajout d'un Z négatif pour être devant la caméra
    envCube->setCurrentShader(nullptr);
    m_objects.push_back(envCube);
}

// ==================== SceneManager Implementation ====================

SceneManager& SceneManager::GetInstance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::AddScene(std::unique_ptr<Scene> scene) {
    std::string name = scene->GetName();
    m_scenes[name] = std::move(scene);
    m_sceneOrder.push_back(name);
}

bool SceneManager::SetActiveScene(const std::string& sceneName) {
    auto it = m_scenes.find(sceneName);
    if (it == m_scenes.end()) {
        std::cerr << "Scene '" << sceneName << "' not found" << std::endl;
        return false;
    }

    m_activeScene = it->second.get();
    m_activeSceneName = sceneName;

    // Mise à jour de l'UI avec les objets de la nouvelle scène
    if (g_UI) {
        g_UI->SetSceneObjects(
            m_activeScene->GetObjects(),
            m_activeScene->GetSun(),
            m_activeScene->GetPlanets()
        );
    }

    return true;
}

bool SceneManager::Initialize() {
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
